	.text
	.global play

play:
	PUSH R4
	PUSH R5

	MOV #20, R4

loop:
	MOV #200, R5
other_loop:
	XOR #0x02, &P2OUT
	NOP
	NOP
	NOP
	NOP
	DEC R5
	JNZ other_loop

	DEC R4
	JNZ loop
	
	BIC #0x02, &P2OUT
	POP R5
	POP R4
	RET
