// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "graphics_card.h"
#include "utils3.h"
#include "i8042.h"

extern vbe_mode_info_t mode_info;
extern uint8_t scancode;
extern unsigned int hres, vres;
extern unsigned int counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  if (vbe_set_mode(mode) != 0) {return 1;}

  sleep(delay);

  if (vg_exit() != 0) {return 1;}

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  if (vbe_set_mode(mode) != 0) {return 1;}

  uint8_t bit_no = 0;
  int ipc_status,r;
  message msg;
  uint32_t irq_set = BIT(1);
  uint8_t bytes[2];
  bool ReadSecond = false;

  if (vg_draw_rectangle(x,y,width,height,color) != 0) {return 1;}

  if (kbd_subscribe_int(&bit_no)!=0) {return 1;}

  while(scancode != ESC){
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (ReadSecond) {
              ReadSecond = false;
              bytes[1] = scancode;
            }
            else {
              bytes[0] = scancode;
              if (scancode == SIZE) {
                ReadSecond = true;
              }
            }
            
          }
          break;
        default:
          break; 
      }
    }
  }
  
  if (kbd_unsubscribe_int()!=0) {return 1;}

  if (vg_exit() != 0) {return 1;}

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  
  uint8_t bit_no = 0;
  int ipc_status,r;
  message msg;
  uint32_t irq_set = BIT(1);
  uint8_t bytes[2];
  bool ReadSecond = false;
  uint32_t red, green, blue;

  if (vbe_set_mode(mode) != 0) {return 1;}

  uint16_t width = hres / no_rectangles;
  uint16_t height = vres / no_rectangles;
  uint16_t comprimento_extra = hres % no_rectangles;
  uint16_t altura_extra  = vres % no_rectangles;

  for (unsigned int c = 0 ; c < no_rectangles ; c++) {
    if (c*height >= vres-altura_extra) break;
    for (unsigned int a = 0 ; a < no_rectangles ; a++) {
      if (a*width >= hres-comprimento_extra) break;
      if (mode_info.MemoryModel != 0x06)
        vg_draw_rectangle(a*width,c*height,width,height,indexed_color(no_rectangles,first,step,c,a));
      else {
        red = R(c,a,first,step);
        green = G(c,first,step);
        blue = B(c,a,first,step);
        vg_draw_rectangle(a*width,c*height, width, height, direct_color(red,green,blue));
      }
    }
  }

  if (kbd_subscribe_int(&bit_no)!=0) {return 1;}

  while(scancode != ESC){
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (ReadSecond) {
              ReadSecond = false;
              bytes[1] = scancode;
            }
            else {
              bytes[0] = scancode;
              if (scancode == SIZE) {
                ReadSecond = true;
              }
            }
            
          }
          break;
        default:
          break; 
      }
    }
  }
  
  if (kbd_unsubscribe_int()!=0) {return 1;}

  if (vg_exit() != 0) {return 1;}

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  uint8_t bit_no = 0;
  int ipc_status,r;
  message msg;
  uint32_t irq_set = BIT(1);
  uint8_t bytes[2];
  bool ReadSecond = false;

  if (vbe_set_mode(0x105) != 0) {return 1;}

  xpm_image_t img;
  uint8_t* map;
  map = xpm_load(xpm,XPM_INDEXED,&img);

  for (unsigned int a = 0 ; a < img.height ; a++) {
    for (unsigned int w = 0 ; w < img.width ; w++) {
      changePixelColor(w + x, a + y,*map);
      map++;
    }
  }

  if (kbd_subscribe_int(&bit_no)!=0) {return 1;}

  while(scancode != ESC){
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (ReadSecond) {
              ReadSecond = false;
              bytes[1] = scancode;
            }
            else {
              bytes[0] = scancode;
              if (scancode == SIZE) {
                ReadSecond = true;
              }
            }
            
          }
          break;
        default:
          break; 
      }
    }
  }
  
  if (kbd_unsubscribe_int()!=0) {return 1;}

  if (vg_exit() != 0) {return 1;}

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {

  uint8_t bit_no = 0;
  int ipc_status,r;
  message msg;
  uint32_t irq_set_kbd = BIT(1), irq_set_timer = BIT(0);
  uint8_t bytes[2];
  bool ReadSecond = false;

  if (vbe_set_mode(0x105) != 0) {return 1;}

  xpm_image_t img;
  uint8_t* map;
  map = xpm_load(xpm,XPM_INDEXED,&img);

  for (unsigned int a = 0 ; a < img.height ; a++) {
    for (unsigned int w = 0 ; w < img.width ; w++) {
      changePixelColor(w + xi, a + yi,*map);
      map++;
    }
  }

  int time_frame = sys_hz()/fr_rate;
  uint16_t x_novo = xi, y_novo = yi;
  int frame_counter = 0;

  if (kbd_subscribe_int(&bit_no) != 0) {return 1;}
  if (timer_subscribe_int(&bit_no) != 0) {return 1;}

  while(scancode != ESC){
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 
          if (msg.m_notify.interrupts & irq_set_kbd) {
            kbc_ih();
            if (ReadSecond) {
              ReadSecond = false;
              bytes[1] = scancode;
            }
            else {
              bytes[0] = scancode;
              if (scancode == SIZE) {
                ReadSecond = true;
              }
            }
          }
          if (msg.m_notify.interrupts & irq_set_timer && (x_novo != xf || y_novo != yf)) {
            timer_int_handler();
            if (counter % time_frame == 0) {
              if (speed > 0) {
                vg_draw_rectangle(x_novo,y_novo,img.width,img.height,0);
                if (xi == xf) {
                  if (yi < yf) {
                    if (y_novo + speed > yf)
                      y_novo = yf;
                    else
                      y_novo = y_novo + speed;
                  }
                  else {
                    if (y_novo - speed < yf)
                      y_novo = yf;
                    else
                      y_novo = y_novo - speed;
                  }
                }
                else {
                  if (xi < xf) {
                    if (x_novo + speed > xf)
                      x_novo = xf;
                    else  
                      x_novo = x_novo + speed;
                  }
                  else {
                    if (x_novo - speed < xf)
                      x_novo = xf;
                    else  
                      x_novo = x_novo - speed;
                  }
                }
                map = xpm_load(xpm,XPM_INDEXED,&img);
                for (unsigned int height = 0 ; height < img.height ; height++) {
                  for (unsigned int width = 0 ; width < img.width; width++) {
                    changePixelColor(x_novo + width,y_novo + height,*map);
                    map++;
                  }
                }
              }
              else {
                frame_counter++;
                if (frame_counter % abs(speed) == 0) {
                  vg_draw_rectangle(x_novo,y_novo,img.width,img.height,0);
                  if (xi == xf) {
                    if (yi < yf) {
                      if (y_novo + 1 > yf)
                        y_novo = yf;
                      else
                        y_novo = y_novo + 1;
                    }
                    else {
                      if (y_novo - 1 < yf)
                        y_novo = yf;
                      else
                        y_novo = y_novo - 1;
                    }
                  }
                  else {
                    if (xi < xf) {
                      if (x_novo + 1 > xf)
                        x_novo = xf;
                      else  
                        x_novo = x_novo + 1;
                    }
                    else {
                      if (x_novo - speed < xf)
                        x_novo = xf;
                      else  
                        x_novo = x_novo - 1;
                    }
                  }
                  map = xpm_load(xpm,XPM_INDEXED,&img);
                  for (unsigned int height = 0 ; height < img.height ; height++) {
                    for (unsigned int width = 0 ; width < img.width; width++) {
                      changePixelColor(x_novo + width,y_novo + height,*map);
                      map++;
                    }
                  }
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
  
  if (timer_unsubscribe_int() != 0) {return 1;}
  if (kbd_unsubscribe_int()!=0) {return 1;}

  if (vg_exit() != 0) {return 1;}

  return 0;
}

int(video_test_controller)() {

  vg_vbe_contr_info_t info;

  controller_info(&info);

  vg_display_vbe_contr_info(&info);

  return 0;
}
