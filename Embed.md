
## implement

```cpp
#ifdef _WIN32

    // Only on Windows: detect Conda env and set Python home
    std::wstring pythonHome;

            
    printf("get conda home\n");
    const char* conda_prefix = std::getenv("CONDA_PREFIX");
    if (conda_prefix && strlen(conda_prefix) > 0) {
        pythonHome = std::wstring(conda_prefix, conda_prefix + strlen(conda_prefix));
    }

    wprintf(L"set conda home:%ws\n", pythonHome.c_str());
    if (!pythonHome.empty()) {
        Py_SetPythonHome(pythonHome.c_str());
    }

#endif

```


## build for conda
```bash
conda activate py310
mkdir build & cd build
cmake ..
```


## run for conda
```bash
conda activate py310
# luanch vs with conda env
start "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" E:\data\github\gsp2\testpybind\vsbuild\TestPyBind.sln
```

## build for system
```bash
conda deactivate
mkdir build & cd build
cmake ..
```

## run for system
```bash
# run msvc from start
```

