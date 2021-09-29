#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "kbd.h"
#include "kbd_macros.h"
#include "i8254.h"

extern uint8_t scancode;
extern unsigned int count_in;
extern unsigned int counter;
uint8_t bytes[2];

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  uint8_t bit_no;
  int ipc_status, r, irq_set = BIT(KEYBOARD_IRQ);
  message msg;
  bool ReadSecond = false;

  if (kbd_subscribe_int(&bit_no) != 0) {return 1;}

  while (scancode != ESC_BREAKCODE) {
    if ((r = driver_receive(ANY,&msg,&ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch(_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbd_ih();
            if (ReadSecond) {
              bytes[1] = scancode;
              if (scancode & BREAK_CODE) {
                kbd_print_scancode(0, 2, bytes);
              }
              else {
                kbd_print_scancode(1, 2, bytes);
              }
              ReadSecond = false;
            }
            else {
              if (scancode == DOUBLE_SCANCODE) {
                ReadSecond = true;
                bytes[0] = scancode;
              }
              else {
                bytes[0] = scancode;
                if (scancode & BREAK_CODE) {
                kbd_print_scancode(0, 1, bytes);
                }
                else {
                  kbd_print_scancode(1, 1, bytes);
                }
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (kbd_unsubscribe_int() != 0) {return 1;}

  if (kbd_print_no_sysinb(count_in) != 0) {return 1;}

  return 0;
}

int(kbd_test_poll)() {
  bool ReadSecond = false;
  
  while (scancode != ESC_BREAKCODE) {
    
    kbd_ih();
    if (ReadSecond) {
      bytes[1] = scancode;
      if (scancode & BREAK_CODE) {
        kbd_print_scancode(0, 2, bytes);
      }
      else {
        kbd_print_scancode(1, 2, bytes);
      }
      ReadSecond = false;
    }
    else {
      if (scancode == DOUBLE_SCANCODE) {
        ReadSecond = true;
        bytes[0] = scancode;
      }
      else {
        bytes[0] = scancode;
        if (scancode & BREAK_CODE) {
        kbd_print_scancode(0, 1, bytes);
        }
        else {
          kbd_print_scancode(1, 1, bytes);
        }
      }
    }
    tickdelay(micros_to_ticks(20000));          
  }

  if (kbd_enable() != 0) {return 1;}

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  
  uint8_t bit_no;
  int ipc_status, r, irq_set_kbd = BIT(KEYBOARD_IRQ), irq_set_timer = BIT(TIMER0_IRQ);
  message msg;
  bool ReadSecond = false;
  uint8_t idle = 0;

  if (kbd_subscribe_int(&bit_no) != 0) {return 1;}

  if (timer_subscribe_int(&bit_no) != 0) {return 1;}

  while ((scancode != ESC_BREAKCODE) && (idle != n)) {
    if ((r = driver_receive(ANY,&msg,&ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch(_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_kbd) {
            kbd_ih();
            if (ReadSecond) {
              bytes[1] = scancode;
              if (scancode & BREAK_CODE) {
                kbd_print_scancode(0, 2, bytes);
              }
              else {
                kbd_print_scancode(1, 2, bytes);
              }
              ReadSecond = false;
            }
            else {
              if (scancode == DOUBLE_SCANCODE) {
                ReadSecond = true;
                bytes[0] = scancode;
              }
              else {
                bytes[0] = scancode;
                if (scancode & BREAK_CODE) {
                kbd_print_scancode(0, 1, bytes);
                }
                else {
                  kbd_print_scancode(1, 1, bytes);
                }
              }
            }
            counter = 0;
          }
          if (msg.m_notify.interrupts & irq_set_timer) {
            timer_int_handler();
            if (counter % 60 == 0 && counter != 0) {
              idle++;
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (timer_unsubscribe_int() != 0) {return 1;}

  if (kbd_unsubscribe_int() != 0) {return 1;}

  return 0;
}
