#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <iostream>

namespace py = pybind11;

class JetAuxContainer{
    public:
        JetAuxContainer(double* pt, double* eta) : m_pt(pt), m_eta(eta){}
        double pt(int index) const {return m_pt[index];}
        double eta(int index) const {return m_eta[index];}
    
    private:
        double* m_pt;
        double* m_eta;
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
        Jet at(int index){return Jet(index, m_aux);}
    private:
        const JetAuxContainer& m_aux;
};

class JetTool {
    public:
        double calibrate(const Jet& j){
            return j.pt()+j.eta();
        }
        std::vector<double> calibrate(const JetContainer& c){
            std::vector<double> v;
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

    JetAuxContainer jetaux(jet_pt_array,jet_eta_array);
    JetContainer jets(jetaux);

    double *jet_calibrated_array = static_cast<double *>(buf3.ptr);

    for (size_t idx = 0; idx < buf1.shape[0]; idx++){
        std::cout << jets.at(idx).pt() << " | " << jets.at(idx).eta() << std::endl;
        jet_calibrated_array[idx] = jet_tool.calibrate(jets.at(idx));
    }
    return result;
}

PYBIND11_MODULE(_core, m) {
    m.def("calibrate", &calibrate, "Calibrate Array");
}
