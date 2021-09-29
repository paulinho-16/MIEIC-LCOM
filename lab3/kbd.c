#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "kbd.h"
#include "kbd_macros.h"

int hook_id;
uint8_t status;
uint8_t scancode;
unsigned int count_in = 0;

int (util_sys_inb) (int port, uint8_t *value) {
    uint32_t temp;
    if (sys_inb(port,&temp) != 0) {return 1;}
    *value = (uint8_t) temp;
    count_in++;
    return 0;
}

int (kbd_subscribe_int) (uint8_t *bit_no) {

    hook_id = 1;
    *bit_no = hook_id;
    if (sys_irqsetpolicy(KEYBOARD_IRQ,IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != 0) {return 1;}
    return 0;
}

int (kbd_unsubscribe_int) () {
    if (sys_irqrmpolicy(&hook_id) != 0) {return 1;}
    return 0;
}

void (kbd_ih) () {
    while (1) {
        if (util_sys_inb(STAT_REG,&status) != 0) {return;}
        if (status & OBF) {
            if (status & (PAR_ERROR | TO_ERROR)) {return;}
            if (util_sys_inb(OUT_BUF,&scancode) != 0) {return;}
            break;
        }
    }
}

int (kbd_enable) () {

    uint8_t cmd_byte;
    if (sys_outb(STAT_REG,READ_CMDB) != 0) {return 1;}
    if (util_sys_inb(OUT_BUF, &cmd_byte) != 0) {return 1;}
    cmd_byte = cmd_byte | BIT(0);
    
    while (1) {
        if (util_sys_inb(STAT_REG, &status) != 0) {return 1;}
        if ((status & IBF) == 0) {
            if (sys_outb(STAT_REG,WRITE_CMDB) != 0) {return 1;}
            if (sys_outb(OUT_BUF,cmd_byte) != 0) {return 1;}
            return 0;
        }
        tickdelay(micros_to_ticks(20000));
    }
}
