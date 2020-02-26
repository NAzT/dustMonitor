# Flashing Instructions
To flash the firmware, first download and install esptool.

Then run the following command while in the export directory:
> esptool.py --chip esp32 --port "/dev/ttyUSB0" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin

Replace /dev/ttyUSB0 with the path or name of your serial device.
