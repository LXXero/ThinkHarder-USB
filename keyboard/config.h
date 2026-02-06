#pragma once

/* PS/2 interrupt mode using INT6 on PE6 (Pro Micro pin 7) */
#define PS2_INT_INIT()  do {    \
    EICRB |= ((1<<ISC61) |      \
              (0<<ISC60));       \
} while (0)
#define PS2_INT_ON()  do {       \
    EIMSK |= (1<<INT6);          \
} while (0)
#define PS2_INT_OFF() do {       \
    EIMSK &= ~(1<<INT6);         \
} while (0)
#define PS2_INT_VECT   INT6_vect

/* wait for mouse to complete power-on BAT before sending commands.
 * BAT can take up to 2 seconds on cold boot. */
#define PS2_MOUSE_INIT_DELAY 2000

/* skip the PS/2 reset command -- the mouse already did BAT on power-up.
 * sending reset blocks for 1+ seconds and stale BAT bytes in the ring
 * buffer cause protocol desync. just flush and enable reporting. */
#define PS2_MOUSE_SKIP_RESET

/* ThinkingMouse native mode: bit 3 = button 4, bit 7 = sync (not overflow) */
#define PS2_MOUSE_THINKING
#define PS2_MOUSE_BTN_MASK 0x0F

/* swap top-left (bit 2, middle) and top-right (bit 3, btn4) so that
 * top-right becomes the middle/scroll button */
#define PS2_MOUSE_THINKING_BTN_SWAP 0x0302

/* slow down scroll-on-hold (default 2 is way too fast for a trackball) */
#define PS2_MOUSE_SCROLL_DIVISOR_V 8
#define PS2_MOUSE_SCROLL_DIVISOR_H 8

/* debounce button releases to suppress micro-switch chatter (ms) */
#define PS2_MOUSE_BTN_DEBOUNCE 50

