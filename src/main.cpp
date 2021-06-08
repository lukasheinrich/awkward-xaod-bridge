#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <iostream>

namespace py = pybind11;

class JetAuxContainer{
    public:
        JetAuxContainer(double* pt, double* eta, size_t size) : m_pt(pt), m_eta(eta), m_size(size){}
        double pt(int index) const {return m_pt[index];}
        double eta(int index) const {return m_eta[index];}
        size_t size() const {return m_size;}
    
    private:
        double* m_pt;
        double* m_eta;
        size_t m_size;
};

class Jet {
    public:
        Jet(const int index, const JetAuxContainer& aux) : m_index(index), m_aux(aux){}
        double pt() const {return m_aux.pt(m_index);}  
        double eta() const {return m_aux.eta(m_index);}  
    private:
        const int m_index;
        const JetAuxContainer& m_aux;
};

class JetContainer{
    public:
        JetContainer(const JetAuxContainer& aux) : m_aux(aux) {}
        Jet at(int index) const {return Jet(index, m_aux);}
        size_t size() const { return m_aux.size(); }
    private:
        const JetAuxContainer& m_aux;
};

class JetTool {
    public:
        double calibrate(const Jet& j){
            return j.pt()+j.eta();
        }
        std::vector<double> calibrate(const JetContainer& c){
            std::vector<double> v(c.size());
            for (size_t idx = 0; idx < v.size(); idx++){
                std::cout << c.at(idx).pt() << " | " << c.at(idx).eta() << std::endl;
                v[idx] = this->calibrate(c.at(idx));
            }
            return v;
        }
};

JetTool jet_tool;

py::array_t<double> calibrate(py::array_t<double> jet_pt, py::array_t<double> jet_eta) {
    py::buffer_info buf1 = jet_pt.request(), buf2 = jet_eta.request();

    if (buf1.ndim != 1 || buf2.ndim != 1)
        throw std::runtime_error("Number of dimensions must be one");

    if (buf1.size != buf2.size)
        throw std::runtime_error("Input shapes must match");

    auto result = py::array_t<double>(buf1.size);

    py::buffer_info buf3 = result.request();

    double *jet_pt_array  = static_cast<double *>(buf1.ptr);
    double *jet_eta_array = static_cast<double *>(buf2.ptr);

    JetAuxContainer jetaux(jet_pt_array,jet_eta_array, buf1.size);
    JetContainer jets(jetaux);

    double *jet_calibrated_array = static_cast<double *>(buf3.ptr);

    auto tool_result = jet_tool.calibrate(jets);
    for (size_t idx = 0; idx < buf1.shape[0]; idx++){ //todo make zero-copy
        jet_calibrated_array[idx] = tool_result[idx];
    }
    return result;
}

PYBIND11_MODULE(_core, m) {
    m.def("calibrate", &calibrate, "Calibrate Array");
}
