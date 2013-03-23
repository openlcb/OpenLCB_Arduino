
#ifndef __PGMSPACE_H_
#define __PGMSPACE_H_ 1


#define prog_char char
#define PROGMEM

#ifdef __cplusplus
extern "C" {
#endif

uint8_t pgm_read_byte(const char* addr) {return *addr;}


#ifdef __cplusplus
}
#endif

#endif /* __PGMSPACE_H_ */
