// -*- C++ -*-
#ifndef _iroha_insn_operands_h_
#define _iroha_insn_operands_h_

namespace iroha {
namespace operand {
const char kLeft[] = "left";
const char kRight[] = "right";
const char kRead[] = "read";
const char kWrite[] = "write";
const char kWait[] = "wait";
const char kNoWait[] = "no_wait";
const char kNotify[] = "notify";
const char kWaitNotify[] = "wait_notify";
const char kGetMailbox[] = "get_mailbox";
const char kPutMailbox[] = "put_mailbox";
const char kSramWrite[] = "sram_write";
const char kSramReadAddress[] = "sram_read_address";
const char kSramReadData[] = "sram_read_data";
}  // namespace operand
}  // namespace iroha

#endif  // _iroha_insn_operands_h_
