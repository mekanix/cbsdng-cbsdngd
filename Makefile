PREFIX ?= /usr/local
LOCALBASE ?= /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/man/man
LIBDIR = ${PREFIX}/lib
INCLUDEDIR = ${PREFIX}/include/cbsdng

CXXFLAGS += -I. -I${LOCALBASE}/include -std=c++17
LDADD += -L${LIBDIR}
LDADD += -pthread
LDADD += -lfmt
LDADD += -lspdlog
LDADD += -lutil
PROG_CXX = cbsdngd
MK_MAN = no
SRCS != ls -1 src/*.cpp

MAININCS = cbsdng/message.h
MAININCSDIR = ${INCLUDEDIR}
PROXYINCS != ls -1 cbsdng/proxy/*.h
PROXYINCSDIR = ${INCLUDEDIR}/proxy
INCSGROUPS = MAININCS PROXYINCS

beforeinstall:
	${INSTALL} -d -m 0755 ${DESTDIR}${INCLUDEDIR}/proxy

.include <bsd.prog.mk>
