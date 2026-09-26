# MAPF CBS visualizer

Start the interactive visualizer and its local CBS API:

```sh
make serve
```

Open `http://localhost:8000`. Set the grid columns and rows, then choose
**Resize**. Edit a robot's name in the name field; use the start/goal tools to
place its endpoints and click cells to add/remove obstacles. Add robots with
**+ Robot**, or choose **Random setup** to create a connected random map and
robot endpoints. Choose **Run CBS** to calculate and animate conflict-free
paths. The selected scenario and CBS paths are also written to `build/result.json`.

To export the built-in demo without opening the interface, run `make demo`.

Run the test suite with `make test`.
