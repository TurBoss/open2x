/*
 * drivers/sound/mp2520f-wm97xx.c
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MP2520F AC97 audio driver
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/sound.h>
#include <linux/soundcard.h>
#include <linux/ac97_codec.h>
#include <linux/pm.h>
#include <linux/proc_fs.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/semaphore.h>
#include <asm/dma.h>
#include <asm/arch/mmsp20.h>

//#include <asm/arch/mdk.h>
#include <linux/sched.h>
#include <asm/arch/mdk.h>
//add Markbh(	
#include <linux/interrupt.h>

#include "mp2520f-audio.h"

#ifdef CONFIG_MACH_MMSP2_DTK3
	#define AC97_ALC650 
#endif

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif


static DECLARE_WAIT_QUEUE_HEAD(wq);
#define IRQ_JACK_DECT	IRQ_GPIO_L(11)
static struct tq_struct jack_detect_task;

static struct completion CAR_completion;
static int waitingForMask;
static DECLARE_MUTEX(CAR_mutex);

static int audio_dev_dsp;
static int audio_dev_mixer;

static struct ac97_codec mp2520f_ac97_codec;

static audio_stream_t ac97_audio_out;
static int audio_jack_dect;
static int int_cnt=0;

#ifdef CONFIG_PROC_FS

#ifndef AC97_ALC650
#define WM97XX_PROC_NAME	"wm9710"
#else
#define WM97XX_PROC_NAME	"alc650"
#endif

static struct proc_dir_entry *wm97xx_dir = NULL;
static struct proc_dir_entry *registers_dir = NULL;

typedef struct wm97xx_reg_entry {
	char* name;
	int  offset;
} wm97xx_reg_entry_t;

static wm97xx_reg_entry_t wm97xx_registers[] = {
#ifndef AC97_ALC650
#include "wm97xx_registers.h"
#else
#include "alc650_registers.h"
#endif
};



int wm97xx_register_proc_entry(char *name, read_proc_t *_read)
{ return create_proc_read_entry(name, 0, wm97xx_dir, _read, NULL) ? 0:-EINVAL; }

void wm97xx_unregister_proc_entry(char *name)
{ remove_proc_entry(name, wm97xx_dir); }

static ssize_t wm97xx_proc_read_reg(struct file * file, char * buf,
				    size_t nbytes, loff_t *ppos)
{
	struct ac97_codec *codec = &mp2520f_ac97_codec;
	struct inode * my_inode = file->f_dentry->d_inode;
	struct proc_dir_entry * dp;
	wm97xx_reg_entry_t *current_reg = NULL;
	char outputbuf[15];
	int count;

	if (*ppos > 0)
		return 0;

	dp = (struct proc_dir_entry *)my_inode->u.generic_ip;
	current_reg = (wm97xx_reg_entry_t *)(dp->data);

	if (current_reg == NULL)
		return -EINVAL;

	count = sprintf(&outputbuf[0], "0x%04x\n",
		codec->codec_read(codec, current_reg->offset));

	*ppos += count;

	if (count > nbytes)
		return -EINVAL;
	if (copy_to_user(buf, &outputbuf[0], count))
		return -EFAULT;

	return count;
}

static ssize_t wm97xx_proc_write_reg(struct file * file, const char * buffer,
				     size_t count, loff_t *ppos)
{
	struct ac97_codec *codec = &mp2520f_ac97_codec;
	struct inode * my_inode = file->f_dentry->d_inode;
	struct proc_dir_entry * dp;
	wm97xx_reg_entry_t *current_reg = NULL;
	unsigned long new_reg_value;
	char *endp;

	dp = (struct proc_dir_entry *)my_inode->u.generic_ip;
	current_reg = (wm97xx_reg_entry_t *)(dp->data);

	if (current_reg == NULL)
		return -EINVAL;

	new_reg_value = simple_strtoul(buffer, &endp, 0);
	codec->codec_write(codec, current_reg->offset, new_reg_value & 0xFFFF);
	return (count + endp - buffer);
}

static struct file_operations wm97xx_proc_reg_fops = {
	.read	= wm97xx_proc_read_reg,
	.write	= wm97xx_proc_write_reg,
};

void inline wm97xx_create_proc_interface(void)
{
	struct proc_dir_entry *entry;
	int i;

	wm97xx_dir = proc_mkdir(WM97XX_PROC_NAME, proc_root_driver);
	registers_dir = proc_mkdir("registers", wm97xx_dir);

	for (i = 0; i < ARRAY_SIZE(wm97xx_registers); i++) {
		entry = create_proc_entry(wm97xx_registers[i].name,
				S_IWUSR |S_IRUSR | S_IRGRP | S_IROTH,
				registers_dir);
		entry->proc_fops = &wm97xx_proc_reg_fops;
		entry->data = &wm97xx_registers[i];
	}
}

void inline wm97xx_destroy_proc_interface(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(wm97xx_registers); i++)
		remove_proc_entry(wm97xx_registers[i].name, registers_dir);
	remove_proc_entry("registers", wm97xx_dir);

	remove_proc_entry(WM97XX_PROC_NAME, proc_root_driver);
}
#else
#define wm97xx_create_proc_interface() (void)(0)
#define wm97xx_destroy_proc_interface() (void)(0)
#endif


static u16 mp2520f_ac97_read(struct ac97_codec *codec, u8 reg)
{
	u16 val = -1;

	down(&CAR_mutex);

	if (!(AC_CAR & AC_CAR_CAIP)) {
		u32 reg_addr;

		waitingForMask = AC_GSR_RD;
		init_completion(&CAR_completion);
		AC_STA_EN |= AC_STA_RD;
		reg_addr = (u32)&AC_REG_BASE + reg;

		*(volatile u16 *)(reg_addr);
		wait_for_completion(&CAR_completion);

		udelay(50);

		val = AC_DATA;
	} else {
		printk(KERN_CRIT "%s : AC_CAR_CAIP already set\n", __func__);
	}

	up(&CAR_mutex);

	if (AC_ADDR != reg) 
		printk("AC_ADDR: 0x%02x, reg: 0x%02x\n", AC_ADDR, reg);

	ds_printk("(0x%02x) = 0x%04x\n", reg, val);

	return val;
}

static void mp2520f_ac97_write(struct ac97_codec *codec, u8 reg, u16 val)
{
	down(&CAR_mutex);

	if (!(AC_CAR & AC_CAR_CAIP)) {
		u32 reg_addr;

		waitingForMask = AC_GSR_WD;
		init_completion(&CAR_completion);
		AC_STA_EN |= AC_STA_WD;
		reg_addr = (u32)&AC_REG_BASE + reg;

		*(volatile u16 *)reg_addr = val;
		wait_for_completion(&CAR_completion);

		udelay(50);
	} else {
		printk(KERN_CRIT "%s : AC_CAR_CAIP already set\n", __func__);
	}

	if (AC_ADDR != reg) 
		printk("AC_ADDR: 0x%02x, reg: 0x%02x\n", AC_ADDR, reg);

	ds_printk("(0x%02x, 0x%04x)\n", reg, val);

	up(&CAR_mutex);
}

static int fout_underflow_check = 0; // hhsong modify
static int fout_underflow_gen = 0;

#if 1
static void mp2520f_ac97_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	int gsr = AC_GSR;
	AC_GSR = gsr & (AC_GSR_WD|AC_GSR_RD);		/* clear */

	if (gsr & waitingForMask)
	{
		AC_STA_EN &= ~(AC_STA_WD|AC_STA_RD);	/* disable */
		complete(&CAR_completion);
	}
}
#else
static void mp2520f_ac97_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	int gsr = AC_GSR;
	if(fout_underflow_check)
	{
		printk("gsr = ox%x\n",gsr);
		printk("AC_GSR_FOUT_UF = ox%x\n",AC_GSR_FOUT_UF);
		if(gsr & AC_GSR_FOUT_UF)
		{
			printk("AC_GSR_FOUT_UF gen!!\n");
			fout_underflow_gen = 1;			
		}
		AC_GSR &=  ~AC_GSR_FOUT_UF;     /* clear */
		AC_STA_EN &= ~AC_STA_FOUT_UF; 	/* disable */
		fout_underflow_check = 0;
	}
	else
	{
		AC_GSR = gsr & (AC_GSR_WD|AC_GSR_RD);		/* clear */

		if (gsr & waitingForMask)
		{
			AC_STA_EN &= ~(AC_STA_WD|AC_STA_RD);	/* disable */
			complete(&CAR_completion);
		}
	}
}
#endif

