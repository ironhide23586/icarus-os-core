# DTCM Fault Address Decoder

When DTCM protection is enabled and a fault occurs, the system will blink out the fault address using LED E3.

## Blink Pattern

1. **4 fast blinks** - MemManage fault indicator
2. **Long pause** (4 seconds)
3. **Address nibbles** - Upper 16 bits of MMFAR (4 hex digits)
4. **Long pause** (6 seconds)
5. **Continuous 4 blinks** - System halted

## Decoding Nibbles

Each hex digit (0-F) is blinked as follows:
- **0** = One long blink (0.8 seconds)
- **1-15** = That many short blinks (0.2 seconds each)

Between nibbles: 1 second pause

## Example

If you see:
```
4 fast blinks → pause → 2 blinks → pause → 0 (long) → pause → 0 (long) → pause → 0 (long) → pause → continuous 4 blinks
```

This decodes to: `0x2000xxxx` (DTCM region)

## Memory Regions

- `0x2000xxxx` = DTCM (128KB at 0x20000000-0x2001FFFF)
- `0x2400xxxx` = RAM_D1 (512KB at 0x24000000-0x2407FFFF)
- `0xFFFFxxxx` = Invalid address (MMFAR not valid)

## What to Do

1. Watch the LED blink pattern after the initial 4 fast blinks
2. Count the blinks for each of the 4 nibbles (upper 16 bits)
3. Write down the hex value
4. Look up the address in `build/icarus_os.map` to find which variable is at that location
5. Move that variable to RAM_D1 using `__attribute__((section(".ram_d1")))`
6. Rebuild and test again
