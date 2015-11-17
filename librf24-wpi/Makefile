PREFIX=/usr/local/

LIB=librf24-wpi
LIBNAME=$(LIB).so.1.0
OBJDIR=obj/
LIBDIR=lib/

CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard

all:librf24-wpi

librf24-wpi:${OBJDIR}spi.o ${OBJDIR}RF24_WPI.o
	g++ -shared -Wl,-soname,$@.so.1 ${CCFLAGS} -o ${LIBDIR}${LIBNAME} $^

${OBJDIR}spi.o:src/spi.cpp
	g++ -Wall -fPIC ${CCFLAGS} -c $^ -o $@
${OBJDIR}RF24_WPI.o:src/RF24.cpp
	g++ -Wall -fPIC ${CCFLAGS} -DGPIO_WPI -c $^ -o $@

clean:
	rm -rf ${OBJDIR}*.o ${LIBDIR}${LIBNAME}

install:
	@echo "[Install]"
	@if ( test ! -d ${PREFIX}lib ) ; then mkdir -p ${PREFIX}lib ; fi
	@if ( test ! -d ${PREFIX}include ) ; then mkdir -p ${PREFIX}include ; fi
	@install -m 0755 ${LIBDIR}${LIBNAME} ${PREFIX}${LIBDIR}
	@ln -sf ${PREFIX}${LIBDIR}${LIBNAME} ${PREFIX}${LIBDIR}${LIB}.so
	@install -m 0755 src/RF24.h ${PREFIX}include
	@ldconfig
