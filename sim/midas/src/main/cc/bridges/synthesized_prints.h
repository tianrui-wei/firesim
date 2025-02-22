#ifndef __SYNTHESIZED_PRINTS_H
#define __SYNTHESIZED_PRINTS_H

#include <fstream>
#include <gmp.h>
#include <iostream>
#include <vector>

#include "bridge_driver.h"
#include "clock_info.h"

// Bridge Driver Instantiation Template
#define INSTANTIATE_PRINTF(FUNC, IDX)                                          \
  FUNC(new synthesized_prints_t(simif,                                         \
                                args,                                          \
                                PRINTBRIDGEMODULE_##IDX##_substruct_create,    \
                                PRINTBRIDGEMODULE_##IDX##_print_count,         \
                                PRINTBRIDGEMODULE_##IDX##_token_bytes,         \
                                PRINTBRIDGEMODULE_##IDX##_idle_cycles_mask,    \
                                PRINTBRIDGEMODULE_##IDX##_print_offsets,       \
                                PRINTBRIDGEMODULE_##IDX##_format_strings,      \
                                PRINTBRIDGEMODULE_##IDX##_argument_counts,     \
                                PRINTBRIDGEMODULE_##IDX##_argument_widths,     \
                                PRINTBRIDGEMODULE_##IDX##_to_cpu_stream_idx,   \
                                PRINTBRIDGEMODULE_##IDX##_to_cpu_stream_depth, \
                                PRINTBRIDGEMODULE_##IDX##_clock_domain_name,   \
                                PRINTBRIDGEMODULE_##IDX##_clock_multiplier,    \
                                PRINTBRIDGEMODULE_##IDX##_clock_divisor,       \
                                IDX));

struct print_vars_t {
  std::vector<mpz_t *> data;
  ~print_vars_t() {
    for (auto &e : data) {
      mpz_clear(*e);
      free(e);
    }
  }
};

typedef struct PRINTBRIDGEMODULE_struct {
  uint64_t startCycleL;
  uint64_t startCycleH;
  uint64_t endCycleL;
  uint64_t endCycleH;
  uint64_t doneInit;
  uint64_t flushNarrowPacket;
} PRINTBRIDGEMODULE_struct;

#ifdef PRINTBRIDGEMODULE_checks
PRINTBRIDGEMODULE_checks;
#endif // PRINTBRIDGEMODULE_checks

class synthesized_prints_t : public bridge_driver_t {

public:
  synthesized_prints_t(simif_t *sim,
                       const std::vector<std::string> &args,
                       const PRINTBRIDGEMODULE_struct &mmio_addrs,
                       unsigned int print_count,
                       unsigned int token_bytes,
                       unsigned int idle_cycles_mask,
                       const unsigned int *print_offsets,
                       const char *const *format_strings,
                       const unsigned int *argument_counts,
                       const unsigned int *argument_widths,
                       unsigned int stream_idx,
                       unsigned int stream_depth,
                       const char *const clock_domain_name,
                       const unsigned int clock_multiplier,
                       const unsigned int clock_divisor,
                       int printno);
  ~synthesized_prints_t();
  virtual void init();
  virtual void tick();
  virtual bool terminate() { return false; };
  virtual int exit_code() { return 0; };
  void flush();
  void finish() { flush(); };

private:
  const PRINTBRIDGEMODULE_struct mmio_addrs;
  const unsigned int print_count;
  const unsigned int token_bytes;
  const unsigned int idle_cycles_mask;
  const unsigned int *print_offsets;
  const char *const *format_strings;
  const unsigned int *argument_counts;
  const unsigned int *argument_widths;
  const unsigned int stream_idx;
  const unsigned int stream_depth;
  ClockInfo clock_info;
  const int printno;

  // Stream batching parameters
  static constexpr size_t beat_bytes = BridgeConstants::STREAM_WIDTH_BYTES;
  // The number of stream beats to pull off the FPGA on each invocation of
  // tick() This will be set based on the ratio of token_size :
  // desired_batch_beats
  size_t batch_beats;
  // This will be modified to be a multiple of the token size
  const size_t desired_batch_beats = stream_depth / 2;

  // Used to define the boundaries in the batch buffer at which we'll
  // initalize GMP types
  using gmp_align_t = uint64_t;
  const size_t gmp_align_bits = sizeof(gmp_align_t) * 8;

  // +arg driven members
  std::ofstream printfile; // Used only if the +print-file arg is provided
  std::string default_filename = "synthesized-prints.out";

  std::ostream *printstream; // Is set to std::cerr otherwise
  uint64_t start_cycle,
      end_cycle; // Bounds between which prints will be emitted
  uint64_t current_cycle = 0;
  bool human_readable = true;
  bool print_cycle_prefix = true;

  std::vector<std::vector<size_t>> widths;
  std::vector<size_t> sizes;
  std::vector<print_vars_t *> masks;

  std::vector<size_t> aligned_offsets; // Aligned to gmp_align_t
  std::vector<size_t> bit_offset;

  bool current_print_enabled(gmp_align_t *buf, size_t offset);
  size_t process_tokens(size_t beats, size_t minimum_batch_beats);
  void show_prints(char *buf);
  void print_format(const char *fmt, print_vars_t *vars, print_vars_t *masks);
  // Returns the number of beats available, once two successive reads return the
  // same value
  int beats_avaliable_stable();
};

#endif //__SYNTHESIZED_PRINTS_H
