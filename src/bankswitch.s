;------------------------------------------------------------------------------
;
; Bank-swiching routines for the Magic Desk artridge under cc65
;
;------------------------------------------------------------------------------

.export		_bankrun, _bankout
.import		_codetable, _exit

.importzp	tmp1, ptr1, ptr2, regbank, sp
.segment	"BANKACCESS"

_bankout:
	lda #$36
    sta $01
	rts

_bankrun:
	asl
	asl
	tax
	lda	_codetable,x
	sta	ptr1
	lda	_codetable+1,x
	sta	ptr1+1
	lda	_codetable+2,x
	sta	$DE00
	lda	#$00
	sta	sp
	lda	#$D0
	sta	sp+1
	ldx	#$00
	txs
	jmp	(ptr1)

	