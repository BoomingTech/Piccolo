# tinyobjloader Python tests

This folder hosts a project for running the Python binding tests.

## Development

The tests require NumPy. To optimize CI install times, the uv.lock excludes
NumPy, as for some Python versions, pinning a version would result in builds
from source which are then discarded. To run the tests locally, after running
`uv sync`, install into the venv a version of NumPy from the build matrix in
`.github/workflows/python.yml`.

