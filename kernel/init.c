/*
init.c
Has initialisation functions for the kernel, and contains the entry point
*/

//Headers

#include <types.h>
#include <textmode.h>
#include <res/strings.h>
#include <stdio.h>
#include <multiboot.h>
#include <arch/x86.h>
#include <devices/x86.h>
#include <arch/stacktrace.h>
#include <elf.h>
#include <vt.h>

//Prototypes
//TODO:Add to includes

void panic(char* reason);
void halt(char* reason);

char kb_readFromBuffer(int index);
char kb_popNextFromBuffer();

//Helper functions


/**
Draws the bar on the top
**/
void drawBar()
{
	volatile unsigned char *videoram = (unsigned char *)0xB8000;
	int i=0;
	int v=40-5; // The place to start drawing (v = vertical). 5 characters before 40
	//Cleans Screen
	tm_clear();
	while(i!=(80*2)) //Draws background
	{
		videoram[i] = ' ';// Space
		videoram[i+1] = 0x70;// Grey
		i=i+2;
	}

	//Draw tite
	i=v*2;
	videoram[i] = 'e';
	i=i+2;
	videoram[i] = 'l';
	i=i+2;
	videoram[i] = 'e';
	i=i+2;
	videoram[i] = 'm';
	i=i+2;
	videoram[i] = 'e';
	i=i+2;
	videoram[i] = 'n';
	i=i+2;
	videoram[i] = 't';
	i=i+2;
	videoram[i] = 'O';
	i=i+2;
	videoram[i] = 'S';
	i=i+2;
	// End title drawing
}

//Init

/**
Main process init point
**/
int kinit_x86(int magic, multiboot_header_t *multiboot)
{
	//Setup
	drawBar();
	tm_clear();

	//Print start info
	printf("%^%s v.%s (%s) (%s)...%^\n",0x09,RES_STARTMESSAGE_S,RES_VERSION_S,RES_SOURCE_S,RES_ARCH_S,0x0F);
	printf("%^Codename:\"%s\"%^\n",0x09,RES_CODENAME_S,0x0F);

	//Verify Multiboot magic number
	if (magic!=0x2BADB002)
	{
		log("BOOT",0x02,"Magic number unverified!\n");
		panic("Booted in inconsistent state");
	}
	printf("%^%s booted elementOS up properly!%^\n",0x04,multiboot->boot_loader_name,0x0F);

	//Print memory
	int memtotal = (multiboot->mem_upper)+(multiboot->mem_lower);
	int memtotalmb = memtotal/1024;
	printf("%^%d kb%^ high, %^%d kb%^ low; a total of %^~%d mb%^ %^(%d kb)%^\n",0x03,multiboot->mem_upper,0x0F,0x0C,multiboot->mem_lower,0x0F,0x02,memtotalmb+1,0x0F,0x0A,memtotal,0x0F);

	//System initialising
	printf("--------------------------------------------------------------------------------");
	printf("Initialising system...\n");

	//For x86

	//GDT
	if(gdt_install()==0)
	{
		log(" OK ",0x02,"Installed GDT\n");
	}
	else
	{
		log("FAIL",0x02,"GDT installation failed. Kernel cannot initialise!\n");
		halt("GDT could not initialise");
	}

	if(idt_install()==0)
	{
		log(" OK ",0x02,"Installed IDT\n");
	}
	else
	{
		log("FAIL",0x02,"IDT installation failed. Kernel cannot initialise!\n");
		halt("IDT could not initialise");
	}

	if(isrs_install()==0)
	{
		log(" OK ",0x02,"Installed ISR's\n");
	}
	else
	{
		log("FAIL",0x02,"ISR installation failed. Kernel cannot initialise!\n");
		halt("ISR's could not initialise");
	}

	if(irq_install()==0)
	{
		log(" OK ",0x02,"Installed IRQ handlers\n");
	}
	else
	{
		log("FAIL",0x02,"IRQ handlers installation failed. Kernel cannot initialise!\n");
		halt("IRQ handlers could not initialise");
	}

	//PIT Setup
	{
		//volatile unsigned char *videoram = (unsigned char *)0xB8000;
		pit_install();
		//pit_phase(1000);
		asm("sti");
		printf("Waiting for %^78%^ ticks to see if %^IRQ's%^ and %^PIT%^ are setup...\n",0x02,0x0F,0x03,0x0F,0x04,0x0F);
		#ifndef OPT_NO_PROGRESS_BARS
		tm_putch_at('[',0,255);tm_putch_at(']',80-1,255);
		#endif
		int i=0;
		int count_inc=0;
		#ifndef OPT_NO_PROGRESS_BARS
		cursor_x=1;
		#endif
		pit_has_ticked();// Resets counter basicaly.
		while(i<78)
		{
			count_inc=pit_has_ticked();
			if(count_inc)
			{
				i+=count_inc;
				#ifndef OPT_NO_PROGRESS_BARS
				if(cursor_x!=1)
					tm_putch_at('=',cursor_x-1,255);
				if(cursor_x!=80)
					tm_putch_at('>',cursor_x,255);
				cursor_x+=count_inc;
				#endif
			}
			move_cursor();
		}
		#ifndef OPT_NO_PROGRESS_BARS
		cursor_x=0;
		cursor_y++;
		#endif
		log(" OK ",0x02,"Installed PIT\n");
		log("PASS",0x02,"Verified IRQ's\n");
	}
	kb_install();
	log(" OK ",0x02,"Installed Keyboard\n");
	printf("System initialised.\n");
	printf("Running VT Tests\n");
	tty_create(0); //Create VT
	tty_print(0, "Modal Dialog\n");
	tty_print(0, "----------------------------------------");
	tty_print(0, "So? This is cool huh. I thought so.\n");
	tty_print(0, "By using virtual terminals, I can make modal dialogs and virtual terminals, of course. They even wrap. ");
	tty_print(0, "However, internal colors, scrolling, clearing, and borders don't work. This gets annoying quickly. Just look down, that's a bug.");
	tty_setscrnpos(0, 20, 5);
	tty_setdim(0, 40, 9);
	tm_setAttribute(0x1F);
	//tty_render(0);
	tty_create(1); //Create VT
	tty_print(1, "BUT!\n");
	tty_print(1, "----------------------------------------");
	tty_print(1, "They are good for multitasking\n");
	tty_print(1, "I plan on having vt0 being kernel only\n");
	tty_setscrnpos(1, 20, 14);
	tty_setdim(1, 40, 5);
	tm_setAttribute(0x2F);
	//tty_render(1);
	tm_setAttribute(0x0F);
	while(true)
	{
	}
	halt("Reached the end of its execution");
	return 0;
}
