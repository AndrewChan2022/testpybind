@echo off
setlocal

REM List of Python versions
set PY_VERSIONS=39 310 311 312

for %%V in (%PY_VERSIONS%) do (
    echo Building wheel for Python %%V
    call conda activate py%%V
    python -m pip install --upgrade pip setuptools wheel cmake pybind11 build
    python -m build --wheel --outdir dist
)

endlocal
