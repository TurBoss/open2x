/*
 * linux/arch/arm/mach-sa1100/leds-system3.c
 *
 * Copyright (C) 2001 Stefan Eletzhofer <stefan.eletzhofer@gmx.de>
 *
 * Original (leds-footbridge.c) by Russell King
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * $Log$
 * Revision 1.1  2005/12/01 14:21:42  djwillis
 * Initial check-in of GamePark Holdings/DignSys's 1st kernel source code release.
 * See http://www.distant-earth.com/gp2x/2005/12/gp2x-linux-1st-source-release.html for info.
 *
 * Revision 1.1.6.1  2001/12/04 15:19:26  seletz
 * - merged from linux_2_4_13_ac5_rmk2
 *
 * Revision 1.1.4.2  2001/11/19 17:58:53  seletz
 * - cleanup
 *
 * Revision 1.1.4.1  2001/11/16 13:49:54  seletz
 * - dummy LED support for PT Digital Board
 *
 * Revision 1.1.2.1  2001/10/15 16:03:39  seletz
 * - dummy function
 *
 *
 */
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>

#include "leds.h"


#define LED_STATE_ENABLED	1
#define LED_STATE_CLAIMED	2

static unsigned int led_state;
static unsigned int hw_led_state;

void system3_leds_event(led_event_t evt)
{
	/* TODO: support LEDs */
}
