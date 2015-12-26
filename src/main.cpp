#include <iostream>

#include "iroha/iroha.h"

void printVersion() {
  std::cout << PACKAGE << "-" << VERSION << "\n";
}

int main(int argc, char **argv) {
  vector<string> files;
  bool verilog = false;

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
    files.push_back(arg);
  }

  iroha::Iroha::Init();

  for (string &fn : files) {
    iroha::IDesign *design = iroha::Iroha::ReadDesignFromFile(fn);
    if (design ==nullptr) {
      cout << "Failed to read design from: " << fn << "\n";
      continue;
    }
    iroha::WriterAPI *writer = iroha::Iroha::CreateWriter(design);
    if (verilog) {
      writer->SetLanguage("verilog");
    }
    writer->Write("/dev/stdout");
  }
  return 0;
}