static void jack_task_handler(void *data)
{
	u16 val;
	u32 reg_addr;
	
	//Verify is again dected
	if(read_gpio_bit(GPIO_L11)==audio_jack_dect) 
	{
		if(read_gpio_bit(GPIO_L11))
		{
			audio_jack_dect=0;
			set_external_irq(IRQ_JACK_DECT, EINT_LOW_LEVEL, GPIOPU_NOSET); 
		}
		else
		{
			audio_jack_dect=1;
			set_external_irq(IRQ_JACK_DECT, EINT_HIGH_LEVEL, GPIOPU_NOSET); 		
		}		
		
		printk("Jack dected BUG....\n");
		return;
	}
	
	if (!(AC_CAR & AC_CAR_CAIP)) //read
	{
		waitingForMask = AC_GSR_RD;
		AC_STA_EN |= AC_STA_RD;
		reg_addr = (u32)&AC_REG_BASE + 0x02;
		*(volatile u16 *)(reg_addr);
		udelay(50);
		val = AC_DATA;
	}
	if (AC_ADDR != 0x02) 
		printk("AC_ADDR: 0x%02x, reg: 0x%02x\n", AC_ADDR, val);

	if(audio_jack_dect) val|=0x8000;
	else val&=~0x8000;
	
	if (!(AC_CAR & AC_CAR_CAIP)) 
	{
		waitingForMask = AC_GSR_WD;
		AC_STA_EN |= AC_STA_WD;
		reg_addr = (u32)&AC_REG_BASE + 0x02;
		*(volatile u16 *)reg_addr = (val);
		udelay(50);
	} 
	
	if(AC_ADDR != 0x02) 
		printk("AC_ADDR: 0x%02x, reg: 0x%02x\n", AC_ADDR, val);

}

