CFLAGS  = -std=c99 -pipe -Os -Wall
#CFLAGS += -Wno-pointer-sign -Wno-main
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -fpack-struct -fshort-enums -Wpadded
CFLAGS += -Wmissing-field-initializers
CFLAGS += -fno-split-wide-types
CFLAGS += -fms-extensions
#CFLAGS += -ffunction-sections -fdata-sections

linear_eq_conversion: linear_eq_conversion.c
	gcc $(CFLAGS) $^ -lm -o $@
