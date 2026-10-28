#ifndef LOGIC_H
#define LOGIC_H
#include <stdint.h>
#include <stdbool.h>
#define COLS 12
#define ROWS 8
#define CELLS (COLS*ROWS)
#define VIEW_COLS 6
#define VIEW_ROWS 4
#define NONE 255
enum { EMPTY, SWITCH, LIGHT, AND, OR, NOT, NAND, NOR, XOR, XNOR, BUFFER, TYPES };
enum { LOW, HIGH, UNKNOWN };
typedef struct { uint8_t type, on, in[2]; } Node;
typedef struct { uint8_t cols, rows; Node node[CELLS]; } Board;
extern const char *const names[TYPES];
uint8_t pins(uint8_t type);
void board_clear(Board *b);
uint8_t board_count(const Board *b);
bool board_resize(Board *b,uint8_t cols,uint8_t rows);
void board_place(Board *b, uint8_t cell, uint8_t type);
void board_delete(Board *b, uint8_t cell);
bool board_connect(Board *b, uint8_t source, uint8_t target, uint8_t pin);
bool board_valid(const Board *b);
void board_eval(const Board *b, uint8_t values[CELLS]);
uint8_t gate_eval(uint8_t type, uint8_t a, uint8_t b);
void board_demo(Board *b, uint8_t demo);
#endif
