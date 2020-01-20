

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
	@echo "done!"

clean:
	@rm -f $(EXE) $(OBJ)

distclean: clean

.PHONY: all build install clean distclean
