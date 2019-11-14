        lst off
*
*  main.s
*  Merlin32 Test
*
*  Created by Lane Roathe on 8/26/19.
*  Copyright © 2019 Ideas From the Deep. All rights reserved.
*
//]XCODESTART     ; Keep this at the start and put your code after this

    	xc	off
        xc
        xc
        mx	%00

*==========================================================
* monitor addresses


TEXT	=	$FB39		;Reset text window
TABV	=	$FB5B		;Complete vtab, using contents of 'A'
MONBELL	=	$FBE4		;random bell noise!
HOME    =	$FC58		;Clear text window
WAIT	=	$FCA8		;delay routine
CROUT	=	$FD8E		;Print a CR
PRBYTE	=	$FDDA		;Print 'A' as a hex number
PRHEX	=	$FDE3		;as above, but bits 0-3 only
COUT	=	$FDED		;Monitor char out
MOVE	=	$FE2C		;memory move routine
INVERSE	=	$FE80		;Print in inverse
NORMAL	=	$FE84		;Normal print

* Jump Vectors
CONNECT	=	$3EA		;Connect DOS
DOSWARM	=	$3D0		;exit to DOS prompt
RSTVEC	=	$3F2		;reset vector

TSTADDR	=	$1000		;absolute address for testing

*==========================================================
* Data Index DUM section test

        lst
		DUM	0
dum0	ds	1			;fractional byte
dum1	ds	1
dumSize	=	*
		DEND
        ;lst off

*==========================================================
* zero page (all zp var names are prefixed with _)

		DUM	0

_ptr	ds	2
_tmp	ds	2

_num1	ds	dumSize		;first and second operand values

; test ORG with DUM section

		ORG	$20

_LFT	ds	1			;Window edge   0..39

		DEND

*==========================================================
* Program Entry

;Issue #26 - This should start at the ORG in the linkscript, not at the last ORG in the DUM sections.
START

; PUT current issue here, so it's the first thing assembled.
; The rest below are unit tests to make sure future changes don't break existing code!


; START OF TESTS KNOWN TO HAVE PASSED IN PREVIOUS BUILDS

; --- Test all instructions in all their modes, with as many variants as possible ---

;adc (ZP,x)
	    adc (0,x)

        adc ($80,x)
        adc	(_tmp,x)
        adc	(_tmp+0,x)
        adc	(_tmp+$10,x)
        adc	($10+_tmp,x)
        adc	(_tmp+dum0,x)
        adc	(_tmp+dum1,x)
        adc	(_tmp+dum1+1,x)
        adc	(_tmp+dum0+dum1,x)

        adc 0
        adc $80
		adc _tmp
        adc #0
        adc #$1111
        adc $1111

; --- Other tests that have proven helpful ---

; Tests regarding issues with math and zp,x
        sta TSTADDR+dum0
        sta TSTADDR+_num1+dum0
        sta TSTADDR+_num1+dum0,x

        lda _num1+dum0
        adc _num1+dum1
        sbc _num1+dum1
        bit _num1+dum0
        sta _num1+dum0    ;(FIXED): can't use sta _num1+dum0
		stz _num1+dum0

		lda _num1+dum0,x
		adc _num1+dum0,x
		sbc _num1+dum0,x
		bit _num1+dum0,x
		sta _num1+dum0,x
		stz _num1+dum0,x

        lda _num1+dum0,y	;these assemble to abs accesses: lda $00C0,y
		adc _num1+dum0,y
		sbc _num1+dum0,y
		sta _num1+dum0,y

; Label & branching tests
GetKey	ldx  $C000
		bpl  GetKey
]loop
        dex
        bne ]loop

		tya
        and #1
        beq :err

        tya
        and #1
        bne	:good
:err
        lda #0
:good
		bne myQuit
		nop
        hex 2C		;bit
        lda #1
myQuit
		jmp DOSWARM

; --- Tests used when addressing issues opened against Merlin32 ---

;Issue #26 (lroathe) - ORG in DUM section is ignored, but can't mess up code ORG

		org $2000

        lda _LFT
        ldx #_LFT
        cpx #$20

        org	;return to ongoing address
        
        lst 
        lda $FF
;Issue #16 (fadden) - Byte reference modifiers are ignored (no way to force DP)
        lda	<$fff0			;zp
        lda	>$fff0			;ABS (lo word)
        lda ^$fff0			;ABS (hi word)
        lda |$fff0			;ABS (long in 65816 mode)

        lda $08
        lda $0008
        lda $FFFF-$FFF7
        lda $FFF0+24
        ldaz $FFF0+24       ; forced DP

        
        ldaz $FFFF          ; forced DP
        lda: $FFFF          ; forced ABS (any char but 'L', 'D', and 'Z"
        ldal $FFFF          ; forced long abs (3 byte address)

        ldaz $05
        lda: $05
        ldal $05


        lda	<$fff0+24       ;zp
        lda	>$fff0+24       ;ABS (lo word)
        lda ^$fff0+24		;ABS (hi word)
        lda |$fff0+24		;ABS (long in 65816 mode)

        mx %11

        lda	#<$fff0+24		;byte
        lda	#>$fff0+24		;page
        lda #^$fff0+24		;bank

        lda	#<$1234			;byte
        lda	#>$1234			;page
        lda #^$1234			;bank
        lda #^$A51234         ;bank


        mx %00

        lda #<$fff0+24      ;byte
        lda #>$fff0+24      ;page
        lda #^$fff0+24      ;bank

        lda #<$1234         ;byte
        lda #>$1234         ;page
        lda #^$1234         ;bank
        lda #^$A51234         ;bank


        mx MX

        lda	$0008           ;ZP
        lda	$08             ;ZP
        lda	$ffff-$fff7     ;ZP
        lda	$fff0+24        ;ABS (long in 65816 mode)


;Issue #8 fadden) - STX zp,y fails to assemble
        org	$bc

L00BC   bit	L00BC

        org

        stx	$bc,y

        ldx	L00BC,y
        stx	L00BC,y


* Data Storage Tests

     	hex 11,22,33,44,55,66,77,88,99
        hex 112233445566778899
        hex aabb,cc,ddee,ff


//]XCODEEND       ; Keep this at the end and put your code above this
    lst off

