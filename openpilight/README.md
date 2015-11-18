#openpilight Communication Gateway

The openpilight communication gateway provides the functionality of the "LimitlessLED"/"Milight LED"
Wifi-gateway. It takes UDP commands (also supports UDP discovery) and send 2,4GHz radio using the
nRF24L01+ radio module. It is designed to run on raspberry pi architecture boards (tested on banana
pi and raspberry pi model B, if you have success on another platform else, please let me know -
also if there are changes to this SW needed to run on another platform). There is no additional
remote as well as no wifi gateway needed, when using this software.

It is compatible with the Milight (and third party) Android/Ios/... apps as well as other sources
around on the net.

##Build Instructions

use make inside openpilight directory

Required libraries:
* librf24-wpi (see this repository)
* libwiringPi (needed for librf24-wpi to work)

##openpilight Communication Gateway usage instructions
```
Start Server: openpilight [options]

radio options:
     -a hwAddress      hardware address to be sent within 2,4GHz commands which identifies the
                       sending remote given in HEX - valid values are from 0001 till FFFF
                       (default: 0001)
     -c radioChannel   channel for sending 2,4GHz, valid are 9, 40, 71 - 0 will send on all three
                       channes (default: 0)
     -r                number of resends of radio packets - except color and brightness (default: 10)
     -d                delay between resends of packages in microseconds (default: 0)
     -u                send key up events after key events (default: disabled)
     -l                activates radio listen mode (server disabled - just listening for radio packages)
UDP server options:
     -i ipAddress      interface ip address to bind the UDP socket to (default: 0.0.0.0)
     -p port           port to listen for UDP packets (default: 8899)
     -x addressString  support automatic discovery by other apps - addressString will containt the response
                       string which is sent for identification typically is the IP address (hostname not supported)
                       of the server (default: disabled)
misc options:
     -v                enables verbose output (default: disabled)
     -t                profile timing (default: disabled)
     -s                activates silent mode - no radio output (default: disabled)
     -h                shows this help
```

##UDP commands

The standard Milight UDP protocol is implemented including discovery.

In addition there are some more UDP packages understood:
- 9 bytes size, 0xff ..... 0x55 with 7 bytes payload in between is raw radio data sent to the LED bulbs
- 5 bytes size, 0xff [ADDRHI] [ADDRLO] [GRP] 0x55 - sends an association command to the bulb (power on bulb and then send the sequence within 2 seconds), ADDRHI and ADDRLO are the remote address bytes to train (same as -a hwAddress later for the server), GRP is the group button that should be trained (0 = all, 1 = group 1 ....)


