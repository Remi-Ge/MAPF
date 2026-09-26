# MAPF CBS visualizer

Start the interactive visualizer and its local CBS API:

```sh
make serve
```

Open `http://localhost:8000`. Click cells to add/remove obstacles or use the
start/goal tools to place the selected robot. Add robots with **+ Robot**, then
choose **Run CBS** to calculate and animate conflict-free paths. The selected
scenario and CBS paths are also written to `build/result.json`.

To export the built-in demo without opening the interface, run `make demo`.

Run the test suite with `make test`.
