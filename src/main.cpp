#include <iostream>

#include "iroha/iroha.h"

void printVersion() {
  std::cout << PACKAGE << "-" << VERSION << "\n";
}

int main(int argc, char **argv) {
  vector<string> files;
  bool verilog = false;

  string output;
  vector<string> opts;

  for (int i = 1; i < argc; ++i) {
    const string arg(argv[i]);
    if (arg == "--version") {
      printVersion();
      return 0;
    }
    if (arg == "-v") {
      verilog = true;
      continue;
    }
    if (arg == "-o" && i + 1 < argc) {
      output = string(argv[i + 1]);
      ++i;
      continue;
    }
    if (arg == "-opt" && i + 1 < argc) {
      iroha::Util::SplitStringUsing(argv[i + 1], ",", &opts);
      ++i;
      continue;
    }
    files.push_back(arg);
  }

  iroha::Iroha::Init();

  for (string &fn : files) {
    iroha::IDesign *design = iroha::Iroha::ReadDesignFromFile(fn);
    if (design ==nullptr) {
      cerr << "Failed to read design from: " << fn << "\n";
      continue;
    }
    iroha::OptAPI *optimizer = iroha::Iroha::CreateOptimizer(design);
    bool has_opt_err = false;
    for (string &phase : opts) {
      if (!optimizer->ApplyPhase(phase)) {
	has_opt_err = true;
      }
    }
    if (has_opt_err) {
      cerr << "Failed to optimize the design: " << fn << "\n";
    }
    iroha::WriterAPI *writer = iroha::Iroha::CreateWriter(design);
    if (verilog) {
      writer->SetLanguage("verilog");
    }
    writer->Write(output);
  }
  return 0;
}
