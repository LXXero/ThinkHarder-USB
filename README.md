# ThinkHarder-USB

PS/2 to USB converter for the **Kensington Expert Mouse** trackball with full
4-button support, built on [QMK Firmware](https://qmk.fm/) running on a
Pro Micro (ATmega32U4).

The Kensington Expert Mouse uses the **ThinkingMouse** protocol -- a
proprietary PS/2 extension that Kensington never publicly documented. Without
the correct initialization sequence, buttons 3 and 4 silently mirror buttons
1 and 2. The protocol was reverse-engineered from the FreeBSD `psm.c` kernel
driver and the X.org `xf86-input-mouse` driver source.

## Wiring

PS/2 Mini-DIN 6 pinout (front view, looking into the socket):

```
    6 5
   4   3
    2 1
```

| PS/2 Pin | Signal | Pro Micro Pin |
|----------|--------|---------------|
| 1        | Data   | Pin 9 (PB5)  |
| 3        | GND    | GND          |
| 4        | VCC    | VCC (5V)     |
| 5        | Clock  | Pin 7 (PE6)  |

Pins 2 and 6 are not connected.

**Pull-up resistors:** 4.7k ohm from Clock to VCC and Data to VCC. PS/2 is
an open-collector bus -- the host (Pro Micro) must provide pull-ups. The
ATmega32U4's internal pull-ups (~20-50k) are too weak.

**Important:** PS/2 is not hot-pluggable. Connect the mouse before applying
power.

## Button Map

| Physical Button | PS/2 Report | USB HID      |
|-----------------|-------------|--------------|
| Bottom-left     | Bit 0       | Button 1 (left)   |
| Bottom-right    | Bit 1       | Button 2 (right)  |
| Top-left        | Bit 2       | Button 3 (middle) |
| Top-right       | Bit 3       | Button 4          |

Hold top-left (middle click) and move the ball to scroll. Quick tap sends
a middle click.

## Building

Requires a working [QMK build environment](https://docs.qmk.fm/newbs_getting_started).

### 1. Apply the QMK driver patch

The stock QMK PS/2 mouse driver only supports 3 buttons and has no
ThinkingMouse support. The included patch adds:

- `PS2_MOUSE_SKIP_RESET` -- skip the PS/2 reset command on init
- `PS2_MOUSE_THINKING` -- ThinkingMouse protocol handling (sync check,
  X MSB reconstruction, overflow bit reinterpretation)
- `PS2_MOUSE_BTN_MASK` made overridable via `#ifndef`

```sh
cd /path/to/qmk_firmware
git apply /path/to/ThinkHarder-USB/qmk_thinkingmouse.patch
```

### 2. Install the keyboard definition

```sh
cp -r /path/to/ThinkHarder-USB/keyboard \
      /path/to/qmk_firmware/keyboards/kensington_ps2usb
```

### 3. Compile and flash

```sh
qmk compile -kb kensington_ps2usb -km default
qmk flash -kb kensington_ps2usb -km default
```

Double-tap RST to GND on the Pro Micro to enter bootloader (Caterina).

## ThinkingMouse Protocol

The Kensington Expert Mouse implements a proprietary PS/2 protocol extension.

### Detection

1. Set sample rate to 10 (`F3 0A`) -- triggers native mode
2. Get device ID (`F2`) -- returns `02` if ThinkingMouse (normally `00`)
3. Set resolution LOW (`E8 00`)
4. Send magic sample rate sequence: `{20, 60, 40, 20, 20, 60, 40, 20, 20}`

Without step 4, buttons 3+4 mirror buttons 1+2.

### Native Mode Byte 0

Standard PS/2 byte 0:
```
bit 7: Y overflow    bit 3: always 1 (sync)
bit 6: X overflow    bit 2: middle button
bit 5: Y sign        bit 1: right button
bit 4: X sign        bit 0: left button
```

ThinkingMouse byte 0:
```
bit 7: always 1 (sync)    bit 3: button 4
bit 6: X data MSB         bit 2: middle button
bit 5: Y sign             bit 1: right button
bit 4: X sign             bit 0: left button
```

Key differences:
- Bit 7 changes from Y overflow to sync (always 1)
- Bit 6 changes from X overflow to X data MSB (the mouse sends 7 bits of
  X in byte 1, with the MSB stored here)
- Bit 3 changes from sync to button 4

### Sources

- FreeBSD `sys/dev/atkbdc/psm.c` -- `enable_kmouse()` function
- X.org `xf86-input-mouse` `src/mouse.c` -- `PROT_THINKPS2` parser

## License

GPL-2.0 (same as QMK)
