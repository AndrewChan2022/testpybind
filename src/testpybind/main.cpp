#include <pybind11/pybind11.h>

namespace py = pybind11;

template<typename T>
T add_scalar(T a, T b) {
    return a + b;
}

void bind_scalar(py::module &m) {
    m.def("add_scalar", &add_scalar<int>, "Add two ints");
    m.def("add_scalar", &add_scalar<float>, "Add two floats");
    m.def("add_scalar", &add_scalar<double>, "Add two doubles");
}

// declare
void bind_array(pybind11::module &m);

PYBIND11_MODULE(testpybind, m) {
    m.doc() = "TestPyBind module";

    // Call sub-bind functions from each file
    bind_scalar(m);
    bind_array(m);
}