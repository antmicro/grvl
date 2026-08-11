## Zephyr sample

Setup west:
```bash
uv venv --python=3.14
. ./.venv/bin/activate
uv pip install west
```

Build demo:
```bash
# Setup
west init -l manifest
west update
west packages pip | xargs uv pip install
west patch apply
export PATH="/sbin:$PATH"

# Clean Build
west build -b stm32h747i_disco/stm32h747xx/m7 -p

# Flash and connect (& rebuild if needed)
# Use Ctrl+A + Ctrl+X to quit picocom
west flash -r openocd && picocom /dev/ttyACM0 -b 115200
```

To reset the board without flashing:
```
openocd -f zephyr/boards/st/stm32h747i_disco/support/openocd_stm32h747i_disco_m7.cfg -c "init; reset run; shutdown" && picocom /dev/ttyACM0 -b 115200
```
