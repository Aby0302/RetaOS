#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum vga_color{VGA_COLOR_BLACK=0,VGA_COLOR_LIGHT_GREY=7,VGA_COLOR_WHITE=15};
static inline uint8_t vga_entry_color(enum vga_color fg,enum vga_color bg){return fg|bg<<4;}
static inline uint16_t vga_entry(unsigned char uc,uint8_t color){return (uint16_t)uc|(uint16_t)color<<8;}
static size_t strlen_(const char* s){size_t n=0;while(s[n])n++;return n;}
static const size_t W=80,H=25; static size_t r,c; static uint8_t col; static uint16_t* buf;
static void term_init(void){r=0;c=0;col=vga_entry_color(VGA_COLOR_LIGHT_GREY,VGA_COLOR_BLACK);buf=(uint16_t*)0xB8000;for(size_t y=0;y<H;y++)for(size_t x=0;x<W;x++)buf[y*W+x]=vga_entry(' ',col);}
static void putat(char ch,uint8_t color,size_t x,size_t y){buf[y*W+x]=vga_entry(ch,color);}
static void put(char ch){if(ch=='\n'){c=0;if(++r==H)r=0;return;}putat(ch,col,c,r);if(++c==W){c=0;if(++r==H)r=0;}}
static void write(const char* d,size_t n){for(size_t i=0;i<n;i++)put(d[i]);}
static void writes(const char* d){write(d,strlen_(d));}
void kernel_main(void){term_init();writes("Welcome to RetaOS!\nMinimal multiboot kernel is up.\n");for(;;){__asm__ __volatile__("hlt");}}
