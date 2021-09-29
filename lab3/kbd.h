int (util_sys_inb) (int port, uint8_t *value);

int (kbd_subscribe_int) (uint8_t *bit_no);

int (kbd_unsubscribe_int) ();

void (kbd_ih) ();

int (kbd_enable) ();
