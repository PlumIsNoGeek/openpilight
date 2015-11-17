# openpilight

The openpilight project brings the radio control of the Milight/LimitlessLED lights to the raspberry
pi (and other boards supporting wiring Pi like the great banana pi I personally use because I want SATA).

It tries to fully be compatible with the Milight UDP protocol to have the lights controlled be remote
clients like mobile phone apps as well as support the association/reset features (which I had many problems
with using other gateways). I personally like to buy more bulbs and more bulbs on no remotes, so this
project's intent is to totally work without having a remote/WifiGateway.

It is based on the great work of:
* openmilight of henryk - https://hackaday.io/project/5888-reverse-engineering-the-milight-on-air-protocol
* Torsten Tränkner - http://torsten-traenkner.de/wissen/smarthome/openmilight.php
* bakkerr - https://github.com/bakkerr/openmilight_pi
