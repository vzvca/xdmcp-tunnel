
PREFIX ?= /usr/local
LDFLAGS=-lX11
CFLAGS=

EXE=xdmcptunnel
SRC=xdmcptunnel.c
OBJ=xdmcptunnel.à

all:
	@echo "Targets:"
	@echo "\tbuild\tcompiles and link"
	@echo "\tinstall\tinstall defaults to /usr/local/bin"
	@echo "\tclean\tremove compilation result"

build: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

install: build
	install -s -v -o root -g root -t $(PREFIX)/bin $(EXE)

clean:
	@rm -f $(EXE) $(OBJ)

distclean: clean

.PHONY: all build install clean distclean
