/**
 * On a Raspberry Pi 2 compile with:
 *
 * g++ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv7-a -mtune=arm1176jzf-s -I/usr/local/include -L/usr/local/lib -lrf24-bcm PL1167_nRF24.cpp MiLightRadio.cpp openmili.cpp -o openmilight
 *
 * for receiver mode run with:
 * sudo ./openmilight
 *
 * for sender mode run with:
 */

#include <cstdlib>
#include <iostream>
#include <string.h>
#include "timing.h"
#include <signal.h>

using namespace std;

#include <RF24.h>
#include "PL1167_nRF24.h"
#include "MiLightRadio.h"

#include "UDPServer.h"

bool quit = false;
void myQuitHandler(int s){
	printf("Caught signal %d\n",s);
	quit = true;
}

RF24 radio(0, 2, "/dev/spidev0.0");
UDPServer* udp;
UDPServer* discoverUdp;
PL1167_nRF24 prf(radio);
MiLightRadio mlr(prf);

static int dupesPrinted = 0;
void receiveLoop()
{
    if (mlr.available()) {
      printf("\n");
      uint8_t packet[7];
      size_t packet_length = sizeof(packet);
      mlr.read(packet, packet_length);

      for (uint32_t i = 0; i < packet_length; i++) {
        printf("%02X ", packet[i]);
      }
    }

    int dupesReceived = mlr.dupesReceived();
    for (; dupesPrinted < dupesReceived; dupesPrinted++) {
      printf(".");
    }
}

bool globalProfileTiming = false;
void exitHelp() {
	printf("\n");
	printf("openpilight Communication Gateway usage instructions\n");
	printf("================================================\n");
	printf("Start Server: openpilight [options]\n");
	printf("\n");
	printf("radio options:\n");
	printf("     -a hwAddress      hardware address to be sent within 2,4GHz commands which identifies the\n");
	printf("                       sending remote given in HEX - valid values are from 0001 till FFFF\n");
	printf("                       (default: 0001)\n");
	printf("     -c radioChannel   channel for sending 2,4GHz, valid are 9, 40, 71 - 0 will send on all three\n");
	printf("                       channes (default: 0)\n");
	printf("     -r                number of resends of radio packets - except color and brightness (default: 10)\n");
	printf("     -d                delay between resends of packages in microseconds (default: 0)\n");
	printf("     -l                activates radio listen mode (server disabled - just listening for radio packages)\n");
	printf("UDP server options:\n");
	printf("     -i ipAddress      interface ip address to bind the UDP socket to (default: 0.0.0.0)\n");
	printf("     -p port           port to listen for UDP packets (default: 8899)\n");
	printf("     -x addressString  support automatic discovery by other apps - addressString will containt the response\n");
	printf("                       string which is sent for identification typically is the IP address or hostname of the\n");
	printf("                       server (default: disabled)\n");
	printf("misc options:\n");
	printf("     -v                enables verbose output (default: disabled)\n");
	printf("     -t                profile timing (default: disabled)\n");
	printf("     -s                activates silent mode - no radio output (default: disabled)\n");
	printf("     -h                shows this help\n");
	exit(0);
}

void sendRadio(string prefixStr, bool silent, bool verbose, uint8_t* outPacket, uint32_t len, uint32_t resends, uint32_t resendMicroDelay) {
	if (verbose) {
		printf("%s", prefixStr.c_str());
		for (uint32_t i = 0; i < len; i++) {
			printf (" %02x", outPacket[i]);
		}
		printf("[%d]\n", len);
	}
	if (!silent) {
		TIMER_START(globalProfileTiming);
		mlr.write(outPacket, len);
		for (uint32_t i = 0; i < resends; i++) {
			if (resendMicroDelay > 0) {
				usleep(resendMicroDelay);
			}
			mlr.resend();
		}
  		TIMER_STOP(globalProfileTiming);
  		TIMER_OUTPUT_MS_F("\tRadio write", globalProfileTiming);
	}
}