///////////////////////////////////////////////////////////////////////////
static void jack_detect_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	/* low=jack ON*/
	if(read_gpio_bit(GPIO_L11)==audio_jack_dect) 
	{
		udelay(20);		//old 50
		
		if(int_cnt++ >= 25) 	//old 15	
		{	
			if(read_gpio_bit(GPIO_L11)) 
			{
				audio_jack_dect=0;
				set_external_irq(IRQ_JACK_DECT, EINT_LOW_LEVEL, GPIOPU_NOSET); 
			}
			else
			{
		 		audio_jack_dect=1;
		 		set_external_irq(IRQ_JACK_DECT, EINT_HIGH_LEVEL, GPIOPU_NOSET); 		
			}	
			
			int_cnt=0;
			
			*(volatile unsigned short *)io_p2v(0xc0001116) |=(1<<11);	//mask
			queue_task(&jack_detect_task,&tq_immediate);
			mark_bh(IMMEDIATE_BH);
		}
	}
	else{
		udelay(20);		
		int_cnt=0;
		 *(volatile unsigned short *)io_p2v(0xc0001116) |=(1<<11);	//mask
	}
}	

///////////////////////////////////////////////////////////////////////////

static struct ac97_codec mp2520f_ac97_codec = {
	.codec_read	 = mp2520f_ac97_read,
	.codec_write = mp2520f_ac97_write,
};

static DECLARE_MUTEX(mp2520f_ac97_mutex);
static int mp2520f_ac97_refcount=0;

