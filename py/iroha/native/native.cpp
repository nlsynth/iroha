// WIP, experimental and might be deleted.
//
// This module provides immutable view of designs in Iroha.
//
// Please use python3.
// $ make -f Build.mk
//
// NOTE: *Wrapper classes are used to have their own lifecycle with Python.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "iroha/iroha.h"

namespace py = pybind11;

class ModuleWrapper {
public:
  ModuleWrapper(iroha::IModule *module) : module_(module) {
  }

private:
  iroha::IModule *module_;
};

class DesignWrapper {
public:
  DesignWrapper(iroha::IDesign *design) : design_(design) {
    for (auto *m : design->modules_) {
      modules_.push_back(new ModuleWrapper(m));
    }
  }

  iroha::IDesign *GetDesign() {
    return design_;
  }

  vector<ModuleWrapper *> GetModules() {
    return modules_;
  }

private:
  iroha::IDesign *design_;
  vector<ModuleWrapper *> modules_;
};

class WriterWrapper {
public:
  WriterWrapper(iroha::WriterAPI *api) : api_(api) {
  }

  void SetLanguage(const string &lang) {
    api_->SetLanguage(lang);
  }

  void Write(const string &fn) {
    api_->Write(fn);
  }

private:
  iroha::WriterAPI *api_;
};

DesignWrapper *ReadDesignFromFile(const string &fn) {
  return new DesignWrapper(iroha::Iroha::ReadDesignFromFile(fn));
}

WriterWrapper *CreateWriter(DesignWrapper *design) {
  return new WriterWrapper(iroha::Iroha::CreateWriter(design->GetDesign()));
}

PYBIND11_MODULE(native, m) {
  m.doc() = "Native module of Iroha";

  iroha::Iroha::Init();

  m.def("read", &ReadDesignFromFile, "Read a design from a file");
  m.def("createWriter", &CreateWriter, "Create a writer");

  py::class_<DesignWrapper>(m, "IDesign")
    .def("getModules", &DesignWrapper::GetModules);
  py::class_<ModuleWrapper>(m, "IModule");
  py::class_<WriterWrapper>(m, "Writer")
    .def("setLanguage", &WriterWrapper::SetLanguage)
    .def("write", &WriterWrapper::Write);
  py::class_<iroha::OptAPI>(m, "Optimizer");
}
