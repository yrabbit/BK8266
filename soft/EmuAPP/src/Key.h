#ifndef MAIN_H_INCLUDE
#define MAIN_H_INCLUDE

#include <stdint.h>

#define Key_SetNewRusLat() Key_Flags |=  KEY_FLAGS_RUSLAT
#define Key_ClrNewRusLat() Key_Flags &= ~KEY_FLAGS_RUSLAT

enum
{
    KEY_LALT        = 0xC0,
    KEY_LSHIFT,
    KEY_LCTRL,
    KEY_CAPSLOCK,
    KEY_RSHIFT,
    KEY_SCROLL,
    KEY_UNKNOWN     = 0xFF
};

#define KEY_FLAGS_RUSLAT_POS    0
#define KEY_FLAGS_LALT_POS      1
#define KEY_FLAGS_RALT_POS      2
#define KEY_FLAGS_LSHIFT_POS    3
#define KEY_FLAGS_RSHIFT_POS    4
#define KEY_FLAGS_LCTRL_POS     5
#define KEY_FLAGS_RCTRL_POS     6
#define KEY_FLAGS_CAPSLOCK_POS  7
#define KEY_FLAGS_TURBO_POS     8

#define KEY_FLAGS_RUSLAT        (1 << KEY_FLAGS_RUSLAT_POS  )
#define KEY_FLAGS_LALT          (1 << KEY_FLAGS_LALT_POS    )
#define KEY_FLAGS_RALT          (1 << KEY_FLAGS_RALT_POS    )
#define KEY_FLAGS_LSHIFT        (1 << KEY_FLAGS_LSHIFT_POS  )
#define KEY_FLAGS_RSHIFT        (1 << KEY_FLAGS_RSHIFT_POS  )
#define KEY_FLAGS_LCTRL         (1 << KEY_FLAGS_LCTRL_POS   )
#define KEY_FLAGS_RCTRL         (1 << KEY_FLAGS_RCTRL_POS   )
#define KEY_FLAGS_CAPSLOCK      (1 << KEY_FLAGS_CAPSLOCK_POS)
#define KEY_FLAGS_TURBO         (1 << KEY_FLAGS_TURBO_POS   )

#define KEY_AR2_PRESSED 0x8000

extern uint16_t Key_Flags;

uint_fast16_t Key_Translate (uint_fast16_t CodeAndFlags);

#endif
