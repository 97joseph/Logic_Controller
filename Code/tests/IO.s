	addi $s0, $zero, 0
	addi $s1, $zero, 10000000
LOOP:	beq $s0, $s1, END
	addi $s0, $s0, 2
	j LOOP
END:    
	