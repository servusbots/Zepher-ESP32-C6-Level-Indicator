#ifndef FONT5X7_H_
#define FONT5X7_H_

#include <stdint.h>

#define FONT5X7_WIDTH   5
#define FONT5X7_HEIGHT  7

/* Returns a pointer to 5 bytes representing the 5 columns of the character */
const uint8_t *font5x7_get_char(char c);

#endif /* FONT5X7_H_ */
