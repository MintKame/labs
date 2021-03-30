// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.


//because the A-instrution's value can only be 15-bits,
// can't write 65535(16-bits)  directly, but we can write -1

//last=8K bit 
@8192 
D=A
@last
M=D

(BEGIN)
//if (kbd==0) color = WHITE
//else color = BLACK
@KBD
D=M
@WHITE
D;JEQ

//set color
@color
M=-1
@FILL
0;JMP

(WHITE)
@color
M=0
@FILL
0;JMP

(FILL)
//for(i=0;i<last;i++)  M[addr]=color;(addr)++
@i
M=0
@SCREEN
D=A
@addr
M=D

(LOOP)
@i
D=M
@last
D=D-M
@BEGIN
D;JGE

@color
D=M
@addr
A=M
M=D

@addr
M=M+1

@i
M=M+1

@LOOP
0;JMP