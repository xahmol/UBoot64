;------------------------------------------------------------------------------
;
; Bank-swiching routines for the Magic Desk artridge under cc65
;
;------------------------------------------------------------------------------

.export		_bankrun, _bankout
.import		_codetable, _exit, donelib

.importzp	tmp1, ptr1, ptr2, regbank, sp

.include    "zeropage.inc"
.include    "c64.inc"
.include	"cbm_kernal.inc"

.segment	"BANKACCESS"

BANKREG		 = $DFFF
KBDBUFFER	 = $0277 

_bankout:

	sei			; Stop interrupts

	; Bank out cartridge
	lda #$70	; Bitmask to switch cartridge ROM out
    sta BANKREG	; Set banking register to bank out cart

	; Clean CC65 environment
	jsr donelib	; Deconstruct C libs

	; Modified kernal reset
	ldx #$FF	; Load stack value	
	txs			; Move to stack
	ldx #$05	; Load value for VIC init
	stx $D016	; VIC init NTSC / PAL check
	jsr $FDB3	; Init CIA
	jsr	$FF84	; Prepare IRQ
	
	; Init memory
	lda #$00
    tay
init_loop1:
	; Wipe first 3 pages
	sta $0002,Y
    sta $0200,Y
    sta $0300,Y
    iny
    bne init_loop1

	; Set Start of Tape Buffer pointer
    ldx #$3c
    ldy #$03
    stx $B2
    sty $B3

	; Set IO Start address and OS end of memory pointer
    ldx #$00
    ldy #$A0
    stx $C1
    stx $0283
    sty $C2
    sty $0284

	; Set OS Start of memory and screen memory
    lda #$08
    sta $0282
    lda #$04
    sta $0288

	; Continue modified kernal reset
	jsr $FD15	; Init I/O
	jsr $FF5B	; Init video
	lda #$00	; Clear start of BASIC area - load 0
	sta $0800	; Clear first byte
	sta $0801	; Clear second byte
	sta $0802	; Clear third byte

	cli			; Restore interrupts

	; Modified BASIC cold start
	jsr $E453	; Initialise BASIC vectors
	jsr $E3BF	; Set BASIC vectors
	jsr $E422	; Print start message and init memory pointers
	ldx #$FB
	txs

;	; Print commands to execute
	sec
    jsr PLOT
    txa
    pha
    tya
    pha
    ldx #$00
exec_loop1:       
	lda execute_commands,x
    beq exec_next1
    jsr CHROUT
    inx
    bne exec_loop1

exec_next1:
	ldx #$00
exec_loop2:        
	lda execute_keys,x
    beq exec_next2
    sta KBDBUFFER,x
    inc KEY_COUNT
    inx
    bne exec_loop2
exec_next2:
    pla
    tay
    pla
    tax
    clc
    jsr PLOT

	; Print adapted READY prompt.
	lda #<bootmsg
    ldy #>bootmsg
    jmp $a478       ; jump into BASIC

_bankrun:
; Routine to jump to specified bank and routine address
; Input is entry number in the code table
	asl
	asl
	tax
	lda	_codetable,x
	sta	ptr1
	lda	_codetable+1,x
	sta	ptr1+1
	lda	_codetable+2,x
	ora #$40
	sta	BANKREG
	lda	#$00
	sta	sp
	lda	#$D0
	sta	sp+1
	ldx	#$00
	txs
	jmp	(ptr1)

execute_commands:
; Buffer for boot execute commands: 200 bytes, zero terminated
	.byte $00
	.res 199

execute_keys:
; Buffer for boot execute enter keys: 10 bytes, zero terminated
	.byte $00
	.res 9

bootmsg:
; Boot message
	.byte $0d,"uboot64.",$0D,$00

	