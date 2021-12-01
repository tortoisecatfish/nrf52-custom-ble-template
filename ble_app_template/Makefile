CONSOLE := /dev/ttyUSB0
APPBAUD := 1000000

BUILD_DIRECTORY=pca10040/s132/armgcc
OUTPUT_DIRECTORY=pca10040/s132/armgcc/_build

.PHONY: default flash_softdevice flash console

default:
	make -j1 -C $(BUILD_DIRECTORY)

flash_softdevice:
	make -C $(BUILD_DIRECTORY) flash_softdevice

# flash:
# 	make -C $(BUILD_DIRECTORY) flash

console:
	@stty raw ignbrk -echo $(APPBAUD) < $(CONSOLE)
	@echo Press ctrl+D to exit
	@bash -c '( cat <&3 & cat >&3; kill %%; ) 3<>$(CONSOLE)'

debug:
	arm-none-eabi-gdb -ex 'file $(OUTPUT_DIRECTORY)/nrf52832_xxaa.out' -ex 'target remote localhost:2331'

erase:
	nrfjprog -f nrf52 --eraseall

$(OUTPUT_DIRECTORY)/bl-settings.hex:$(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	nrfutil settings generate --family NRF52 --application $^ --application-version 1 --bootloader-version 1 --bl-settings-version 1 $@

$(OUTPUT_DIRECTORY)/merged.hex: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex $(OUTPUT_DIRECTORY)/bl-settings.hex $(BOOTLOADER_HEX)
	@mergehex -o $@ -m $^

# Flash the program
flash: $(OUTPUT_DIRECTORY)/merged.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $< --sectorerase
	nrfjprog -f nrf52 --reset
