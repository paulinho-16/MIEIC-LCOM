#define KEYBOARD_IRQ 1

#define STAT_REG 0x64

#define OUT_BUF 0x60

#define PAR_ERROR BIT(7)

#define TO_ERROR BIT(6)

#define OBF BIT(0)

#define IBF BIT(1)

#define ESC_BREAKCODE 0x81

#define BREAK_CODE BIT(7)

#define DOUBLE_SCANCODE 0xE0

#define READ_CMDB 0x20

#define WRITE_CMDB 0x60
