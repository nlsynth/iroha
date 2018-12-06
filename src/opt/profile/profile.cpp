#include "opt/profile/profile.h"

#include "iroha/i_design.h"

namespace iroha {
namespace opt {
namespace profile {

bool Profile::HasProfile(IDesign *design) {
  for (IModule *mod : design->modules_) {
    for (ITable *tab : mod->tables_) {
      for (IState *st : tab->states_) {
	const IProfile &prof = st->GetProfile();
	if (prof.valid_) {
	  return true;
	}
      }
    }
  }
  return false;
}

void Profile::FillFakeProfile(IDesign *design) {
  for (IModule *mod : design->modules_) {
    for (ITable *tab : mod->tables_) {
      for (IState *st : tab->states_) {
	IProfile *prof = st->GetMutableProfile();
	prof->valid_ = true;
	prof->raw_count_ = 1;
	prof->normalized_count_ = 0;
      }
    }
  }
}

void Profile::ClearProfile(IDesign *design) {
  for (IModule *mod : design->modules_) {
    for (ITable *tab : mod->tables_) {
      for (IState *st : tab->states_) {
	IProfile *prof = st->GetMutableProfile();
	prof->valid_ = false;
      }
    }
  }
}

void Profile::NormalizeProfile(IDesign *design) {
  long raw_total = 0;
  for (IModule *mod : design->modules_) {
    for (ITable *tab : mod->tables_) {
      for (IState *st : tab->states_) {
	const IProfile &prof = st->GetProfile();
	if (prof.valid_) {
	  raw_total += prof.raw_count_;
	}
      }
    }
  }
  for (IModule *mod : design->modules_) {
    for (ITable *tab : mod->tables_) {
      for (IState *st : tab->states_) {
	IProfile *prof = st->GetMutableProfile();
	if (prof->valid_) {
	  prof->normalized_count_ =
	    GetNormalizedCount(prof->raw_count_, raw_total);
	} else {
	  prof->valid_ = true;
	  prof->raw_count_ = 1;
	  prof->normalized_count_ = 1;
	}
      }
    }
  }
}

long Profile::GetNormalizedCount(long raw_count, long raw_total) {
  // Just in case to avoid 0 division.
  if (raw_total == 0) {
    raw_total = 1;
  }
  long c = raw_count * 1000000 / raw_total;
  int s;
  for (s = 1; (1 << s) < c; ++s) {
    // do nothing.
  }
  return s;
}

}  // namespace profile
}  // namespace opt
}  // namespace iroha
