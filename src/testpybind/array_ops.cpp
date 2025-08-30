#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <stdexcept>

namespace py = pybind11;

py::array_t<double> add_arrays(py::array_t<double> a, py::array_t<double> b) {
    if (a.size() != b.size())
        throw std::runtime_error("Arrays must have the same size");

    auto result = py::array_t<double>(a.size());
    auto buf_a = a.unchecked<1>();
    auto buf_b = b.unchecked<1>();
    auto buf_r = result.mutable_unchecked<1>();

    for (py::ssize_t i = 0; i < a.size(); ++i)
        buf_r(i) = buf_a(i) + buf_b(i);

    // Correct: convert shape to vector
    std::vector<py::ssize_t> shape(a.ndim());
    for (py::ssize_t i = 0; i < a.ndim(); ++i)
        shape[i] = a.shape(i);

    result.resize(shape);
    return result;
}

void bind_array(py::module &m) {
    m.def("add_arrays", &add_arrays, "Add two numpy arrays elementwise");
}
