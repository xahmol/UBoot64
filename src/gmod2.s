;
; Startup code for cc65 (C64 version)
;

	;.export		_exit
        .export         __STARTUP__ : absolute = 1      ; Mark as startup
	.import		initlib, donelib, callirq
       	.import	       	zerobss
	.import	     	callmain, pushax
        .import         RESTOR, BSOUT, CLRCH
	.import	       	__INTERRUPTOR_COUNT__
	.import		__RAM_START__, __RAM_SIZE__	; Linker generated

	.import		_cgetc, _puts, _memcpy, _bankrun

	.import		__LOWCODE_LOAD__, __LOWCODE_RUN__, __LOWCODE_SIZE__
	.import		__CODE_LOAD__, __CODE_RUN__, __CODE_SIZE__
	.import		__RODATA_LOAD__, __RODATA_RUN__, __RODATA_SIZE__
	.import		__DATA_LOAD__, __DATA_RUN__, __DATA_SIZE__
	.import		__BANKACCESS_LOAD__, __BANKACCESS_RUN__, __BANKACCESS_SIZE__

        .include        "zeropage.inc"
	.include     	"c64.inc"


.segment	"HEADERDATA"

HeaderB:
@magic:
	;.byt	"C64  CARTRIDGE  "
	.byt	$43,$36,$34,$20, $43,$41,$52,$54
	.byt	$52,$49,$44,$47, $45,$20,$20,$20
@headelen:
	.byt	$00,$00,$00,$40
@ver:
	.byt	$01,$00
@carttye:
	.byt	$00,$3C
@EXROM:
	.byt	$00
@GAME:
	.byt	$01
@reserved1:
	.byt	$00,$00,$00,$00,$00,$00
@Name:	;You put the name of the cartridge here.
	.byt 	"uboot64",$00,$00,$00,$00,$00,$00,$00,$00,$00
	.byt    $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00

.segment	"CHIP0"
ChipB0:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$20,$10	;Use for 8k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0000
@start:
	.byt	$80,$00
@size2:
	.byt	$20,$00

.segment	"CHIP1"
ChipB1:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$20,$10	;Use for 8k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0100
@start:
	.byt	$80,$00
@size2:
	.byt	$20,$00

.segment	"CHIP2"
ChipB2:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$20,$10	;Use for 8k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0200
@start:
	.byt	$80,$00
@size2:
	.byt	$20,$00

.segment	"CHIP3"
ChipB3:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$20,$10	;Use for 8k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0300
@start:
	.byt	$80,$00
@size2:
	.byt	$20,$00


.segment	"STARTUP"
	.word	startup		;Cold start
	.word	$FEBC		;Warm start, default calls NMI exit.
	.byt	$C3,$C2,$CD,$38,$30 ;magic to identify cartridge

startup:
	jsr	$FF84		;Init. I/O
	jsr	$FF87		;Init. RAM
	jsr	$FF8A		;Restore vectors
	jsr	$FF81		;Init. screen
	
; Switch to second charset

	lda	#14
	jsr	BSOUT

; Clear the BSS data

; If we have IRQ functions, chain our stub into the IRQ vector

;        lda     #<__INTERRUPTOR_COUNT__
;      	beq	NoIRQ1
;      	lda	IRQVec
;       	ldx	IRQVec+1
;      	sta	IRQInd+1
;      	stx	IRQInd+2
;      	lda	#<IRQStub
;      	ldx	#>IRQStub
;      	sei
;      	sta	IRQVec
;      	stx	IRQVec+1
;      	cli

; Call module constructors

	
NoIRQ1:
	lda	#<__DATA_LOAD__
	sta	ptr1
	lda	#>__DATA_LOAD__
	sta	ptr1+1

	lda	#<__DATA_RUN__
	sta	ptr2
	lda	#>__DATA_RUN__
	sta	ptr2+1

	lda	#<__DATA_SIZE__
	sta	ptr3
	lda	#>__DATA_SIZE__
	sta	ptr3+1
	jsr	copym

	lda	#<__CODE_LOAD__
	sta	ptr1
	lda	#>__CODE_LOAD__
	sta	ptr1+1

	lda	#<__CODE_RUN__
	sta	ptr2
	lda	#>__CODE_RUN__
	sta	ptr2+1

	lda	#<__CODE_SIZE__
	sta	ptr3
	lda	#>__CODE_SIZE__
	sta	ptr3+1
	jsr	copym


	lda	#<__LOWCODE_LOAD__
	sta	ptr1
	lda	#>__LOWCODE_LOAD__
	sta	ptr1+1

	lda	#<__LOWCODE_RUN__
	sta	ptr2
	lda	#>__LOWCODE_RUN__
	sta	ptr2+1

	lda	#<__LOWCODE_SIZE__
	sta	ptr3
	lda	#>__LOWCODE_SIZE__
	sta	ptr3+1
	jsr	copym


	lda	#<__RODATA_LOAD__
	sta	ptr1
	lda	#>__RODATA_LOAD__
	sta	ptr1+1

	lda	#<__RODATA_RUN__
	sta	ptr2
	lda	#>__RODATA_RUN__
	sta	ptr2+1

	lda	#<__RODATA_SIZE__
	sta	ptr3
	lda	#>__RODATA_SIZE__
	sta	ptr3+1
	jsr	copym


	lda	#<__BANKACCESS_LOAD__
	sta	ptr1
	lda	#>__BANKACCESS_LOAD__
	sta	ptr1+1

	lda	#<__BANKACCESS_RUN__
	sta	ptr2
	lda	#>__BANKACCESS_RUN__
	sta	ptr2+1

	lda	#<__BANKACCESS_SIZE__
	sta	ptr3
	lda	#>__BANKACCESS_SIZE__
	sta	ptr3+1
	jsr	copym

	lda    	#$00;(__RAM_START__ + __RAM_SIZE__)
	sta	sp
	lda	#$80;>(__RAM_START__ + __RAM_SIZE__)
       	sta	sp+1   		; Set argument stack ptr

	jsr	zerobss
	jsr	initlib


start2:

	lda	#0
	jmp	_bankrun


copym:
	lda	ptr3
	ora	ptr3+1
	bne	*+3
	rts
	ldy	#0
@lp:
	lda	(ptr1),y
	sta	(ptr2),y
	inc	$D020
	inc	ptr1
	bne	*+4
	inc	ptr1+1
	inc	ptr2
	bne	*+4
	inc	ptr2+1
	lda	ptr3
	bne	*+4
	dec	ptr3+1
	dec	ptr3
	lda	ptr3
	ora	ptr3+1
	bne	@lp
	rts
	
;m:
;	.byt	"Hello, world!\n",0
.segment	"LOWCODE"
.segment	"CODE4"
.segment	"RODATA4"