int mp2520f_ac97_get(struct ac97_codec **codec)
{
	int ret;
	MMSP20_CLOCK_POWER *pPMR_REG;			
	
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();	

	*codec = NULL;
	down(&mp2520f_ac97_mutex);

	if (!mp2520f_ac97_refcount) {

		set_gpio_ctrl(GPIO_L6, GPIOMD_ALT1, GPIOPU_NOSET);
		set_gpio_ctrl(GPIO_L7, GPIOMD_ALT1, GPIOPU_NOSET);
		set_gpio_ctrl(GPIO_L8, GPIOMD_ALT1, GPIOPU_NOSET);
		set_gpio_ctrl(GPIO_L9, GPIOMD_ALT1, GPIOPU_NOSET);
		set_gpio_ctrl(GPIO_L10, GPIOMD_ALT1, GPIOPU_NOSET);

#ifdef CONFIG_MACH_GP2X
		set_gpio_ctrl(GPIO_L11, GPIOMD_IN, GPIOPU_NOSET);
		if( read_gpio_bit(GPIO_L11) ) audio_jack_dect=0;
		else audio_jack_dect=1;

		//set_external_irq(IRQ_JACK_DECT, EINT_FALLING_EDGE, GPIOPU_EN); 		
#endif

		pPMR_REG->ASCLKENREG &= ~I2SCLK;	/* I2S clock disable */
		pPMR_REG->ASCLKENREG |= AC97CLK;	/* ac97 clock enable */

		pPMR_REG->AUDICSETREG &= ~(AUDCLKSRC | AUDCLKDIV);	/* source:ABIT_CLK divider:0 */
		udelay(100);

		AC_CTL = (AC_CTL_NORM_OP|AC_CTL_ACLINK_OFF); /* ac-link off */
		AC_CTL = 0;				/* cold reset */

		udelay(100);
		AC_CTL = (AC_CTL_NORM_OP|AC_CTL_ACLINK_OFF); /* ac-link off */

		AC_CTL = AC_CTL_NORM_OP;	/* normal op & ac-link on */
		udelay(1000);

		AC_STA_EN |= AC_STA_REDAY;	/* codec ready */
		while (!(AC_GSR & AC_GSR_REDAY)) {
			schedule();
		}
		AC_STA_EN &= ~AC_STA_REDAY;
		AC_GSR |= AC_GSR_REDAY;		/* clear */

		ret = request_irq(IRQ_AC97, mp2520f_ac97_irq, 0, "AC97", NULL);
		if (ret) {
			up(&mp2520f_ac97_mutex);
			return ret;
		}

#ifndef CONFIG_MACH_GP2X		
		ret = request_irq(IRQ_JACK_DECT, jack_detect_interrupt,SA_INTERRUPT, "JACK_DECT",NULL);
		if (ret) {
			printk("Audio jack dect : request_irq failed\n");
			free_irq(IRQ_AC97, NULL);
			AC_CTL = AC_CTL_ACLINK_OFF;
			pPMR_REG->ASCLKENREG &= ~AC97CLK;
			up(&mp2520f_ac97_mutex);
			return ret;
		}
		queue_task
#endif
		

		AC_CONFIG = 0;			/* data width 16bit */

		ret = ac97_probe_codec(&mp2520f_ac97_codec);
		if (ret != 1) {
			free_irq(IRQ_AC97, NULL);
			AC_CTL = AC_CTL_ACLINK_OFF;
			pPMR_REG->ASCLKENREG &= ~AC97CLK;	
			up(&mp2520f_ac97_mutex);
			return ret;
		}

#ifdef CONFIG_MACH_GP2X		
		if(audio_jack_dect) set_external_irq(IRQ_JACK_DECT, EINT_HIGH_LEVEL, GPIOPU_NOSET); 		
		else set_external_irq(IRQ_JACK_DECT, EINT_LOW_LEVEL, GPIOPU_NOSET); 		
		
		ret = request_irq(IRQ_JACK_DECT, jack_detect_interrupt, 0,"JACK_DECT",NULL);
		if (ret) 
		{
			printk("Audio jack dect : request_irq failed\n");
			free_irq(IRQ_AC97, NULL);
			AC_CTL = AC_CTL_ACLINK_OFF;
			pPMR_REG->ASCLKENREG &= ~AC97CLK;
			up(&mp2520f_ac97_mutex);
			return ret;
		}
		
		jack_detect_task.data = NULL;
		jack_detect_task.routine = jack_task_handler;
#endif
	}
	mp2520f_ac97_refcount++;

	up(&mp2520f_ac97_mutex);
	*codec = &mp2520f_ac97_codec;

	return 0;
}

void mp2520f_ac97_put(void)
{
	MMSP20_CLOCK_POWER *pPMR_REG;		
	
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();

	down(&mp2520f_ac97_mutex);
	mp2520f_ac97_refcount--;
	if (!mp2520f_ac97_refcount) {
		AC_CTL = AC_CTL_ACLINK_OFF;
		pPMR_REG->ASCLKENREG &= ~AC97CLK;
		free_irq(IRQ_AC97, NULL);

#ifdef CONFIG_MACH_GP2X 		
		free_irq(IRQ_JACK_DECT,NULL);
#endif	

	}
	
	up(&mp2520f_ac97_mutex);
}

