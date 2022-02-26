	.setcpu	6800
	.zp
tone:
	defs	2
duration:
	defs	2
ptr:
	defs	2

	.code
	sei
; clear screen
	clra
	ldx	#$100
cls_loop:
	staa	,x
	inx
	cpx	#$400
	bne	cls_loop
; print message
	sts	ptr
	lds	#msg_1
	ldx	#$100-1
str_loop:
	inx
	pula
	staa	,x
	bne	str_loop
	lds	ptr
;	cli
entire_loop:
	ldx	#_data
	stx	ptr
element_loop:
	ldx	ptr
	ldaa	,x
	oraa	1,x
	beq	rest
	ldaa	,x
	staa	tone
	ldaa	1,x
	staa	tone+1
	ldaa	2,x
	staa	duration
	ldaa	3,x
	staa	duration+1
	inx
	inx
	inx
	inx
	stx	ptr
note_loop:
	ldaa	#$3e
	staa	$ee80
	ldx	tone
loop1:
	dex
	bne	loop1
	clra
	staa	$ee80
	ldx	tone
loop2:
	dex
	bne	loop2
	ldx	duration
	dex
	stx	duration
	bne	note_loop
	bra	element_loop
rest:
	oraa	2,x
	oraa	3,x
	beq	entire_loop
	ldaa	2,x
	staa	duration
	ldaa	3,x
	staa	duration+1
	inx
	inx
	inx
	inx
	stx	ptr
	ldx	duration
	clra
rest_loop:
	nop
	deca
	bne	rest_loop
	dex
	bne	rest_loop
	bra	element_loop

	.data
msg_1:
	defs	1
msg:
	.ascii	'SOUND TEST (MONO) Set clock 750'
	defb	0
