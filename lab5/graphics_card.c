// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include "graphics_card.h"

mmap_t mem_map;
vbe_mode_info_t mode_info;
unsigned int vram_base, vram_size;
unsigned int hres, vres;
unsigned int numBytesperPixel;
struct minix_mem_range mr;
void *video_mem;

phys_bytes (far_pointer)(phys_bytes pp) {
  phys_bytes msb = (pp & 0xFFFF0000) >> 12;
  phys_bytes lsb = pp & 0xFFFF;
  return (phys_bytes) mem_map.virt + (msb | lsb) - (phys_bytes) mem_map.phys;
}

#pragma pack(1)
typedef struct 
{
  char VbeSignature[4];
  BCD VbeVersion[2];
  phys_bytes OemStringPtr;
  uint32_t Capabilities;
  phys_bytes VideoModePtr;
  uint16_t TotalMemory;

  BCD OemSoftwareRev[2];
  phys_bytes OemVendorNamePtr;
  phys_bytes OemProductNamePtr;
  phys_bytes OemProductRevPtr;
  char Reserved[222];
  char OemData[256];

} VbeInfoBlock;
#pragma options align=reset

int (vbe_get_mode) (uint16_t mode, vbe_mode_info_t *mode_info) {
  lm_alloc(sizeof(vbe_mode_info_t),&mem_map);
  reg86_t reg;
  memset(&reg,0,sizeof(reg86_t));
  reg.ax = 0x4F01;
  reg.es = PB2BASE(mem_map.phys);
  reg.di = PB2OFF(mem_map.phys);
  reg.cx = mode;
  reg.intno = 0x10;
  if (sys_int86(&reg) != OK) {return 1;}

  memcpy(mode_info,mem_map.virt,mem_map.size);
  lm_free(&mem_map);
  return 0;
}

int (vbe_set_mode) (uint16_t mode) {

int r;
mem_map.phys = 0;
mem_map.size = 1024*1024;
lm_alloc(mem_map.size,&mem_map);

if (vbe_get_mode(mode,&mode_info) != 0) {return 1;}

vram_base = mode_info.PhysBasePtr;
vram_size = mode_info.XResolution * mode_info.YResolution * ((mode_info.BitsPerPixel + 7)/8);
hres = mode_info.XResolution;
vres = mode_info.YResolution;
numBytesperPixel = (mode_info.BitsPerPixel + 7) / 8;

mr.mr_base = (phys_bytes) vram_base;
mr.mr_limit = mr.mr_base + vram_size;

if (OK != (r = sys_privctl(SELF,SYS_PRIV_ADD_MEM,&mr)))
  panic("sys_privctl (ADD_MEM) failed: %d\n", r);

video_mem = vm_map_phys(SELF,(void *)mr.mr_base, vram_size);

if (video_mem == MAP_FAILED)
  panic("couldn't map video memory");

reg86_t r86;

memset(&r86, 0, sizeof(reg86_t));	/* zero the structure */

r86.ax = 0x4F02;
r86.bx = 1<<14|mode; // set bit 14: linear framebuffer
r86.intno = 0x10;

if (sys_int86(&r86) != OK ) {
  printf("set_vbe_mode: sys_int86() failed \n");
  return 1;
}

lm_free(&mem_map);

return 0;
}

int (vg_draw_rectangle) (uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  for (unsigned int j = 0 ; j < height ; j++) {
    if (vg_draw_hline(x,y+j,width,color) != 0) {return 1;}
  }

  return 0;
}

int (vg_draw_hline) (uint16_t x, uint16_t y, uint16_t len, uint32_t color) {

  for (unsigned int i = 0 ; i < len ; i++) {
    if (changePixelColor(x+i,y,color) != 0) {return 1;}
  }

  return 0;
}

