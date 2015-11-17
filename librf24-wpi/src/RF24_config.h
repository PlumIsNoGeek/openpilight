/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.

 */
#ifndef __RF24_CONFIG_H__
#define __RF24_CONFIG_H__

#define SPI_COMMUNICATION_FREQUENCY	12000000
//#define SPI_COMMUNICATION_FREQUENCY	2000000

#define _BV(x) (1<<(x))
#define SERIAL_DEBUG 1

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
#define IF_SERIAL_DEBUG(x) ({x;})
#else
#define IF_SERIAL_DEBUG(x)
#endif

#define pgm_read_word(p) (*(p))

#endif // __RF24_CONFIG_H__
