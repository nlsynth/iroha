// Copyright (c) 2015-2017, Yusuke Tabata (tabata.yusuke@gmail.com) and the team.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the developers nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "iroha/iroha.h"

#include "builder/exp_builder.h"
#include "design/design_tool.h"
#include "opt/optimizer.h"
#include "writer/writer.h"

namespace iroha {

OptAPI::~OptAPI() {
}

WriterAPI::~WriterAPI() {
}

void Iroha::Init() {
  opt::Optimizer::Init();
}

void Iroha::SetImportPaths(const vector<string> &paths) {
  Util::SetImportPaths(paths);
}

IDesign *Iroha::ReadDesignFromFile(const string &fn) {
  return builder::ExpBuilder::ReadDesign(fn);
}

WriterAPI *Iroha::CreateWriter(IDesign *design) {
  WriterAPI *writer = design->GetWriterAPI();
  if (writer == nullptr) {
    writer = new writer::Writer(design);
    design->SetWriterAPI(writer);
  }
  return writer;
}

OptAPI *Iroha::CreateOptimizer(IDesign *design) {
  OptAPI *opt = design->GetOptAPI();
  if (opt == nullptr) {
    opt = new opt::Optimizer(design);
    design->SetOptAPI(opt);
  }
  return opt;
}

}  // namespace iroha
