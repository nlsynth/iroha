// -*- C++ -*-
//
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
//
// Main APIs and structures are defined (or included from this file).
//
#ifndef _iroha_iroha_h_
#define _iroha_iroha_h_

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/common.h"
#include "iroha/i_design.h"
#include "iroha/insn_operands.h"
#include "iroha/opt_api.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/writer_api.h"

namespace iroha {

// High level APIs and factory methods.
class Iroha {
public:
  static void Init();
  static void SetImportPaths(const vector<string> &paths);
  static IDesign *ReadDesignFromFile(const string &fn);
  static WriterAPI *CreateWriter(IDesign *design);
  static OptAPI *CreateOptimizer(IDesign *design);
};

}  // namespace iroha

#endif  // _iroha_iroha_h_
