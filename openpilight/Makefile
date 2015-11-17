#PREFIX=/usr/local/

EXECUTABLE=openpilight
OBJDIR=obj/

CCFLAGS=-Ofast -mfpu=vfp -mfloat-abi=hard -march=armv7-a -mtune=arm1176jzf-s -std=c++11 -I/usr/local/include -L/usr/local/lib -lrf24-wpi -lwiringPi

all:openpilight

openpilight:${OBJDIR}UDPServer.o ${OBJDIR}MiLightRadio.o ${OBJDIR}PL1167_nRF24.o ${OBJDIR}openpilight.o
	g++ -Wl,-soname,$@.so.1 ${CCFLAGS} -o ${EXECUTABLE} $^

${OBJDIR}UDPServer.o:src/UDPServer.cpp
	g++ -Wall ${CCFLAGS} -c $^ -o $@
${OBJDIR}MiLightRadio.o:src/MiLightRadio.cpp
	g++ -Wall ${CCFLAGS} -c $^ -o $@
${OBJDIR}PL1167_nRF24.o:src/PL1167_nRF24.cpp
	g++ -Wall ${CCFLAGS} -c $^ -o $@
${OBJDIR}openpilight.o:src/openpilight.cpp
	g++ -Wall ${CCFLAGS} -c $^ -o $@

clean:
	rm -rf ${OBJDIR}*.o ${EXECUTABLE}
	