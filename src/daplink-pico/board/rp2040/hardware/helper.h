#include <stdint.h>
#include <assert.h>

// A non-functional (empty) helper macro to help IDEs follow links from the autogenerated
// hardware struct headers in hardware/structs/xxx.h to the raw register definitions
// in hardware/regs/xxx.h. A preprocessor define such as TIMER_TIMEHW_OFFSET (a timer register offset)
// is not generally clickable (in an IDE) if placed in a C comment, so _REG_(TIMER_TIMEHW_OFFSET) is
// included outside of a comment instead
#define _REG_(x)

typedef volatile uint32_t io_rw_32;
typedef const volatile uint32_t io_ro_32;

#define XIP_SSI_BASE _u(0x18000000)
