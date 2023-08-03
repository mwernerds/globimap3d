#include <pybind11/pybind11.h>
#include<pybind11/numpy.h>

#include <boost/config.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/int.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>

#include<iostream>

#include "globimap.hpp"



int add(int i, int j) {
    return i + j;
}
namespace py = pybind11;
namespace bg = boost::geometry;

template<typename func>
void map_matrix(py::array_t<double> mat, func f){
    auto ref = mat.unchecked<2>();
    auto shapeX = ref.shape(0);
    auto shapeY = ref.shape(1);
//    auto dataPtr = (double*) mat.request().ptr;

    for (int x=0; x < shapeX; x++)
        for(int y=0; y < shapeY; y++)
        {
            
            f(x,y,ref(x,y));
        }
}


typedef GloBiMap<bool> globimap_t;
class globimap3d_t :public GloBiMap<bool> {int bogus;};

PYBIND11_MODULE(helena, m) {
    /**
     * NOT FOR PRODUCTIVITY - DEVELOPMENT ONLY
     * */

    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: python_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

//    m.def("test", &test,R"pbdoc(
//        A test for numpy
//
//        Some other explanation about the add function.
//    )pbdoc");
    m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

    m.def("subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");

    /**
    * END  - NOT FOR PRODUCTIVITY - DEVELOPMENT ONLY
    * */

#ifdef VERSION_INFO
        m.attr("__version__") = VERSION_INFO;
#else
        m.attr("__version__") = "dev";
#endif

    py::class_<globimap_t>(m, "globimap",py::module_local())
            .def(py::init<>())
            .def("configure", +[](globimap_t &self, size_t k, size_t m)  {  self.configure(k,m);})
            .def("summary", +[](globimap_t &self) { return self.summary();})
            .def("put", +[](globimap_t &self, uint32_t x, uint32_t y) { self.put({x,y});})
            .def("get", +[](globimap_t &self, uint32_t x, uint32_t y) { return self.get({x,y});})
            .def("clear", +[](globimap_t &self){self.clear();})
            .def("map", +[](globimap_t &self, py::array_t<double> mat, int o0, int o1) {

                auto f = [&](int x, int y, double v){
                    if (v != 0 && v!= 1)
                        throw(std::runtime_error("data is not binary."));
                    if (v == 1)
                        self.put({static_cast<uint32_t>(o0+x),static_cast<uint32_t>(o1+y)});
                };

                map_matrix(mat, f);
            })
            .def("enforce", +[](globimap_t &self, py::array_t<double> mat, int o0, int o1){

                auto f = [&](int x, int y, double v){
                    if (v==0 && self.get({static_cast<uint32_t>(o0+x),static_cast<uint32_t>(o1+y)})) // this is a false positive
                        self.add_error({static_cast<uint32_t>(o0+x),static_cast<uint32_t>(o1+y)});
                };

                map_matrix(mat, f);
            })
            .def("rasterize", +[](globimap_t& self, size_t x, size_t y, size_t s0, size_t s1){
                std::vector<double>* v = &self.rasterize(x,y,s0,s1);

                std::unique_ptr<std::vector<double> > seq_ptr = std::make_unique<std::vector<double>>((*v)); // Unique ptr does not leak if for some reason py::capsule would throw.
                auto capsule = py::capsule(seq_ptr.get(), [](void *p) { std::unique_ptr<std::vector<double> >(reinterpret_cast<std::vector<double> *>(p)); });
                seq_ptr.release();

                return py::array({ s0, s1 }, v->data(), capsule);
            })

            .def("correct", +[](globimap_t& self, size_t x, size_t y, size_t s0, size_t s1) {
                std::vector<double>* v = &self.apply_correction(x, y, s0, s1);

                std::unique_ptr<std::vector<double> > seq_ptr = std::make_unique<std::vector<double>>((*v)); // Unique ptr does not leak if for some reason py::capsule would throw.
                auto capsule = py::capsule(seq_ptr.get(), [](void *p) { std::unique_ptr<std::vector<double> >(reinterpret_cast<std::vector<double> *>(p)); });
                seq_ptr.release();

                return py::array_t({ s0, s1 }, v->data(), capsule);
            });


    py::class_<globimap3d_t>(m, "globimap3d",py::module_local())
            .def(py::init<>())
            .def("configure", +[](globimap3d_t &self, size_t k, size_t m)  {  self.configure(k,m);})
            .def("summary", +[](globimap3d_t &self) { return self.summary();})
            .def("put", +[](globimap3d_t &self, uint64_t x, uint64_t y, uint64_t z) {
	    // arrange everything in first 128 bit of array
	    const uint64_t w = 0;
	    uint64_t a = (x << 32) | y, b = (z << 32) | w;
	    self.put({a,b});})
            .def("get", +[](globimap3d_t &self, uint64_t x, uint64_t y, uint64_t z) { 
	    const uint64_t w = 0;
	    uint64_t a = (x << 32) | y, b = (z << 32) | w;
	    return self.get({a,b});})
	    
            .def("clear", +[](globimap3d_t &self){self.clear();});
            /*  THESE ARE NOT YET 3D
		
.def("map", +[](globimap_t &self, py::array_t<double> mat, int o0, int o1) {

                auto f = [&](int x, int y, double v){
                    if (v != 0 && v!= 1)
                        throw(std::runtime_error("data is not binary."));
                    if (v == 1)
                        self.put({static_cast<uint32_t>(o0+x),static_cast<uint32_t>(o1+y)});
                };

                map_matrix(mat, f);
            })
            .def("enforce", +[](globimap_t &self, py::array_t<double> mat, int o0, int o1){

                auto f = [&](int x, int y, double v){
                    if (v==0 && self.get({static_cast<uint32_t>(o0+x),static_cast<uint32_t>(o1+y)})) // this is a false positive
                        self.add_error({static_cast<uint32_t>(o0+x),static_cast<uint32_t>(o1+y)});
                };

                map_matrix(mat, f);
            })
            .def("rasterize", +[](globimap_t& self, size_t x, size_t y, size_t s0, size_t s1){
                std::vector<double>* v = &self.rasterize(x,y,s0,s1);

                std::unique_ptr<std::vector<double> > seq_ptr = std::make_unique<std::vector<double>>((*v)); // Unique ptr does not leak if for some reason py::capsule would throw.
                auto capsule = py::capsule(seq_ptr.get(), [](void *p) { std::unique_ptr<std::vector<double> >(reinterpret_cast<std::vector<double> *>(p)); });
                seq_ptr.release();

                return py::array({ s0, s1 }, v->data(), capsule);
            })

            .def("correct", +[](globimap_t& self, size_t x, size_t y, size_t s0, size_t s1) {
                std::vector<double>* v = &self.apply_correction(x, y, s0, s1);

                std::unique_ptr<std::vector<double> > seq_ptr = std::make_unique<std::vector<double>>((*v)); // Unique ptr does not leak if for some reason py::capsule would throw.
                auto capsule = py::capsule(seq_ptr.get(), [](void *p) { std::unique_ptr<std::vector<double> >(reinterpret_cast<std::vector<double> *>(p)); });
                seq_ptr.release();

                return py::array_t({ s0, s1 }, v->data(), capsule);
            });*/



} // THE MODULE END
