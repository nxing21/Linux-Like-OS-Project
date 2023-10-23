#define RTC_REGISTER_SELECT 0x70 /* Selects a register in the RTC space */
#define RTC_REGISTER_DATA_PORT 0x71 /* Port that allows read/write to registers*/

#include "lib.h"
#include "nmi.h"
#include "i8259.h"

/* Registers A-C of the RTC */
#define RTC_REG_A   0x0A
#define RTC_REG_B   0x0B
#define RTC_REG_C   0x0C

#define rtc_max_frequency           32768
#define rtc_max_usable_frequency    1024
#define rtc_min_frequency           2
#define rtc_lowest_rate             6
#define rtc_highest_rate            15

/* Initializes the RTC. */
void init_RTC();

/* Sets the RTC to a frequency. */
void set_RTC_frequency(uint32_t rate);

/* Handles RTC interrupts */
void RTC_handler();

/* Initialize the RTC frequency to 2 Hz */
int RTC_open(const uint8_t* filename);

/* Does nothing unless we virtualize */
int RTC_close(int32_t fd);

/* Blocks RTC until next interrupt */
int RTC_read(int32_t fd, void* buffer, int32_t nbytes);

/* Changes the RTC frequency */
int RTC_write(int32_t fd, const void* buffer, int32_t nbytes);

