# D'après
# https://github.com/digiampietro/arduino-makefile
# Date : 22 juin 2026
#
# Pour compiler les projets avec arduino-cli. 
# Les options sont :
# - all
# - compile
# - clean
# - upload
# - help
# - docs
# - debug

PROJECT     := $(notdir $(CURDIR))
VERSION     := $(shell git describe --abbrev=0)
BOARD       := $(shell echo $(board) | tr '[:lower:]' '[:upper:]')
CFLAGS      := -D$(oled) -D$(BOARD) -DVERSION=\"$(VERSION)\"
CFLAGS      := --build-property build.extra_flags="$(CFLAGS)"
BIN_DIR     := bin/$(board)
SRCINO      := $(PROJECT).ino
HEX         := $(BIN_DIR)/$(SRCINO).hex
FLASH       := -U flash:w:bin/firmware.hex:i
FUSES       := -U fuse2:w:0x01:m -U fuse5:w:0xC9:m -U fuse8:w:0x00:m
UFLAGS      := $(FLASH)
DOXYFILE    :=
DOCS        :=
MODULES     = $(wildcard $(DOCS)/modules/*.md)
METADATA    = $(DOCS)/metadata.yaml
MAN         = $(DOCS)/MANUAL_FR.md
JEKYLL_DIR  :=
MANIFEST    := ../arduino-manifest/arduino-manifest.pl

ifneq (,$(wildcard Doxyfile))
	DOXYFILE    := Doxyfile
endif
ifneq (,$(wildcard docs))
	DOCS        := docs
endif
ifneq (,$(wildcard ../ciylab.github.io))
	JEKYLL_DIR  := ../ciylab.github.io
endif

ifeq ($(board), nano)
	FQBN = arduino:avr:nano
	MCU = atmega328p
	PROGRAMMER = arduino
	PORT = /dev/ttyUSB0
	BAUDRATE = 57600
else ifeq ($(board), every)
	FQBN = arduino:megaavr:nona4809:mode=off
	MCU = atmega4809
	PROGRAMMER = jtag2updi
	PORT = /dev/ttyACM0
	BAUDRATE = 115200
	UFLAGS += $(FUSES)
else ifeq ($(board), thinary)
# FQBN = MegaCoreX:megaavr:4808:BOD=disabled,pinout=nano_4808
	FQBN = thinary:avr:nona4808:mode=off
	MCU = atmega4808
    PROGRAMMER = jtag2updi
    PORT = /dev/ttyUSB0
    BAUDRATE = 115200
    UFLAGS += $(FUSES)
endif

all: debug compile clean docs

compile: $(SRCINO)
	$(info **************** build $(VERSION))
	@arduino-cli compile --warnings all -b $(FQBN) $(CFLAGS) --output-dir $(BIN_DIR)
	@cp $(HEX) bin/firmware.hex

clean: 
	$(info **************** delete unused binaries files) 
	@rm -fr $(BIN_DIR) *.lst
	
upload:
ifeq ($(board), every)
	$(shell stty --file $(PORT) 1200)
	$(shell stty --file $(PORT) 1200)
endif 
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUDRATE) -e -D $(UFLAGS)

doxygen: 
	$(info **************** create html)
	@doxygen $(DOXYFILE) 1> /dev/null 2>&1

.PHONY: tags
tags:
	$(info **************** create tags)
	@ctags -R $(SRCINO) src
	
metadata:
	$(info **************** create metadata.yaml)
	@echo "---\nlayout: page" > $(METADATA)
	@echo "title: $(PROJECT)\nsubtitle: version $(VERSION)" >> $(METADATA)
	@echo "author: Pierrick MEIGNEN" >> $(METADATA)
	@echo "email: contact@ciylab.com" >> $(METADATA)
	@echo "lang: fr-FR\nfontfamily: times" >> $(METADATA)
	@echo "header-includes:" >> $(METADATA)
	@echo '  - \\renewcommand{\\familydefault}{\sfdefault}' >> $(METADATA)
	@echo "---\n" >> $(METADATA)
	@echo "![Paramètres](docs/assets/images/$(PROJECT)_menus.png)\n" >> $(METADATA)

.PHONY: man
man: metadata
	$(info **************** create MANUAL_FR.md)
	@cat $(METADATA) > $(MAN)
	@cat $(DOCS)/Main.md >> $(MAN)
	@cat $(MODULES) >> $(MAN)
	@cat $(DOCS)/Tech.md >> $(MAN)
	@soffice --convert-to png --outdir docs docs/menus.odg 1> /dev/null 2>&1
	@mogrify -resize 75% docs/menus.png
	@mv docs/menus.png docs/assets/images/$(PROJECT)_menus.png
	@pandoc $(MAN) -V geometry:margin=2cm -o man/$(PROJECT)_MANUAL_FR.pdf
	@mv docs/MANUAL_FR.md $(JEKYLL_DIR)/$(PROJECT)_MANUAL_FR.md

manifest:
	$(info **************** create requirements.txt)
	@$(MANIFEST) -r -v -b $(FQBN) $(SRCINO) > requirements.txt
	
.PHONY: docs
docs: doxygen tags man manifest

help:
	@echo "Usage: make board=BOARD oled=OLED OPTION where BOARD is"
	@echo "    - nano"
	@echo "    - every"
	@echo "    - thinary"
	@echo "and OLED is"
	@echo "    - SSD1306 (128x64)"
	@echo "    - SH1106 (132x64)"
	@echo "and OPTION is"
	@echo "    - all"
	@echo "    - compile"
	@echo "    - clean"
	@echo "    - upload"
	@echo "    - docs"
	@echo "    - help"
	@echo "    - debug"
	
debug:
	$(info **************** info)
	@echo projet = $(PROJECT)
	@echo version = $(VERSION)
	@echo fqbn = $(FQBN)
	@echo board = $(BOARD)
	@echo cflags = $(CFLAGS)
	@echo bin_dir = $(BIN_DIR)
	@echo srcino = $(SRCINO)
	@echo hex = $(HEX)
	@echo mcu = $(MCU)
	@echo programmer = $(PROGRAMMER)
	@echo port = $(PORT)
	@echo baudrate = $(BAUDRATE)
	@echo uflags = $(UFLAGS)
	@echo doxyfile = $(DOXYFILE)
	@echo docs = $(DOCS)
	@echo modules = $(basename $(notdir $(MODULES)))
	@echo metada = $(METADATA)
	@echo manual = $(MAN)
	@echo jekyll = $(JEKYLL_DIR)
	@echo manifest = $(MANIFEST)

	
