#include "builder/reader.h"

#include <fstream>
#include <ctype.h>

namespace iroha {

Exp::~Exp() {
  for (Exp *e : vec) {
    if (e) {
      delete e;
    }
  }
}

File *Reader::ReadFile(const string &fn) {
  std::ifstream *ifs = new std::ifstream(fn);
  std::unique_ptr<Reader> reader(new Reader(ifs));
  return reader->Read();
}

void Reader::DumpFile(File *f) {
  if (f->exps.size()) {
    for (Exp *e : f->exps) {
      DumpExp(e);
      cout << "\n\n";
    }
  } else {
    cout << "Read failure.\n";
  }
}

void Reader::DumpExp(Exp *s) {
  if (!s->atom.str.empty()) {
    cout << s->atom.str;
    return;
  }
  cout << "(";
  bool is_first = true;
  for (Exp *elm : s->vec) {
    if (!is_first) {
      cout << " ";
    }
    DumpExp(elm);
    is_first = false;
  }
  cout << ")";
}

Reader::Reader(istream *ifs) : column_(0), has_error_(false) {
  ifs_.reset(ifs);
}

File *Reader::Read() {
  File *f = new File;
  string s = ReadToken();
  while (!s.empty()) {
    UnreadToken(s);
    Exp *e = ReadExp();
    if (HasError()) {
      delete e;
      for (Exp *cur : f->exps) {
	delete cur;
      }
      f->exps.clear();
      break;
    } else {
      f->exps.push_back(e);
    }
    s = ReadToken();
  }
  return f;
}

Exp *Reader::ReadExp() {
  if (HasError()) {
    return nullptr;
  }
  string token = ReadToken();
  if (token.empty()) {
    SetError();
    return nullptr;
  }
  if (token == "(") {
    return ReadList();
  }
  Exp *s = new Exp;
  s->atom.str = token;
  return s;
}

Exp *Reader::ReadList() {
  if (HasError()) {
    return nullptr;
  }
  Exp *lst = new Exp;
  while (true) {
    string token = ReadToken();
    if (token == ")") {
      break;
    }
    if (token.empty()) {
      SetError();
      break;
    }
    UnreadToken(token);
    lst->vec.push_back(ReadExp());
  }
  return lst;
}

bool Reader::EnsureLine() {
 again:
  if (column_ == cur_line_.size()) {
    if (ifs_->eof()) {
      return false;
    }
    std::getline(*ifs_, cur_line_);
    column_ = 0;
    if (cur_line_.empty()) {
      goto again;
    }
  }
  return true;
}

string Reader::ReadToken() {
  if (!unread_token_.empty()) {
    return std::move(unread_token_);
  }
 again:
  if (!EnsureLine()) {
    return string();
  }
  // Skip spaces.
  const char *c = CurrentChar();
  while (*c > 0 && isspace(*c)) {
    ++column_;
    if (column_ == cur_line_.size()) {
      goto again;
    }
    c = CurrentChar();
  }
  if (*c == ';') {
    column_ = cur_line_.size();
    goto again;
  }
  // Seek the end of token.
  const char *start = c;
  bool in_quote = (*start == '"');
  int len = 0;
  c = CurrentChar();
  while (column_ < cur_line_.size()) {
    if (in_quote) {
      if (*c == '"' && len > 0) {
	++column_;
	++len;
	break;
      }
    } else if (*c > 0 && isspace(*c)) {
      break;
    }
    bool one_char = (*c == '(' || *c == ')');
    if (one_char && !in_quote && len > 0) {
      break;
    }
    ++column_;
    ++len;
    if (one_char && len == 1) {
      break;
    }
    c = CurrentChar();
  }
  return string(start, len);
}

void Reader::UnreadToken(string &t) {
  unread_token_ = t;
}

const char *Reader::CurrentChar() {
  return cur_line_.c_str() + column_;
}

void Reader::SetError() {
  has_error_ = true;
}

bool Reader::HasError() {
  return has_error_;
}

}  // namespace iroha
