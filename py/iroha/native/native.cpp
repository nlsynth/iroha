#include <pybind11/pybind11.h>

#include "iroha/iroha.h"

namespace py = pybind11;

int add(int i, int j) {
  return i + j;
}

void Init() {
  iroha::Iroha::Init();
}

iroha::IDesign *ReadDesignFromFile(const string &fn) {
  return iroha::Iroha::ReadDesignFromFile(fn);
}

iroha::WriterAPI *CreateWriter(iroha::IDesign *design) {
  return iroha::Iroha::CreateWriter(design);
}

PYBIND11_MODULE(native, m) {
  m.doc() = "Native module of Iroha";

  m.def("Init", &Init, "Initialize Iroha");
  m.def("Read", &ReadDesignFromFile, "Read a design from a file");
  m.def("CreateWriter", &CreateWriter, "Create a writer");

  py::class_<iroha::IDesign>(m, "IDesign");
  py::class_<iroha::WriterAPI>(m, "Writer")
    .def("setLanguage", &iroha::WriterAPI::SetLanguage)
    .def("write", &iroha::WriterAPI::Write);
  py::class_<iroha::OptAPI>(m, "Optimizer");
}
