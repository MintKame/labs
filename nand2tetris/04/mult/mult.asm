// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.

//R0>=0
@R0
D=M
@NEG
D;JLT

(START)
//R2=0
@R2
M=0

//for(i=0;i<R0;i++)
@i
M=0

(LOOP)
@R0
D=M
@i
D=D-M
@END
D;JLE

@i
M=M+1

//R2+=R1      
@R1
D=M
@R2
M=M+D

@LOOP
0;JMP

(END)
@END
0;JMP

(NEG)
@R0
M=-M
@R1
M=-M
@START
0;JMP