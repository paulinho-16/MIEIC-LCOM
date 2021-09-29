#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

static int hook_id;
unsigned int counter = 0;
uint8_t status;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  /*
    Como queremos um número de 16 bits, se freq for menor que 19, TIMER_FREQ / freq vai dar um número que é maior que o máximo representável em 16 bits (2^16 = 65536), logo, freq deve ser maior que 19, e menor que o valor de TIMER_FREQ.
  */
  
  if (freq < 19 || freq > TIMER_FREQ) {
    return 1;
  }

  uint8_t nova_freq_lsb, nova_freq_msb;
  uint16_t nova_freq = TIMER_FREQ / freq;
  util_get_LSB(nova_freq, &nova_freq_lsb);
  util_get_MSB(nova_freq, &nova_freq_msb);

  if (timer_get_conf(timer, &status) != 0) {return 1;}

  uint8_t novo_status = status & 0x0F;

  novo_status = novo_status | TIMER_LSB_MSB;

  if (timer == 0) {
    if (sys_outb(TIMER_CTRL, novo_status) != 0) {return 1;}
    if (sys_outb(TIMER_0, nova_freq_lsb) != 0) {return 1;}
    if (sys_outb(TIMER_0, nova_freq_msb) != 0) {return 1;}
  }
  else if (timer == 1) {
    novo_status = novo_status | BIT(6);
    if (sys_outb(TIMER_CTRL, novo_status) != 0) {return 1;}
    if (sys_outb(TIMER_1, nova_freq_lsb) != 0) {return 1;}
    if (sys_outb(TIMER_1, nova_freq_msb) != 0) {return 1;}
  }
  else if (timer == 2) {
    novo_status = novo_status | BIT(7);
    if (sys_outb(TIMER_CTRL, novo_status) != 0) {return 1;}
    if (sys_outb(TIMER_2, nova_freq_lsb) != 0) {return 1;}
    if (sys_outb(TIMER_2, nova_freq_msb) != 0) {return 1;}
  }
  else {
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  hook_id = 0;
  *bit_no = hook_id;
  if (sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE,&hook_id) != 0) {return 1;}

  return 0;
}

int (timer_unsubscribe_int)() {

  if (sys_irqrmpolicy(&hook_id) != 0) {return 1;}

  return 0;
}

void (timer_int_handler)() {
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  uint8_t rb_cmd = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));

  if (sys_outb(TIMER_CTRL,rb_cmd) != 0) {return 1;}

  if (timer == 0) {
    if (util_sys_inb(TIMER_0,st) != 0) {return 1;}
  }
  else if (timer == 1) {
    if (util_sys_inb(TIMER_1,st) != 0) {return 1;}
  }
  else if (timer == 2) {
    if (util_sys_inb(TIMER_2,st) != 0) {return 1;}
  }
  else {
    return 1;
  }
  
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  
  union timer_status_field_val conf;

  if (field == tsf_all) {
    conf.byte = st;
  }
  else if (field == tsf_initial) {
    if ((st & TIMER_LSB) && !(st & TIMER_MSB)) {
      conf.in_mode = LSB_only;
    }
    else if (!(st & TIMER_LSB) && (st & TIMER_MSB)) {
      conf.in_mode = MSB_only;
    }
    else if ((st & TIMER_LSB) && (st & TIMER_MSB)) {
      conf.in_mode = MSB_after_LSB;
    }
    else {
      conf.in_mode = INVAL_val;
    }
  }
  else if (field == tsf_mode) {
    if (!(st & BIT(1)) && !(st & BIT(2)) && !(st & BIT(3))) {
      conf.count_mode = 0;
      printf("\n1\n");
    }
    else if (!(st & BIT(3)) && !(st & BIT(2)) && (st & BIT(1))) {
      conf.count_mode = 1;
      printf("\n2\n");
    }
    else if (!(st & BIT(3)) && (st & BIT(2)) && !(st & BIT(1))) {
      conf.count_mode = 2;
      printf("\n3\n");
    }
    else if ((st & BIT(3)) && (st & BIT(2)) && !(st & BIT(1))) {
      conf.count_mode = 2;
      printf("\n4\n");
    }
    else if (!(st & BIT(3)) && (st & BIT(2)) && (st & BIT(1))) {
      conf.count_mode = 3;
      printf("\n5\n");
    }
    else if ((st & BIT(3)) && (st & BIT(2)) && (st & BIT(1))) {
      conf.count_mode = 3;
      printf("\n6\n");
    }
    else if ((st & BIT(3)) && !(st & BIT(2)) && !(st & BIT(1))) {
      conf.count_mode = 4;
      printf("\n7\n");
    }
    else if ((st & BIT(3)) && !(st & BIT(2)) && (st & BIT(1))) {
      conf.count_mode = 5;
      printf("\n8\n");
    }
    else {
      return 1;
    }
  }
  else if (field == tsf_base) {
    if (st & BIT(0)) {
      conf.bcd = true;
    }
    else {
      conf.bcd = false;
    }
  }
  else {
    return 1;
  }

  if (timer_print_config(timer, field, conf) != 0) {return 1;}

  return 0;
}
