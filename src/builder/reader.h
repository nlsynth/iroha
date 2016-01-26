// -*- C++ -*-
#ifndef _builder_reader_h_
#define _builder_reader_h_

#include "iroha/common.h"

namespace iroha {
namespace builder {

class Atom {
public:
  string str;
};

class Exp {
public:
  ~Exp();

  Atom atom;
  vector<Exp *> vec;
};

class File {
public:
  vector<Exp *> exps;
};

class Reader {
public:
  Reader(istream &ifs);

  File *Read();

  static File *ReadFile(const string &fn);
  static void DumpFile(File *s);
  static void DumpExp(Exp *e);

private:
  Exp *ReadExp();
  Exp *ReadList();
  bool EnsureLine();
  string ReadToken();
  void UnreadToken(string &t);
  const char *CurrentChar();
  void SetError();
  bool HasError();
  
  istream &ifs_;
  string unread_token_;
  string cur_line_;
  int column_;
  bool has_error_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_reader_h_
