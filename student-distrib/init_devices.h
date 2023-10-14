/* PS/2 Controller Ports/ Commands*/
#define PS2_COMMAND_PORT    0x64 /* Command is sent */
#define PS2_DATA_PORT   0x60 /* Response byte is sent to this port, also inputs to the devices are sent to this port*/
#define READ_CTR_CONFIG 0x20
#define DISABLE_PS2_PORT1 0xAD
#define DISABLE_PS2_PORT2 0xA7
#define WRITE_PS2_DATA  0x60
#define TEST_PS2_CONTROLLER 0xAA
#define TEST_PS2_FIRST_PORT 0xAB
#define ENABLE_PS2_PORT1 0xAE
#define ENABLE_PS2_PORT2 0xA8
#define WRITE_PS2_PORT1

/* PS/2 Keyboard Commands */
#define RESET_DEVICE 0xFF




#define READ_PS2_OUTPUT 0xD0

extern int init_keyboard(void);