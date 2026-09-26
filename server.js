const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const { spawn, spawnSync } = require('node:child_process');

const root = __dirname;
const port = Number(process.env.PORT || 8000);
const maxBodyBytes = 1024 * 1024;

function sendJson(response, status, data) {
  response.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8' });
  response.end(JSON.stringify(data));
}

function isInteger(value) {
  return Number.isInteger(value);
}

function validateScenario(data) {
  if (!data || !isInteger(data.width) || !isInteger(data.height)
      || data.width < 1 || data.height < 1 || data.width > 40 || data.height > 40
      || !Array.isArray(data.obstacles) || !Array.isArray(data.agents)
      || data.agents.length < 1 || data.agents.length > 8) {
    throw new Error('Use a grid from 1×1 to 40×40 and between 1 and 8 robots.');
  }

  const inside = ([x, y]) => isInteger(x) && isInteger(y)
    && x >= 0 && x < data.width && y >= 0 && y < data.height;
  const key = ([x, y]) => `${x},${y}`;
  const obstacles = new Set();
  for (const point of data.obstacles) {
    if (!Array.isArray(point) || point.length !== 2 || !inside(point)) {
      throw new Error('An obstacle is outside the grid.');
    }
    obstacles.add(key(point));
  }

  const starts = new Set();
  const goals = new Set();
  for (const [index, agent] of data.agents.entries()) {
    if (!agent || !Array.isArray(agent.start) || agent.start.length !== 2
        || !Array.isArray(agent.goal) || agent.goal.length !== 2
        || !inside(agent.start) || !inside(agent.goal)) {
      throw new Error(`Robot ${index + 1} needs a start and a goal inside the grid.`);
    }
    if (obstacles.has(key(agent.start)) || obstacles.has(key(agent.goal))) {
      throw new Error(`Robot ${index + 1} start or goal is on an obstacle.`);
    }
    if (starts.has(key(agent.start))) {
      throw new Error('Two robots cannot start on the same cell.');
    }
    if (goals.has(key(agent.goal))) {
      throw new Error('Two robots cannot share a goal cell.');
    }
    starts.add(key(agent.start));
    goals.add(key(agent.goal));
  }

  return {
    width: data.width,
    height: data.height,
    obstacles: [...obstacles].map(cell => cell.split(',').map(Number)),
    agents: data.agents
  };
}

function readBody(request) {
  return new Promise((resolve, reject) => {
    let body = '';
    request.on('data', chunk => {
      body += chunk;
      if (Buffer.byteLength(body) > maxBodyBytes) {
        reject(new Error('Scenario payload is too large.'));
        request.destroy();
      }
    });
    request.on('end', () => {
      try {
        resolve(JSON.parse(body));
      } catch {
        reject(new Error('Request body must contain valid JSON.'));
      }
    });
    request.on('error', reject);
  });
}

function compileSolver() {
  const build = spawnSync('make', ['build/mapf_demo'], {
    cwd: root,
    encoding: 'utf8',
    timeout: 30000
  });
  if (build.error || build.status !== 0) {
    throw new Error(build.stderr || build.error?.message || 'Could not compile the CBS solver.');
  }
}

function runSolver(scenario) {
  const args = [
    '--solve', String(scenario.width), String(scenario.height),
    String(scenario.obstacles.length),
    ...scenario.obstacles.flatMap(([x, y]) => [String(x), String(y)]),
    String(scenario.agents.length),
    ...scenario.agents.flatMap(agent => [
      ...agent.start.map(String),
      ...agent.goal.map(String)
    ])
  ];

  return new Promise((resolve, reject) => {
    const child = spawn(path.join(root, 'build', 'mapf_demo'), args, { cwd: root });
    let stdout = '';
    let stderr = '';
    let timedOut = false;
    const timeout = setTimeout(() => {
      timedOut = true;
      child.kill('SIGKILL');
    }, 20000);

    child.stdout.setEncoding('utf8');
    child.stderr.setEncoding('utf8');
    child.stdout.on('data', chunk => { stdout += chunk; });
    child.stderr.on('data', chunk => { stderr += chunk; });
    child.on('error', error => {
      clearTimeout(timeout);
      reject(error);
    });
    child.on('close', code => {
      clearTimeout(timeout);
      if (timedOut) {
        reject(new Error('CBS took too long for this scenario. Try a smaller map or fewer robots.'));
        return;
      }
      if (code !== 0) {
        reject(new Error(stderr.trim() || 'CBS could not find a solution.'));
        return;
      }
      try {
        resolve(JSON.parse(stdout));
      } catch {
        reject(new Error('CBS returned an invalid result.'));
      }
    });
  });
}

const server = http.createServer(async (request, response) => {
  const url = new URL(request.url, `http://${request.headers.host || 'localhost'}`);

  if (request.method === 'GET' && url.pathname === '/health') {
    sendJson(response, 200, { ok: true });
    return;
  }

  if (request.method === 'POST' && url.pathname === '/solve') {
    try {
      const scenario = validateScenario(await readBody(request));
      compileSolver();
      const result = await runSolver(scenario);
      fs.mkdirSync(path.join(root, 'build'), { recursive: true });
      fs.writeFileSync(path.join(root, 'build', 'result.json'), `${JSON.stringify(result, null, 2)}\n`);
      sendJson(response, 200, result);
    } catch (error) {
      const status = error.message.includes('too long') ? 408
        : error.message.includes('compile') || error.code ? 500
        : error.message.includes('valid JSON') || error.message.includes('Scenario payload') ? 400
        : 422;
      sendJson(response, status, { error: error.message });
    }
    return;
  }

  if (request.method !== 'GET' && request.method !== 'HEAD') {
    response.writeHead(405, { Allow: 'GET, HEAD, POST' });
    response.end();
    return;
  }

  const pathname = url.pathname === '/' ? '/visualizer.html' : decodeURIComponent(url.pathname);
  const filePath = path.resolve(root, `.${pathname}`);
  if (!filePath.startsWith(`${root}${path.sep}`) || !fs.existsSync(filePath) || !fs.statSync(filePath).isFile()) {
    response.writeHead(404);
    response.end('Not found');
    return;
  }

  const contentTypes = {
    '.html': 'text/html; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.js': 'text/javascript; charset=utf-8',
    '.css': 'text/css; charset=utf-8'
  };
  response.writeHead(200, { 'Content-Type': contentTypes[path.extname(filePath)] || 'application/octet-stream' });
  if (request.method === 'HEAD') response.end();
  else fs.createReadStream(filePath).pipe(response);
});

server.listen(port, '127.0.0.1', () => {
  console.log(`MAPF visualizer ready at http://localhost:${port}/`);
});

server.on('error', error => {
  if (error.code !== 'EADDRINUSE') {
    console.error(error.message);
    process.exitCode = 1;
    return;
  }

  const request = http.get({ hostname: '127.0.0.1', port, path: '/health', timeout: 1000 }, response => {
    let body = '';
    response.setEncoding('utf8');
    response.on('data', chunk => { body += chunk; });
    response.on('end', () => {
      try {
        if (response.statusCode === 200 && JSON.parse(body).ok === true) {
          console.log(`MAPF visualizer is already running at http://localhost:${port}/`);
          return;
        }
      } catch {
        // The occupied port does not host this MAPF server.
      }
      console.error(`Port ${port} is already in use by another application.`);
      process.exitCode = 1;
    });
  });
  request.on('timeout', () => request.destroy(new Error('Health check timed out.')));
  request.on('error', () => {
    console.error(`Port ${port} is already in use by another application.`);
    process.exitCode = 1;
  });
});
