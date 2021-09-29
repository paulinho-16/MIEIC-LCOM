int (vbe_set_mode) (uint16_t mode);

int (vbe_get_mode) (uint16_t mode, vbe_mode_info_t *mode_info);

int (vg_draw_rectangle) (uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

int (vg_draw_hline) (uint16_t x, uint16_t y, uint16_t len, uint32_t color);

int (changePixelColor) (uint16_t x, uint16_t y, uint32_t color);

uint32_t (indexed_color) (uint8_t no_rectangles, uint32_t first, uint8_t step,uint16_t row, uint16_t col);

uint32_t (direct_color) (uint32_t red, uint32_t green, uint32_t blue);

uint32_t (R_First)(uint32_t first);

uint32_t (G_First)(uint32_t first);

uint32_t (B_First)(uint32_t first);

uint32_t (R)(uint16_t row, uint16_t col, uint32_t first, uint32_t step);

uint32_t (G)(uint16_t row, uint32_t first, uint32_t step);

uint32_t (B)(uint16_t row, uint16_t col, uint32_t first, uint32_t step);

int (controller_info)(vg_vbe_contr_info_t * info);

