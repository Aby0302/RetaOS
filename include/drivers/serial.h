#pragma once
void serial_init(void);
void serial_write(const char* s);
void serial_write_hex(unsigned int v);
void serial_write_dec(unsigned int v);
int  serial_getchar_nonblock(void);
