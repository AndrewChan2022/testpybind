#!/bin/bash
set -e

PYTHON_VERSIONS=("3.9" "3.10" "3.11" "3.12")

for ver in "${PYTHON_VERSIONS[@]}"; do
    echo "Building wheel for Python ${ver}"
    conda activate "py${ver}"
    python -m pip install --upgrade pip setuptools wheel cmake pybind11
    python -m build --wheel --outdir dist
done
