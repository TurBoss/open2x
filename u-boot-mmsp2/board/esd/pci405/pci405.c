/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <command.h>
#include <malloc.h>
#include <pci.h>
#include <405gp_pci.h>

#include "pci405.h"


/* ------------------------------------------------------------------------- */
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);/*cmd_boot.c*/
#if 0
#define FPGA_DEBUG
#endif

/* fpga configuration data - generated by bin2cc */
const unsigned char fpgadata[] =
{
#include "fpgadata.c"
};

/*
 * include common fpga code (for esd boards)
 */
#include "../common/fpga.c"


/* Prototypes */
int gunzip(void *, int, unsigned char *, int *);


int board_pre_init (void)
{
	unsigned long cntrl0Reg;

	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) CAN1; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) CAN2; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) CAN3; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) unused; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) FPGA Timestamp; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) PCI Reset; active low; level sensitive
	 */
	mtdcr(uicsr, 0xFFFFFFFF);        /* clear all ints */
	mtdcr(uicer, 0x00000000);        /* disable all ints */
	mtdcr(uiccr, 0x00000000);        /* set all to be non-critical*/
	mtdcr(uicpr, 0xFFFFFF80);        /* set int polarities */
	mtdcr(uictr, 0x10000000);        /* set int trigger levels */
	mtdcr(uicvcr, 0x00000001);       /* set vect base=0,INT0 highest priority*/
	mtdcr(uicsr, 0xFFFFFFFF);        /* clear all ints */

	/*
	 * Setup GPIO pins (IRQ4/GPIO21 as GPIO)
	 */
	cntrl0Reg = mfdcr(cntrl0);
	mtdcr(cntrl0, cntrl0Reg | 0x00008000);

	/*
	 * EBC Configuration Register: set ready timeout to 512 ebc-clks -> ca. 25 us
	 */
	mtebc (epcr, 0xa8400000); /* ebc always driven */

	return 0;
}


/* ------------------------------------------------------------------------- */

int misc_init_f (void)
{
	return 0;  /* dummy implementation */
}


int misc_init_r (void)
{
	unsigned char *dst;
	ulong len = sizeof(fpgadata);
	int status;
	int index;
	int i;
	unsigned int *ptr;
	unsigned int *magic;

	/*
	 * On PCI-405 the environment is saved in eeprom!
	 * FPGA can be gzip compressed (malloc) and booted this late.
	 */

	dst = malloc(CFG_FPGA_MAX_SIZE);
	if (gunzip (dst, CFG_FPGA_MAX_SIZE, (uchar *)fpgadata, (int *)&len) != 0) {
		printf ("GUNZIP ERROR - must RESET board to recover\n");
		do_reset (NULL, 0, 0, NULL);
	}

	status = fpga_boot(dst, len);
	if (status != 0) {
		printf("\nFPGA: Booting failed ");
		switch (status) {
		case ERROR_FPGA_PRG_INIT_LOW:
			printf("(Timeout: INIT not low after asserting PROGRAM*)\n ");
			break;
		case ERROR_FPGA_PRG_INIT_HIGH:
			printf("(Timeout: INIT not high after deasserting PROGRAM*)\n ");
			break;
		case ERROR_FPGA_PRG_DONE:
			printf("(Timeout: DONE not high after programming FPGA)\n ");
			break;
		}

		/* display infos on fpgaimage */
		index = 15;
		for (i=0; i<4; i++) {
			len = dst[index];
			printf("FPGA: %s\n", &(dst[index+1]));
			index += len+3;
		}
		putc ('\n');
		/* delayed reboot */
		for (i=20; i>0; i--) {
			printf("Rebooting in %2d seconds \r",i);
			for (index=0;index<1000;index++)
				udelay(1000);
		}
		putc ('\n');
		do_reset(NULL, 0, 0, NULL);
	}

	puts("FPGA:  ");

	/* display infos on fpgaimage */
	index = 15;
	for (i=0; i<4; i++) {
		len = dst[index];
		printf("%s ", &(dst[index+1]));
		index += len+3;
	}
	putc ('\n');

	/*
	 * Reset FPGA via FPGA_DATA pin
	 */
	SET_FPGA(FPGA_PRG | FPGA_CLK);
	udelay(1000); /* wait 1ms */
	SET_FPGA(FPGA_PRG | FPGA_CLK | FPGA_DATA);
	udelay(1000); /* wait 1ms */

	/*
	 * Check if magic for pci reconfig is written
	 */
	magic = (unsigned int *)0x00000004;
	if (*magic == PCI_RECONFIG_MAGIC) {
		/*
		 * Rewrite pci config regs (only after soft-reset with magic set)
		 */
		ptr = (unsigned int *)PCI_REGS_ADDR;
		if (crc32(0, (char *)PCI_REGS_ADDR+4, PCI_REGS_LEN-4) == *ptr) {
			puts("Restoring PCI Configurations Regs!\n");
			ptr = (unsigned int *)PCI_REGS_ADDR + 1;
			for (i=0; i<0x40; i+=4) {
				pci_write_config_dword(PCIDEVID_405GP, i, *ptr++);
			}
		}
		mtdcr(uicsr, 0xFFFFFFFF);        /* clear all ints */

		*magic = 0;      /* clear pci reconfig magic again */
	}

	free(dst);
	return (0);
}


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	unsigned char str[64];
	int i = getenv_r ("serial#", str, sizeof(str));

	puts ("Board: ");

	if (i == -1) {
		puts ("### No HW ID - assuming PCI405");
	} else {
		puts (str);
	}
	putc ('\n');

	return 0;
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	unsigned long val;

	mtdcr(memcfga, mem_mb0cf);
	val = mfdcr(memcfgd);

#if 0
	printf("\nmb0cf=%x\n", val); /* test-only */
	printf("strap=%x\n", mfdcr(strap)); /* test-only */
#endif

#if 0 /* test-only: all PCI405 version must report 16mb */
	return (4*1024*1024 << ((val & 0x000e0000) >> 17));
#else
	return (16*1024*1024);
#endif
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: 16 MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */
