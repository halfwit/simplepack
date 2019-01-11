# binpack - lay out windows automatically
# See LICENSE file for copyright and license details.

SRC = simplepack.c
OBJ = ${SRC:.c=.o}

all: simplepack

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

binpack: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f binpack ${OBJ}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f simplepack ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/simplepack
uninstall:
	@rm -f ${DESTDIR}${PREFIX}/bin/simplepack

.PHONY: all clean install uninstall
