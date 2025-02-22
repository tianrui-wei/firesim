
#ifndef __PLUSARGS_H
#define __PLUSARGS_H
// See LICENSE for license details.

#include "bridges/bridge_driver.h"
#include <gmp.h>
#include <string.h>
#include <string_view>

typedef struct PLUSARGSBRIDGEMODULE_struct {
  uint64_t initDone;
} PLUSARGSBRIDGEMODULE_struct;

#ifdef PLUSARGSBRIDGEMODULE_checks
PLUSARGSBRIDGEMODULE_checks;
#endif // PLUSARGSBRIDGEMODULE_checks

#define INSTANTIATE_PLUSARGS(FUNC, IDX)                                        \
  FUNC(new plusargs_t(simif,                                                   \
                      args,                                                    \
                      PLUSARGSBRIDGEMODULE_##IDX##_substruct_create,           \
                      PLUSARGSBRIDGEMODULE_##IDX##_name,                       \
                      PLUSARGSBRIDGEMODULE_##IDX##_default,                    \
                      PLUSARGSBRIDGEMODULE_##IDX##_width,                      \
                      PLUSARGSBRIDGEMODULE_##IDX##_slice_count,                \
                      PLUSARGSBRIDGEMODULE_##IDX##_slice_addrs));

/**
 * @brief PlusArgs Bridge Driver class
 *
 * This Bridge Driver talks to a PlusArgs Bridge. This class will determine
 * if the default, or overridden PlusArg value shoud be driven.
 *
 * Arbitrary wide bit widths are supported via
 * MPFR.
 */
class plusargs_t : public bridge_driver_t {
public:
  plusargs_t(simif_t *sim,
             const std::vector<std::string> &args,
             const PLUSARGSBRIDGEMODULE_struct &mmio_addrs,
             const std::string_view name,
             const char *default_value,
             const uint32_t bit_width,
             const uint32_t slice_count,
             const uint32_t *slice_addrs);
  ~plusargs_t();
  virtual void init();
  virtual void tick(){};
  virtual void finish(){};
  virtual bool terminate() { return false; };
  virtual int exit_code() { return 0; };
  uint32_t slice_address(const uint32_t idx);
  bool get_overridden();

private:
  const PLUSARGSBRIDGEMODULE_struct mmio_addrs;
  mpz_t value;            // either the default or the PlusArg value
  bool overriden = false; // true if the PlusArg was found and parsed
  const uint32_t slice_count = 0;
  const std::vector<uint32_t> slice_addrs;
};

#endif //__PLUSARGS_H
