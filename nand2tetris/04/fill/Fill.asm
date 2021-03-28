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

//add=screen
@8192
D=A
@last
M=D

(BEGIN)
//if(kbd==0) jump to WHITE
@KBD
D=M
@WHITE
D;JEQ
//else jmp to BLACK
@BLACK
0;JMP
/////////////////////////////////////////////
(BLACK)
//for(i=0;i<last;i++)  R[*add]=-1;(*add)++
@i
M=0
@SCREEN
D=A
@add
M=D

(BLOOP)
@i
D=M
@last
D=D-M
@BEGIN
D;JGE

@add
A=M
M=-1

@add
M=M+1

@i
M=M+1

@BLOOP
0;JMP
/////////////////////////////////////////////
(WHITE)
//for(i=0;i<last;i++)  R[*add]=0;(*add)++
@i
M=0
@SCREEN
D=A
@add
M=D

(WLOOP)
@i
D=M
@last
D=D-M
@BEGIN
D;JGE

@add
A=M
M=0

@add
M=M+1

@i
M=M+1

@WLOOP
0;JMP