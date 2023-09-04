
#ifndef MAGICDESK_H_
#define MAGICDESK_H_

//Main routine table.  Define in your code as the locations of your main
//parts, where each entry determines a module in your program.
//Put in RODATA segment.
extern const struct codetable {
	void*		calladdr;	//Address of routine in cartridge bank.
	unsigned char	bank, 		//Bank containing routine.
			x;		//Padding--no meaning.
} codetable [];

//Transfer control to a different module in your code.
void __fastcall__ bankrun (char p);	//p is the ID of the module, as in the codetable
					//subscript of codetable[].
void __fastcall__ bankout();

#endif

