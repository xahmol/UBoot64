/**************************************************************************
 *
 * Individual C header created by TempC Small for DOS.
 *
 * The end-user may use this code as he/she likes under the following
 * conditions:
 *
 * 1. TempC, along with its author, is mentioned in the program's 
 *    documentation, and
 * 2. If this source code is published, this header must be kept
 *    intact.
 *
 **************************************************************************/

#ifndef GMOD2_H_
#define GMOD2_H_

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
#endif