int (changePixelColor) (uint16_t x, uint16_t y, uint32_t color) {

  uint8_t *ptr;
  uint8_t temp;
  ptr = (uint8_t *)video_mem + x*numBytesperPixel + hres*y*numBytesperPixel;

  for (unsigned int i = 0 ; i < numBytesperPixel ; i++) {
    temp = color & 0xFF;
    *(ptr+i) = temp;
    color  = color >> 8;
  }
  return 0;
}

uint32_t (indexed_color) (uint8_t no_rectangles, uint32_t first, uint8_t step,uint16_t row, uint16_t col) {
  uint32_t color;
  color = (first + (row*no_rectangles + col) * step) % (1 << mode_info.BitsPerPixel);
  return color;
}

uint32_t (direct_color) (uint32_t red, uint32_t green, uint32_t blue) {
  uint32_t color;
  color = blue | (green << mode_info.GreenFieldPosition) | (red << mode_info.RedFieldPosition);
  return color;
}

uint32_t (R_First)(uint32_t first) {
  return (first >> mode_info.RedFieldPosition) & ((1 << mode_info.RedMaskSize) - 1);
}

uint32_t (G_First)(uint32_t first) {
  return (first >> mode_info.GreenFieldPosition) & ((1 << mode_info.GreenMaskSize) - 1);
}

uint32_t (B_First)(uint32_t first) {
  return (first >> mode_info.BlueFieldPosition) & ((1 << mode_info.BlueMaskSize) - 1);
}

uint32_t (R)(uint16_t row, uint16_t col, uint32_t first, uint32_t step) {
  uint32_t color;
  color = (R_First(first) + col*step) % (1 << mode_info.RedMaskSize);
  return color;
}

uint32_t (G)(uint16_t row, uint32_t first, uint32_t step) {
  uint32_t color;
  color = (G_First(first) + row*step) % (1 << mode_info.GreenMaskSize);
  return color;
}

uint32_t (B)(uint16_t row, uint16_t col, uint32_t first, uint32_t step) {
  uint32_t color;
  color = (B_First(first) + (col+row)*step) % (1 << mode_info.BlueMaskSize);
  return color;
}

int (controller_info)(vg_vbe_contr_info_t * info) {

  VbeInfoBlock v;
  VbeInfoBlock *vbeBlock = &v;
  lm_alloc(sizeof(VbeInfoBlock), &mem_map);
  ((char*) mem_map.virt)[0] = 'V';
  ((char*) mem_map.virt)[1] = 'B';
  ((char*) mem_map.virt)[2] = 'E';
  ((char*) mem_map.virt)[3] = '2';

  reg86_t reg;
  memset(&reg,0,sizeof(reg));
  reg.ax = 0x4F00;
  reg.es = PB2BASE(mem_map.phys); /* set a segment base */
  reg.di = PB2OFF(mem_map.phys); /* set the offset accordingly */
  reg.intno = 0x10;

  if (sys_int86(&reg) != OK) {return 1;}

  memcpy(vbeBlock, mem_map.virt, mem_map.size);

  info->VBESignature[0] = vbeBlock->VbeSignature[0];
  info->VBESignature[1] = vbeBlock->VbeSignature[1];
  info->VBESignature[2] = vbeBlock->VbeSignature[2];
  info->VBESignature[3] = vbeBlock->VbeSignature[3];

  info->VBEVersion[0] = vbeBlock->VbeVersion[0];
  info->VBEVersion[1] = vbeBlock->VbeVersion[1];

  info->TotalMemory = 64*vbeBlock->TotalMemory;
  info->OEMString = (char *)(far_pointer(vbeBlock->OemStringPtr));
  info->VideoModeList = (uint16_t *)(far_pointer(vbeBlock->VideoModePtr));
  info->OEMVendorNamePtr = (char *)(far_pointer(vbeBlock->OemVendorNamePtr));
  info->OEMProductNamePtr = (char *)(far_pointer(vbeBlock->OemProductNamePtr));
  info->OEMProductRevPtr = (char *)(far_pointer(vbeBlock->OemProductRevPtr));

  lm_free(&mem_map);

  return 0;
}