EXPORT_SYMBOL(mp2520f_ac97_get);
EXPORT_SYMBOL(mp2520f_ac97_put);

static audio_state_t ac97_audio_state;
static audio_stream_t ac97_audio_in;

static int mixer_ioctl( struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int ret;

	ret = mp2520f_ac97_codec.mixer_ioctl(&mp2520f_ac97_codec, cmd, arg);
	if (ret)
		return ret;

	return 0;
}

static struct file_operations mixer_fops = {
	.ioctl		= mixer_ioctl,
	.llseek		= no_llseek,
	.owner		= THIS_MODULE,
};



static int codec_adc_rate = 48000;
static int codec_dac_rate = 48000;

static int ac97_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg)
{
	int ret;
	long val = 0;

	switch(cmd) {
	case SNDCTL_DSP_STEREO:
		if (get_user(val, (int *) arg))
			return -EINVAL;
		ac97_audio_out.channels = (val) ? 2 : 1;
		return 0;
		
	case SNDCTL_DSP_CHANNELS:
		if (get_user(val, (int *) arg))
			return -EINVAL;
		if (val != 1 && val != 2)
			return -EINVAL;
		ac97_audio_out.channels = val;
		return put_user(val, (int *) arg);

	case SOUND_PCM_READ_CHANNELS:
		return put_user(ac97_audio_out.channels, (long *) arg);

	case SNDCTL_DSP_SPEED:
		ret = get_user(val, (long *) arg);
		if (ret)
			return ret;
		if (file->f_mode & FMODE_READ)
			codec_adc_rate = ac97_set_adc_rate(&mp2520f_ac97_codec, val);
		if (file->f_mode & FMODE_WRITE)
			codec_dac_rate = ac97_set_dac_rate(&mp2520f_ac97_codec, val);
		return put_user(val, (long *) arg); 
		/* fall through */  // Why?

	case SOUND_PCM_READ_RATE:
		if (file->f_mode & FMODE_READ)
			val = codec_adc_rate;
		if (file->f_mode & FMODE_WRITE)
			val = codec_dac_rate;
		return put_user(val, (long *) arg);

	case SNDCTL_DSP_SETFMT:
	case SNDCTL_DSP_GETFMTS:
		/* FIXME: can we do other fmts? */
		return put_user(AFMT_S16_LE, (long *) arg);
		
	default:
		/* Maybe this is meant for the mixer (As per OSS Docs) */
		return mixer_ioctl(inode, file, cmd, arg);
	}
	return 0;
}

static audio_stream_t ac97_audio_out = {
	.name		= "AC97 audio out",
	.dma_ch		= 1,
};

static audio_stream_t ac97_audio_in = {
	.name		= "AC97 audio in",
	.dma_ch		= 2,
};

static audio_state_t ac97_audio_state = {
	.output_stream		= &ac97_audio_out,
	.input_stream		= &ac97_audio_in,
	.output_id			= "MP2520F AC97 out",
	.client_ioctl		= ac97_ioctl,
	.sem				= __MUTEX_INITIALIZER(ac97_audio_state.sem),
};

static int ac97_audio_open(struct inode *inode, struct file *file)
{
	int value;	
	
	value=mp2520f_audio_attach(inode, file, &ac97_audio_state);
	if(audio_jack_dect) mp2520f_ac97_write(&mp2520f_ac97_codec, 0x02, 0x8000); /* SPK MUTE */
	
	return value;
}

static struct file_operations ac97_audio_fops = {
	.open		= ac97_audio_open,
	.owner		= THIS_MODULE,
};

static int __init mp2520f_ac97_init(void)
{
	int ret;
	struct ac97_codec *codec;

	ret = mp2520f_ac97_get(&codec);
	if (ret)
		return ret;

	wm97xx_create_proc_interface();

	audio_dev_dsp = register_sound_dsp(&ac97_audio_fops, -1);
	audio_dev_mixer = register_sound_mixer(&mixer_fops, -1);

	return 0;
}

static void __exit mp2520f_ac97_exit(void)
{
	wm97xx_destroy_proc_interface();

	unregister_sound_dsp(audio_dev_dsp);
	unregister_sound_mixer(audio_dev_mixer);

	mp2520f_ac97_put();
}


module_init(mp2520f_ac97_init);
module_exit(mp2520f_ac97_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION ("Audio driver for MP2520F wm97xx codec.");
