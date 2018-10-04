#ifndef UI_H
#define UI_H


#include "ets.h"

typedef struct
{
	uint8_t  Show;
	uint8_t  PrevRusLat;
	uint8_t  CursorX;
	uint8_t  CursorY;
	char     scr[25][32];

} TUI_Data;

extern TUI_Data UI_Data;


void ui_clear(void);
void ui_header(const char *s);
void ui_header_default(void);
void ui_draw_list(uint8_t x, uint8_t y, const char *s);
void ui_draw_text(uint8_t x, uint8_t y, const char *s);
int8_t ui_select(uint8_t x, uint8_t y, uint8_t count);
const char* ui_input_text(const char *comment, const char *text, uint8_t max_len);
int8_t ui_yes_no(const char *comment);

void ui_start(void);
void ui_stop(void);

void ui_sleep(uint16_t ms);

uint_fast16_t ui_GetKey(void);


#endif
