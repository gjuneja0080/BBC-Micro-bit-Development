.PHONY: flash
flash: build
	cp build/bbc-microbit-classic-gcc/source/microbit-a3-combined.hex /media/chance/MICROBIT

.PHONY: flashb
flashb: build
	cp build/bbc-microbit-classic-gcc/source/microbit-a3-combined.hex /media/chance/MICROBIT
	cp build/bbc-microbit-classic-gcc/source/microbit-a3-combined.hex /media/chance/MICROBIT1


.PHONY: build
build:
	docker run -v "$(shell pwd)":/yotta_wd --rm yotta

.PHONY: patch
patch:
	sed -i "205s/.*/#define MICROBIT_BLE_ENABLED 0/" yotta_modules/microbit-dal/inc/core/MicroBitConfig.h
	sed -i "46s/.*/#define MICROBIT_BUTTON_HOLD_TIME 300/" yotta_modules/microbit-dal/inc/drivers/MicroBitButton.h
	sed -i "68s/.*/#define MICROBIT_RADIO_MAX_PACKET_SIZE 200/" yotta_modules/microbit-dal/inc/drivers/MicroBitRadio.h
	sed -i "33s/.*/#define MICROBIT_SERIAL_DEFAULT_BUFFER_SIZE 200/" yotta_modules/microbit-dal/inc/drivers/MicroBitSerial.h