void sendAssociationSequence(bool silent, bool verbose, uint8_t addr1, uint8_t addr2, uint8_t group) {
  uint8_t data[7] = {0xB0, addr1, addr2, 0x00, 0x00, 0x00, 0x01};
  if (group > 4) { group = 0; }
  data[4] = 0xb8 | group; //brightness also contains group id
  data[5] = group*2+1; //button code, group all: 0x01, group 1: 0x03, group2: 0x05....
  for (int i = 0; i < 3; i++) { //3 times switch on command
  	sendRadio("ASSOC", silent, verbose, data, 7, 10, 0);
  	usleep(250000);
  	data[6]++;
  }
  data[5]|= 0x10; //add long press flag
  for (int i = 0; i < 8; i++) { //8 times switch on long press command
  	sendRadio("ASSOC", silent, verbose, data, 7, 10, 0);
  	usleep(250000);
  	data[6]++;
  }
}

int main(int argc, char** argv)
{
	int c;
	char * pEnd;
	int address = 0x0001;
	string addressListen = "0.0.0.0";
	string discoveryResponse = "";
	int port = 8899;
	int channel = 0;
	int resends = 10;
	int resendMicroDelay = 0;
	bool verbose = false;
	bool silent = false;
	bool listenOnly = false;
	
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = myQuitHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGTERM, &sigIntHandler, NULL);

	while ((c = getopt (argc, argv, "a:i:p:c:r:d:x:ltvsh")) != -1) {
		switch (c) {
			case 'l':
				listenOnly = true;
				printf("activate radio listen mode\n");
				break;
			case 'a':
				address = strtol(optarg,&pEnd,16);
				break;
			case 'i':
				addressListen = optarg;
				break;
			case 'x':
				discoveryResponse = optarg;
				discoveryResponse = discoveryResponse + +",BABECAFEBEEF,"; //add dummy mac address
				break;
			case 'p':
				port = strtol(optarg,&pEnd,10);
				break;
			case 'r':
				resends = strtol(optarg,&pEnd,10);
				break;
			case 'd':
				resendMicroDelay = strtol(optarg,&pEnd,10);
				break;
			case 'c':
				channel = strtol(optarg,&pEnd,10);
				if ((channel != 0) && (channel != 9) && (channel != 40) && (channel != 71)) {
					fprintf (stderr, "Warning: valid channels for milight are 9, 40, 71, using %d\n", channel);
				}
				break;
			case 'v':
				verbose = true;
				printf("activate verbose output\n");
				break;
			case 's':
				silent = true;
				printf("activate silent mode - no radio output\n");
				break;
			case 't':
				globalProfileTiming = true;
				printf("activate timing profiling\n");
				break;
			case 'h':
				exitHelp();
			case '?':
/*				if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else */
				if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n",	optopt);
				return 1;
				break;
		}
	}
	if ((address <= 0) || (address > 0xFFFF)) {
		fprintf (stderr, "no valid address specified for milight transceiver [use -a 0x0001-0xFFFF]\n");
		exitHelp();
	}
	if ((port < 100) || (address > 65535)) {
		fprintf (stderr, "no valid port specified for UDP listener [use -p 100-65535]\n");
		exitHelp();
	}
	
  mlr.begin();
  mlr.setChannel(channel);

  if (verbose) {
  	radio.printDetails();  
  }
  
  if (listenOnly) {
  	while (!quit) {
  		receiveLoop();
  	}
  	printf("Ended gracefully\n");
  	return 0;
  }

  #define UDP_COLOR_OFFSET_SHIFT	-52
  #define UDP_MSG_BUF_SIZE	1024
  #define UDP_MSG_TIMEOUT_MS	50
  char udpMsgBuf[UDP_MSG_BUF_SIZE];
  int size;
  
  udp = new UDPServer(addressListen, port);
  if (discoveryResponse != "") {
	  try {
	  	discoverUdp = new UDPServer(addressListen, 48899);
	  } catch (...) {
	  	discoverUdp = NULL;
		fprintf (stderr, "Warning: discovery channel is alread used\n"); //so let's do it like they do on the discovery channel....
	  }
  } else {
  	discoverUdp = NULL;
  }
  
  uint8_t discoSequence = 255;
  uint8_t seqn = 0;
  int32_t color = 0;
  int32_t brightness = 0x02;
  uint8_t outPacket[7] = {0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  outPacket[1] = 0xFF&(address>>8);
  outPacket[2] = 0xFF&(address>>0);
  
  uint8_t currentGroupControl = 0;
  uint8_t brightnessSent = 0; //needed resend last packet (to be stable)
  uint8_t colorSent = 0; //needed resend last packet (to be stable)
  uint32_t packetHammer = 0;
  		
  while (!quit) {
  	size = udp->timed_recv(udpMsgBuf, UDP_MSG_BUF_SIZE, UDP_MSG_TIMEOUT_MS);
  	//size = udp->recv(udpMsgBuf, UDP_MSG_BUF_SIZE);
  	if (size > 0) {
  		if (verbose) {
			printf("UDP ");
	  		for (int i = 0; i < size; i++) {
		  		printf("%x ", udpMsgBuf[i]);
	  		}
  			printf("\n");
  		}
  		if ((size == 9) && (udpMsgBuf[0] == 0xff) && (udpMsgBuf[8] == 0x55)) {
	  		sendRadio("RAW", silent, verbose, (uint8_t*)&udpMsgBuf[1], 7, 0, 0);
  		} else
  		if ((size == 5) && (udpMsgBuf[0] == 0xff) && (udpMsgBuf[4] == 0x55)) { //special commands
  			sendAssociationSequence(silent, verbose, udpMsgBuf[1], udpMsgBuf[2], udpMsgBuf[3]);
  		} else if ((size == 2) || ((size == 3) && (udpMsgBuf[2] == 0x55))) {
	  		int button = -1;
	  		int doResends = resends;
	  		if (packetHammer > 10) {
	  			doResends = 0; //packet repeat threshold exceeded
  				if (verbose) {
		  			printf("HAMMERTIME\n"); //don't touch that, na na na na :-D
  				}
	  		} else {
	  			packetHammer++;
	  		}
	  		if ((udpMsgBuf[0] >= 0x45) && (udpMsgBuf[0]<=0x4C)) {
		  		/*0x45 => group 1 on = button 3
		  		0x46 => group 1 off = button 4
		  		0x47 => group 2 on = button 5
		  		0x48 => group 2 off
		  		0x49 => group 3 on
		  		0x4A => group 3 off
		  		0x4B => group 4 on = button 9
		  		0x4C => group 4 off = button A*/
	  			button = (udpMsgBuf[0]-0x45) + 3; //button index for group buttons starts at 3
	  			if (button % 2 == 1) {
	  			    currentGroupControl = ((button-3)/2) + 1; //on command changes active group
	  			}
	  		} else if ((udpMsgBuf[0] >= 0xC5) && (udpMsgBuf[0]<=0xCC)) {
	  			button = (udpMsgBuf[0]-0xC5) + 3; //button index for group buttons starts at 3
	  			button|= 0x10; //long press for white/night mode
	  			if (button % 2 == 1) {
	  			    currentGroupControl = ((button-3)/2) + 1; //on command changes active group
	  			}
	  			discoSequence = 255; //reset disco mode
	  		} else {
		  		switch (udpMsgBuf[0]) {
		  			case 0x40:
		  				//color
		  				//there is a shift and a sign inversion between UDP and 2,4Ghz color values
		  				button = 0x0f;
		  				color = (0xC8 - udpMsgBuf[1] + 0x100) & 0xFF;
		  				doResends = 0;
		  				colorSent++;
		  				//reset disco mode
	  					discoSequence = 255;
		  				break;
		  			case 0x41:
		  				//all off
		  				button = 2;
		  				break;
		  			case 0x42:
		  				//all on
		  				button = 1;
	  					currentGroupControl = 0;
		  				break;
		  			case 0xC1:
		  				//all to night mode
		  				button = 0x12;
	  					discoSequence = 255; //reset disco mode
		  				break;
		  			case 0xC2:
		  				//white all on
		  				button = 0x11;
	  					currentGroupControl = 0;
	  					discoSequence = 255; //reset disco mode
		  				break;
		  			case 0x43:
		  				//disco faster
		  				button = 0x0b;
		  				break;
		  			case 0x44:
		  				//disco slower
		  				button = 0x0c;
		  				break;
		  			case 0x4D:
		  				//disco mode
		  				button = 0x0d;
		  				if (discoSequence == 255) {
		  				    discoSequence = 0;
		  				} else {
		  				    discoSequence++;
		  					if (discoSequence > 8) {
			  					discoSequence = 0;
		  					}
		  				}
		  				colorSent = 0;
		  				brightnessSent = 0;
		  				break;
		  			case 0x4E:
		  				//brightness
		  				button = 0x0e;
		  				brightness = ((0x90 - (udpMsgBuf[1] * 8) + 0x100) & 0xFF);
		  				brightnessSent++;
		  				doResends = 0;
		  				//valid from 0x80 to 0 till 0xb8
		  				break;
		  		}
		  	}
		  	if (button >= 0) {
	  			seqn++;
	  			if (discoSequence != 255) {
	  			    outPacket[0] = 0xB0 | discoSequence;
	  			} else
	  			if (button >= 0x0e) {
	  				outPacket[0] = 0xB0;
	  			} else {
	  				//color/bright buttons and long presses are 0xB0 [or disco], others are 0xB8
	  				outPacket[0] = 0xB8;
	  			}
	  			outPacket[3] = color;
	  			outPacket[4] = (brightness&0xF8) | currentGroupControl;
	  			outPacket[5] = button;
	  			outPacket[6] = seqn;
	  			sendRadio("OUT", silent, verbose, outPacket, sizeof(outPacket), doResends, resendMicroDelay);
	  			if (doResends >= 0) {
	  				//outPacket[5] = 0; //don not resend package below (no hammering detected - so resending has been done already here)
	  			}
		  	}
  		}
  	} else {
  		if (packetHammer > 0) {
  			packetHammer--;
  		}
  		int doResends = resends;
  		if (colorSent > 0) {
	  		seqn++;
  			colorSent = 0;
	  		outPacket[5] = 0x0f;
	  		outPacket[6] = seqn;
	  		sendRadio("OUTEND", silent, verbose, outPacket, sizeof(outPacket), doResends, resendMicroDelay);
  		}
  		if (brightnessSent > 0) {
	  		seqn++;
  			brightnessSent = 0;
	  		outPacket[5] = 0x0e;
	  		outPacket[6] = seqn;
  			sendRadio("OUTEND", silent, verbose, outPacket, sizeof(outPacket), doResends, resendMicroDelay);
  		}
  		
  		if (outPacket[5] != 0) {
  			outPacket[5] = 0;
  			sendRadio("OUTEND", silent, verbose, outPacket, sizeof(outPacket), doResends, resendMicroDelay);
  		}
  		
  		//only do discovery if I have nothing else to do
  		if (discoverUdp != NULL) {
	  		size = discoverUdp->timed_recv(udpMsgBuf, UDP_MSG_BUF_SIZE, 1);
	  		if (size > 0) {
	  			if(!strncmp(udpMsgBuf, "Link_Wi-Fi", 10)) {
	  				discoverUdp->respond(discoveryResponse.c_str(), discoveryResponse.length());
	  			}
	  		}
  		}
  	}
  }
  
  delete udp;
  if (discoverUdp != NULL) delete discoverUdp;
  printf("Ended gracefully\n");

  return 0;
}
