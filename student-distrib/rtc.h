#define RTC_REGISTER_SELECT 0x70 /* Selects a register in the RTC space */
#define RTC_REGISTER_DATA_PORT 0x71 /* Port that allows read/write to registers*/

/* Registers A-C of the RTC */
#define RTC_REG_A   0x0A
#define RTC_REG_B   0x0B
#define RTC_REG_C   0x0C

#define rtc_max_frequency 32768

/* Initializes the RTC. */
extern void init_RTC();

/* Sets the RTC to a VIRTUAL frequency. */
extern void set_RTC_frequency();

/* Handles RTC interrupts */
extern void RTC_handler();