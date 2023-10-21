#define RTC_REGISTER_SELECT 0x70 /* Selects a register in the RTC space */
#define RTC_REGISTER_DATA_PORT 0x71 /* Port that allows read/write to registers*/

#include "lib.h"
#include "nmi.h"
#include "i8259.h"

/* Registers A-C of the RTC */
#define RTC_REG_A   0x0A
#define RTC_REG_B   0x0B
#define RTC_REG_C   0x0C

#define rtc_max_frequency 32768

/* Initializes the RTC. */
void init_RTC();

/* Sets the RTC to a VIRTUAL frequency. */
void set_RTC_frequency(uint8_t rate);

/* Handles RTC interrupts */
void RTC_handler();

/* Initialize the RTC frequency to 2 Hz */
int open(const char* filename);

/* Does nothing unless we virtualize */
int close(uint32_t fd);

/* Blocks RTC until next interrupt */
int read(uint32_t fd, void* buffer, int nbytes);

/* Changes the RTC frequency */
int write(uint32_t fd, void* buffer, int nbytes);

