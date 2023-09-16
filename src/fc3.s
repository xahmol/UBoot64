;
; Startup code for cc65 (C64 version)
;

	;.export		_exit
    .export     __STARTUP__ : absolute = 1      ; Mark as startup
	.import		initlib, donelib, callirq
    .import	    zerobss
	.import	    callmain, pushax
    .import     RESTOR, BSOUT, CLRCH
	.import	    __INTERRUPTOR_COUNT__
	.import		__RAM_START__, __RAM_SIZE__	; Linker generated

	.import		_cgetc, _puts, _memcpy, _bankrun

	.import		__LOWCODE_LOAD__, __LOWCODE_RUN__, __LOWCODE_SIZE__
	.import		__CODE_LOAD__, __CODE_RUN__, __CODE_SIZE__
	.import		__RODATA_LOAD__, __RODATA_RUN__, __RODATA_SIZE__
	.import		__DATA_LOAD__, __DATA_RUN__, __DATA_SIZE__
	.import		__BANKACCESS_LOAD__, __BANKACCESS_RUN__, __BANKACCESS_SIZE__
	.import		__LOGO_LOAD__, __LOGO_RUN__, __LOGO_SIZE__


    .include        "zeropage.inc"
	.include     	"c64.inc"

	SPRITE_POINT	= $07F8

.segment	"HEADERDATA"

HeaderB:
@magic:
	;.byt	"C64  CARTRIDGE  "
	.byt	$43,$36,$34,$20, $43,$41,$52,$54
	.byt	$52,$49,$44,$47, $45,$20,$20,$20
@headelen:
	; 40 bytes header length
	.byt	$00,$00,$00,$40
@ver:
	.byt	$01,$00
@carttye:
	; Cartridge type 3 - Final Cartridge 3
	.byt	$00,$03
@EXROM:
	.byt	$01
@GAME:
	.byt	$01
@cartsubtype:
	; $01 for FC3 subtype not using df00-df1f, needed to see the Ultimate Command Interface
	.byt	$01
@reserved1:
	.byt	$00,$00,$00,$00,$00
@Name:	;You put the name of the cartridge here.
	.byt 	"uboot64",$00,$00,$00,$00,$00,$00,$00,$00,$00
	.byt    $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00

.segment	"CHIP0"
ChipB0:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$40,$10	;Use for 16k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0000
@start:
	.byt	$80,$00
@size2:
	.byt	$40,$00

.segment	"CHIP1"
ChipB1:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$40,$10	;Use for 16k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0100
@start:
	.byt	$80,$00
@size2:
	.byt	$40,$00

.segment	"CHIP2"
ChipB2:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$40,$10	;Use for 16k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0200
@start:
	.byt	$80,$00
@size2:
	.byt	$40,$00

.segment	"CHIP3"
ChipB3:
@magic:
	.byt	$43,$48,$49,$50 ;"CHIP"
@size:
	.byt	$00,$00,$40,$10	;Use for 16k carridge
@chiptype:	;ROM
	.byt	$00,$00
@bank:
	.word	$0300
@start:
	.byt	$80,$00
@size2:
	.byt	$40,$00


.segment	"STARTUP"
	.word	startup		;Cold start
	.word	$FE5E		;Warm start, default calls NMI exit.
	.byt	$C3,$C2,$CD,$38,$30 ;magic to identify cartridge

startup:
	sei									; Disable interrrupts
	ldx #$FF				
	txs
	ldx #$05
	sta $D016							; Turn on VIC for PAL / NTSC check
	jsr $FDA3							; Init I/O
	jsr	$FF84							; Prepare IRQ
	
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

	jsr $FD15							; Init I/O
	jsr $FF5B							; Init video
	cli									; Restore interrupts
	
; Init screen
; Switch to second charset
	lda	#14
	jsr	BSOUT

; Set colors
	lda #$00							; Load value for black
	sta VIC_BG_COLOR0					; Store as background color
	lda #$07							; Load value for yellow
	sta CHARCOLOR						; Store as foreground color

; Copy sprite
	lda	#<__LOGO_LOAD__
	sta	ptr1
	lda	#>__LOGO_LOAD__
	sta	ptr1+1

	lda	#<__LOGO_RUN__
	sta	ptr2
	lda	#>__LOGO_RUN__
	sta	ptr2+1

	lda	#<__LOGO_SIZE__
	sta	ptr3
	lda	#>__LOGO_SIZE__
	sta	ptr3+1
	jsr	copym

; Enable sprite
	lda #$0D							; Load block 13 to point at sprite in cassette buffer
	sta SPRITE_POINT+2					; Save pointer for sprite 2
	lda #$04							; Sprite 2
	sta VIC_SPR_ENA						; Enable with VIC sprite enable register
	sta VIC_SPR_HI_X					; Set Most Significant Bit for sprite 2
	lda #$30							; Load X coord
	sta VIC_SPR2_X      				; Store X coord
	lda #$32							; Load Y coord
	sta VIC_SPR2_Y      				; Store Y coord
	lda #$03							; Set color as yellow
	sta VIC_SPR2_COLOR					; Set color in VIC register

; Print start message
	ldx #$00
msg_loop1:       
	lda startmessage,x
    beq msg_next1
    jsr BSOUT
    inx
    bne msg_loop1

msg_next1:
; Call module constructors
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

; Set border to black
	lda #$00							; Load value for black
	sta VIC_BORDERCOLOR					; Store as background color

; Clear the BSS data
	jsr	zerobss
; Buold C environment
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
	inc	VIC_BORDERCOLOR 				; Border color flashing effect
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

startmessage:
	.byt "Starting UBoot64.",$0D,$0D
	.byt "Copying core to RAM.",$0D,$00

.segment	"LOGO"
	.byte $00,$0F,$80,$00,$10,$40,$00,$10,$40,$00,$13,$80,$00,$12,$00,$00
	.byte $3F,$00,$40,$C0,$C0,$A3,$00,$30,$94,$00,$18,$9A,$BA,$94,$92,$A2
	.byte $A2,$52,$BB,$A2,$22,$A8,$A1,$23,$B8,$A1,$5C,$00,$22,$93,$00,$22
	.byte $88,$C0,$D4,$94,$3F,$18,$93,$00,$30,$A0,$C0,$C0,$40,$3F,$00,$07

.segment	"LOWCODE"
.segment	"CODE3"
.segment	"RODATA3"
.segment	"CODE4"
.segment	"RODATA4"
