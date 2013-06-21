// Keyboard
#include <textmode.h>
#include <types.h>
#include <low.h>
#include <arch/x86.h>
unsigned char kbdus[256] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		
unsigned char kbdus_sft[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		
#define bool int
#define true 1
#define false 0
bool keystat_crtl, keystat_shift, keystat_alt;
bool keystat_numlock, keystat_capslock, keystat_scrolllock;

/**
Sends a command to the keyboard, and returns the output.
**/
uint8_t kb_sendCommand(uint8_t command)
{
	while(true)
		if ((inb(0x64) & 2) == 0) break;

}

void keyboard_handler(struct regs *r)
{
    unsigned char scancode;
    /* Read from the keyboard's data buffer */
    scancode = inb(0x60);
    if (scancode & 0x80)
    {
		if(scancode == 0x2a+0x80)
		keystat_shift = false;

		if(scancode == 0x36+0x80)
		keystat_shift = false;
    }
	else
	{

		if(scancode == 0x2a)
			keystat_shift = true;		
		if(scancode == 0x36)
			keystat_shift = true;		
		if(scancode == 69)
		{
		keystat_numlock = true;
		}
		if(scancode == 58)
		{
		if(keystat_capslock)
		keystat_capslock = false;

		else if(!keystat_capslock)
		keystat_capslock = true;
		}


		if(keystat_shift)
    		tm_putch(kbdus_sft[scancode]);
    	else
    		tm_putch(kbdus[scancode]);
	}
}

void kb_install()
{
	irq_install_handler(1, keyboard_handler);
	inb(0x60);
}