#include <iostream>

#include "iroha/iroha.h"

void printVersion() {
  std::cout << PACKAGE << "-" << VERSION << "\n";
}

int main(int argc, char **argv) {
  vector<string> files;
  
  for (int i = 1; i < argc; ++i) {
    const string arg(argv[i]);
    if (arg == "--version") {
      printVersion();
      return 0;
    }
    files.push_back(arg);
  }

  iroha::Iroha::Init();

  for (string &fn : files) {
    iroha::IDesign *design = iroha::Iroha::ReadDesignFromFile(fn);
    iroha::WriterAPI *writer = iroha::Iroha::CreateWriter(design);
    writer->Write("/dev/stdout");
  }
  return 0;
}
