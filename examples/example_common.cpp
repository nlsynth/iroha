#include "example_common.h"

#include "iroha/iroha.h"

using namespace iroha;

static string output;
static bool reset_polarity = true;

void example_init(int argc, char **argv) {
  for (int i = 1; i < argc; ++i) {
    string arg(argv[i]);
    if (string(argv[i - 1]) == "-o") {
      output = argv[i];
    }
    if (arg == "-n") {
      reset_polarity = false;
    }
    if (arg == "-p") {
      reset_polarity = true;
    }
  }
  Iroha::Init();
}

void example_write(IDesign *design) {
  design->GetParams()->SetResetPolarity(reset_polarity);
  iroha::WriterAPI *writer = Iroha::CreateWriter(design);
  if (output.empty()) {
    writer->SetLanguage("");
    writer->Write("/dev/stdout");
  } else {
    if (output.find(".v") == output.size() - 2) {
      writer->SetLanguage("verilog");
    } else {
      writer->SetLanguage("");
    }
    writer->Write(output);
  }
  delete writer;
}
