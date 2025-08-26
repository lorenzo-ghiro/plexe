#include "plexe/apps/NNWrapper.h"

#define PYBIND11_EXPORT __attribute__((visibility("default")))
#include <cmath>


struct NNWrapper::Impl {
    py::object model;
    py::object scaler;
    py::object aimds_instance;


    Impl(const std::string& model_path, const std::string& scaler_path, int MCdropRep, double MCdropCU) {
        // Allow only first initialization of the python interpreter
        if (!Py_IsInitialized()) {
            py::initialize_interpreter();
        }
        py::module_ keras = py::module_::import("tensorflow.keras.models");
        py::module_ pickle = py::module_::import("pickle");
        py::module_ builtins = py::module_::import("builtins");

        // Load the model
        model = keras.attr("load_model")(model_path, py::arg("compile") = false);

        // Load the scaler
        py::object scaler_file = builtins.attr("open")(scaler_path, "rb");
        scaler = pickle.attr("load")(scaler_file);
        scaler_file.attr("close")();

        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("insert")(0, "./offlineAnalysis");
        py::module_ ai_mds = py::module_::import("ai_mds");
        aimds_instance = ai_mds.attr("Aimds")(model_path, scaler_path, MCdropRep, MCdropCU);
    }

    ~Impl() {

    }

    py::array_t<double> normalize(std::vector<const CAM*> rawSequence)
    {
        Message first_msg(rawSequence[0]);

        std::vector<double> norm_diffs;

        for (int i=1; i<5; ++i) {
            Message current(rawSequence[i]);
            norm_diffs.push_back((current.time - first_msg.time)/0.1);
            norm_diffs.push_back((current.posx - first_msg.posx)/0.1);
            norm_diffs.push_back((current.posy - first_msg.posy)/0.1);
            norm_diffs.push_back((current.spdx - first_msg.spdx)/0.1);
            norm_diffs.push_back((current.spdy - first_msg.spdy)/0.1);
            norm_diffs.push_back((current.acl - first_msg.acl)/0.1);
        }
        return reshapeNormDiff(norm_diffs);
    }


    py::array_t<double> reshapeNormDiff(std::vector<double>& normunshaped)
    {
        if (normunshaped.size() != 24) {
            throw std::runtime_error("Expected 24 elements in normunshaped");
        }

        // Step 1: make a (1, 24) array shape
        py::array_t<double> input_array({1, 24});
        auto buf = input_array.request();
        double* ptr = static_cast<double*>(buf.ptr);

        for (size_t i = 0; i < 24; ++i)
            ptr[i] = normunshaped[i];

        // Step 2: Reshape in (1, 4, 6)
        py::object reshaped = input_array.attr("reshape")(py::make_tuple(1, 4, 6));
        py::array_t<double> tensor = reshaped.cast<py::array_t<double>>();
        return tensor;
    }

    void predict_with_confidence(py::object& aimds_instance, py::array_t<double>& tensor, int& label, double& mu_selected, double& CL) {
        py::tuple result = aimds_instance.attr("evaluate_sequence")(tensor, py::none());

        label = result[0].cast<int>();
        mu_selected = result[1].cast<double>();
        CL = result[2].cast<double>();
    }

};

NNWrapper::NNWrapper(const std::string& model_path, const std::string& scaler_path, int MCdropRep, double MCdropCU)
    : pimpl(new Impl(model_path, scaler_path, MCdropRep, MCdropCU)) {}

NNWrapper::~NNWrapper() {
    delete pimpl;
}

int NNWrapper::predict(std::vector<const CAM*> ordMsgs, int& label, double& cai, double& muai, double& CLai){
    py::array_t<double> tensor = pimpl->normalize(ordMsgs);
    pimpl->predict_with_confidence(pimpl->aimds_instance, tensor, label, muai, CLai);
    cai = muai * CLai;

    return label;
}
