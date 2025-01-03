#ifndef WORKE_BUFF_HW  // Include guard to prevent redefinition
#define WORKE_BUFF_HW

#include "../Include/stdint.h"

struct vbe_mode_info_structure {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__ ((packed));

// As we set in Source/Loader/boot.s, we cast this structure over 0x6000. From osdever
struct vbe_mode_info_structure* vbe_info = (struct vbe_mode_info_structure*)0x1000;

unsigned char *FONT_BUFF = (unsigned char *)(0x70000 + 512);
unsigned char *WORK_BUFF = (unsigned char *)0x100000; // start here, anything bellow and it eats into the botherboard BIOS

uint16_t WIN_WIDTH;
uint16_t WIN_HEIGHT;
uint8_t WIN_BPP;

uint8_t *WIN_FBUFF;

#endif