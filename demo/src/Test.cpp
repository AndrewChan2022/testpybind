#include <iostream>
#include <myadd/fadd.h>

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;


class PyObjectHolder {
public:
    PyObjectHolder(const std::string& modulePath) {


        if (Py_IsInitialized() == 0) {
            py::initialize_interpreter();
            mIsPythonOwner = true;

            PyObject* sys_module = PyImport_ImportModule("sys");
            PyObject* py_executable = PyObject_GetAttrString(sys_module, "executable");
            const char* executable_path = PyUnicode_AsUTF8(py_executable);

            std::cout << "Python initialized from: " << executable_path << std::endl;

            // Get sys.prefix (Conda env path)
            PyObject* py_prefix = PyObject_GetAttrString(sys_module, "prefix");
            const char* prefix_path = PyUnicode_AsUTF8(py_prefix);
            std::cout << "Python sys.prefix: " << prefix_path << std::endl;

			Py_XDECREF(py_executable);
			Py_XDECREF(sys_module);
            Py_DECREF(sys_module);


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
    ~PyObjectHolder() {
        if (m_mainThreadState) {
            PyEval_RestoreThread(m_mainThreadState); // restore main thread
            m_mainThreadState = nullptr;

            // almost all embedded Python, just skip Py_Finalize() entirely.
            // skip Py_Finalize() if you embed into larger app
            // py::finalize_interpreter();
            // std::cout << "[Finalize] Python finalized" << std::endl;
        }
    }
    

private: 
    bool mIsPythonOwner = false;
    PyThreadState* m_mainThreadState = nullptr;

};



int main(int argc, char* argv[]){

    float c = myadd::fadd(1.0f, 2.0f);

    printf("c: %f\n", c);   


    std::string exepath = argv[0];
    std::string prjpath = exepath.substr(0, exepath.find_last_of("/\\")) + "/../../..";
    std::string pypath = prjpath + "/test";
    printf("user python path: %s\n", pypath.c_str());


    PyObjectHolder pyHolder(pypath);
    {
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

    return 0;
}