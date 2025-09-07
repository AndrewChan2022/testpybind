#include <iostream>
#include <myadd/fadd.h>

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;


class PyObjectHolder {
public:
    PyObjectHolder(const std::string& modulePath);
    ~PyObjectHolder();
    
private: 
    bool mIsPythonOwner = false;
    PyThreadState* m_mainThreadState = nullptr;
};


PyObjectHolder::PyObjectHolder(const std::string& modulePath) {

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

    if (Py_IsInitialized() == 0) {

        printf("start python\n");


        py::initialize_interpreter();
        mIsPythonOwner = true;

        // get python.exe path
        py::module_ sys = py::module_::import("sys");
        std::string executable_path = sys.attr("executable").cast<std::string>();
        std::cout << "Python initialized from: " << executable_path << std::endl;

        // sys.prefix (Conda env path)
        std::string prefix_path = sys.attr("prefix").cast<std::string>();
        std::cout << "Python sys.prefix: " << prefix_path << std::endl;
    }
    else {
        printf("someone has already init python \n");
    }

    {
        // Acquire GIL explicitly for setup
        py::gil_scoped_acquire gil;

        // py::exec("import sys; print(sys.path)");

        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("append")(modulePath);

        // try {
        //     mPreprocessInstance = py::module_::import(moduleNames[0].c_str()).attr(classNames[0].c_str())();
        // }
        // catch (py::error_already_set& e) {
        //     std::cerr << "Python import error:\n" << e.what() << std::endl;
        // }

    }

    // Release GIL permanently, store the thread state
    if (mIsPythonOwner) {
        m_mainThreadState = PyEval_SaveThread();
        printf("[Init] GIL released in init thread");
    }
}

PyObjectHolder::~PyObjectHolder() {
    if (m_mainThreadState) {
        PyEval_RestoreThread(m_mainThreadState); // restore main thread
        m_mainThreadState = nullptr;

        // almost all embedded Python, just skip Py_Finalize() entirely.
        // skip Py_Finalize() if you embed into larger app
        // py::finalize_interpreter();
        // std::cout << "[Finalize] Python finalized" << std::endl;
    }
}



void run_python_code() {
    py::gil_scoped_acquire gil;
    try {
        py::module_ testModule = py::module_::import("callee");
        py::object result = testModule.attr("test_function")(3.0, 4.0);
        double sum = result.cast<double>();
        std::cout << "Result from Python: " << sum << std::endl;
    }
    catch (py::error_already_set& e) {
        std::cerr << "Python error:\n" << e.what() << std::endl;
    }
}


int main(int argc, char* argv[]){

    float c = myadd::fadd(1.0f, 2.0f);

    printf("c: %f\n", c);   


    std::string exepath = argv[0];
    std::string prjpath = exepath.substr(0, exepath.find_last_of("/\\")) + "/../../..";
    std::string pypath = prjpath + "/test";
    printf("user python path: %s\n", pypath.c_str());


    PyObjectHolder pyHolder(pypath);
    run_python_code();

    return 0;
}