#include QMK_KEYBOARD_H
#include "ps2.h"
#include "wait.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(KC_NO)
};

void ps2_mouse_init_user(void) {
    /*
     * ThinkingMouse enable sequence (from FreeBSD psm.c enable_kmouse).
     * Without this, buttons 3+4 mirror buttons 1+2.
     *
     * Protocol: set sample rate 10 triggers native mode (device ID
     * changes from 0 to 2), then the magic sample rate sequence
     * {20,60,40,20,20,60,40,20,20} enables independent button reporting.
     */

    /* disable data reporting while we reconfigure */
    ps2_host_send(0xF5);
    wait_ms(20);

    /* flush any pending data */
    while (pbuf_has_data()) ps2_host_recv();

    /* set sample rate 10 -- triggers native mode */
    ps2_host_send(0xF3);
    ps2_host_send(10);
    wait_ms(20);

    /* get device ID -- should return 2 in native mode */
    ps2_host_send(0xF2);
    wait_ms(20);
    uint8_t id = ps2_host_recv_response();

    if (id != 0x02) {
        /* not a ThinkingMouse, bail out */
        ps2_host_send(0xF4);
        return;
    }

    /* set resolution LOW (required before magic sequence) */
    ps2_host_send(0xE8);
    ps2_host_send(0);
    wait_ms(20);

    /* magic sample rate sequence to enable buttons 3+4 */
    static const uint8_t rates[] = {20, 60, 40, 20, 20, 60, 40, 20, 20};
    for (uint8_t i = 0; i < sizeof(rates); i++) {
        ps2_host_send(0xF3);
        ps2_host_send(rates[i]);
        wait_ms(10);
    }

    /* set resolution back to 4 counts/mm */
    ps2_host_send(0xE8);
    ps2_host_send(2);
    wait_ms(10);

    /* set sample rate to 100/sec */
    ps2_host_send(0xF3);
    ps2_host_send(100);
    wait_ms(10);

    /* flush stale bytes before restarting */
    while (pbuf_has_data()) ps2_host_recv();

    /* re-enable data reporting */
    ps2_host_send(0xF4);

    /* let a few full packets arrive, then flush so ps2_mouse_task()
     * starts reading on a clean packet boundary */
    wait_ms(80);
    while (pbuf_has_data()) ps2_host_recv();
}
