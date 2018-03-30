ROOT_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CUR_DIR := $(notdir $(patsubst %/,%,$(ROOT_PATH)))

QMAKE ?= qmake

BUILD_PATH := $(ROOT_PATH)build/
PROJECT_FILE := $(ROOT_PATH)BKSUPPLY.pro
DESTDIR ?= /usr/local

all install: $(BUILD_PATH)Makefile
	+$(MAKE) -C "$(BUILD_PATH)" $@

$(BUILD_PATH)Makefile: $(PROJECT_FILE)
	@[ -d "$(BUILD_PATH)" ] || mkdir "$(BUILD_PATH)"
	cd "$(BUILD_PATH)"; $(QMAKE) "$(PROJECT_FILE)" PREFIX="$(DESTDIR)"

clean:
	rm -rf "$(BUILD_PATH)"
