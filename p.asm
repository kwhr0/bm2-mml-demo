; 10cc vvvv nnnn nnn0	note
; 11cc ---- eeee eeee	envelope
; 0www wwww wwww wwww	wait

	.setcpu	6800
DELTA	equ	0
VOLUME	equ	2
ENV	equ	3
ACC	equ	4
ENVP	equ	6
	.zp
delta1:
	defs	2
volume1:
	defs	1
env1:
	defs	1
acc1:
	defs	2
envp1:
	defs	2
delta2:
	defs	2
volume2:
	defs	1
env2:
	defs	1
acc2:
	defs	2
envp2:
	defs	2
delta3:
	defs	2
volume3:
	defs	1
env3:
	defs	1
acc3:
	defs	2
envp3:
	defs	2
deltan:
	defs	2
volumen:
	defs	1
envn:
	defs	1
accn:
	defs	2
envpn:
	defs	2
duration:
	defs	2
ptr:
	defs	2
tmp:
	defs	2
tmp2:
	defs	2
noise:
	defw	1

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
insn_loop:
	ldx	ptr
	ldab	,x
	bitb	#$80
	beq	generate
	clra
	lsrb
	andb	#$18
	addb	#<delta1
	adca	#>delta1
	staa	tmp	; base address
	stab	tmp+1
	ldaa	,x
	bita	#$40
	bne	envp
	asla
	asla
	asla
	asla
	ldx	tmp
	staa	VOLUME,x
	ldx	ptr
	clra
	ldab	1,x
	addb	#<_tonetable
	adca	#>_tonetable
	staa	tmp2
	stab	tmp2+1
	inx
	inx
	stx	ptr
	ldx	tmp2
	ldaa	,x
	ldab	1,x
	ldx	tmp
	staa	DELTA,x
	stab	DELTA+1,x
	ldaa	ENVP,x
	staa	ENV,x
	bra	insn_loop
envp:
	ldaa	1,x
	inx
	inx
	stx	ptr
	ldx	tmp
	staa	ENVP,x
	bra	insn_loop
generate:
	stab	duration
	ldab	1,x
	stab	duration+1
	inx
	inx
	stx	ptr
	ldaa	duration
	oraa	duration+1
	beq	entire_loop	; end of data
smpl_loop:
	tst	volumen
	beq	tone
	ldaa	accn
	adda	deltan
	staa	accn
	bcc	labelna
	ldaa	noise
	asla
	rola
	staa	tmp
	ldaa	noise
	lsra
	lsra
	lsra
	eora	tmp
	staa	tmp
	ldaa	noise+1
	lsra
	lsra
	eora	tmp
	eora	noise+1
	anda	#1
	staa	tmp
	ldaa	noise
	ldab	noise+1
	aslb
	rola
	orab	tmp
	staa	noise
	stab	noise+1
labelna:
	tst	envn
	beq	labelnc
	dec	envn
	bne	labelnc
	ldaa	envpn
	staa	envn
	tst	volumen
	beq	labelnc
	bmi	labelnb
	dec	volumen
	jmp	sum
labelnb:
	inc	volumen
labelnc:
	jmp	sum
tone:
	ldaa	acc1+1
	adda	delta1+1
	staa	acc1+1
	ldaa	acc1
	adca	delta1
	staa	acc1
	bcc	label1a
	neg	volume1
label1a:
	tst	env1
	beq	label1c
	dec	env1
	bne	label1c
	ldaa	envp1
	staa	env1
	tst	volume1
	beq	label1c
	bmi	label1b
	dec	volume1
	bra	label1c
label1b:
	inc	volume1
label1c:
	ldaa	acc2+1
	adda	delta2+1
	staa	acc2+1
	ldaa	acc2
	adca	delta2
	staa	acc2
	bcc	label2a
	neg	volume2
label2a:
	tst	env2
	beq	label2c
	dec	env2
	bne	label2c
	ldaa	envp2
	staa	env2
	tst	volume2
	beq	label2c
	bmi	label2b
	dec	volume2
	bra	label2c
label2b:
	inc	volume2
label2c:
	ldaa	acc3+1
	adda	delta3+1
	staa	acc3+1
	ldaa	acc3
	adca	delta3
	staa	acc3
	bcc	label3a
	neg	volume3
label3a:
	tst	env3
	beq	label3c
	dec	env3
	bne	label3c
	ldaa	envp3
	staa	env3
	tst	volume3
	beq	label3c
	bmi	label3b
	dec	volume3
	bra	label3c
label3b:
	inc	volume3
label3c:
sum:
	ldaa	volumen
	asra
	asra
	asra
	ldab	noise
	bitb	#1
	bne	label8
	nega
label8:
	ldab	volume1
	asrb
	asrb
	asrb
	aba
	ldab	volume2
	asrb
	asrb
	asrb
	aba
	ldab	volume3
	asrb
	asrb
	asrb
	aba
; clip
	cmpa	#$1f
	ble	label6
	ldaa	#$1f
label6:
	cmpa	#$e0
	bge	label7
	ldaa	#$e0
label7:
; output
	adda	#$20
	anda	#$3e
	staa	$ee80
; count
	ldx	duration
	dex
	stx	duration
	beq	label9
	jmp	smpl_loop
label9:
	jmp	insn_loop
	.data
msg_1:
	defs	1
msg:
	.ascii	'SOUND TEST (POLY) Set clock 3000'
	defb	0
