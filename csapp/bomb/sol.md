<!--
 * @Author: your name
 * @Date: 2021-04-12 23:10:43
 * @LastEditTime: 2021-04-14 23:43:36
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /bomb/sol.md
-->
# 答案：
Border relations with Canada have never been better. 
1 2 4 8 16 32
1 311
7 0
9?>567

# 思路：
+ 用strings和objdump分别获得了printable_string和symbol_table
+ 用objdump disassemble 存于bomb.s
+ 查看main 函数，分别调用6个phase，参数为输入的string，存于%rdi

1. 
+ strings_not_equal(a1, a2): 
  比较存储在a1,a1的str，相同则返回0（都空为相同）

+ phase：
  调用strings_not_equal(a1,a2) 函数，需要返回0
  a1为输入的句子，a2为 0x402400处存的句子
  通过指令print (char *) 得：
  Border relations with Canada have never been better.

2. 
+ phase： 
  调用read_six_numbers，s为输入的字符串，p为stack的top
  stack所存的数分别对比，希望输入的数: 第一个为1，之后的数为前一个的2倍
  1 2 4 8 16 32

+ read_six_numbers(s,p)：
  从s中读6个int，分别存在p + 0x0, 0x4, 0x8, 0xc, 0x10, 0x14

3. 
+ phase：
  输入两个int a b，分别存储在sp +8, c处 
  需0 <= a <= 7 (因为使用ja，unsign比较)
  间接jmp到 *(0x402470 + 8*a)
  并设置 %eax 为不同的值，需要 = b
  例如输入 a = 1， 跳到 *(0x402470 + 8*1) = 0x400fb9
  %eax被设置为 0x137 = 311
   
4. 
+ phase
  输入两个int a b，分别存储于 sp +8, +c
  需要 0 <= a <= 14 (因使用jbe，unsigned比较)
  调用func4(a, 0, 14)
  需要返回0 (a == 7 时满足)
  需要 b == 0 
  
+ func4(x, y, z)的伪代码:  
  cx = (((z - y) >> 31) + z - y) >> 1 + y 
  // >> 1st logi, 2nd arth
  if (cx > x){
    z = cx - 1
    ret = 2 * func4(x, y, z)
    return ret
  } 
  else if (cx < x){
      y = 1 + cx
      ret = func4(x, y, z)
      ret = 1 + 2 * ret
      return ret
  }
  else return 0

5.  
+ 伪代码
  输入句子长度=6
  rbx 存输入字符串的地址
  eax = 0 
  // 遍历输入的6个字符
  do{
    当前字符 存到stack
    M[当前字符的低4位 + 0x4024b0] 存到 M[0x10 + rsp + rax] 
    rax++
  }while(rax != 6)
  调用strings_not_equal 需要 M[0x10 + rsp] == M[0x40245e] (存储：102 108 121 101 114 115)

+ 解
  用x/20b 查看0x4024b0存的20个字节：
  0x4024b0  109     97      100     117     105     101     114     115
  0x4024b8  110     102     111     116     118     98      121     108
  0x4024c0  83      111     32      121
  相等的6个值分别存在： 0x4024b9 0x4024bf 0x4024be 0x4024b5 0x4024b6 0x4024b7
  输入字符串各字符低四位： 9 f e 5 6 7 
  可输入字符串：9?>567

6. 
+ phase： 
  调用函数读6个int，分别存在sp + 0x0, 0x4, 0x8, 0xc, 0x10, 0x14
  sp的值存在 r13，r14, rbp

   0x0000000000401114 <+32>:    mov    %r13,%rbp
   0x0000000000401117 <+35>:    mov    0x0(%r13),%eax
   0x000000000040111b <+39>:    sub    $0x1,%eax
   0x000000000040111e <+42>:    cmp    $0x5,%eax
   0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52>
   0x0000000000401123 <+47>:    callq  0x40143a <explode_bomb>

   0x0000000000401128 <+52>:    add    $0x1,%r12d
   0x000000000040112c <+56>:    cmp    $0x6,%r12d
   0x0000000000401130 <+60>:    je     0x401153 <phase_6+95>

   0x0000000000401132 <+62>:    mov    %r12d,%ebx
   0x0000000000401135 <+65>:    movslq %ebx,%rax
   0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax
   0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp)
   0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
   0x0000000000401140 <+76>:    callq  0x40143a <explode_bomb>
   0x0000000000401145 <+81>:    add    $0x1,%ebx
   0x0000000000401148 <+84>:    cmp    $0x5,%ebx
   0x000000000040114b <+87>:    jle    0x401135 <phase_6+65>
   0x000000000040114d <+89>:    add    $0x4,%r13
   0x0000000000401151 <+93>:    jmp    0x401114 <phase_6+32>
   



   0x0000000000401153 <+95>:    lea    0x18(%rsp),%rsi
   0x0000000000401158 <+100>:   mov    %r14,%rax
   0x000000000040115b <+103>:   mov    $0x7,%ecx
   0x0000000000401160 <+108>:   mov    %ecx,%edx
   0x0000000000401162 <+110>:   sub    (%rax),%edx
   0x0000000000401164 <+112>:   mov    %edx,(%rax)
   0x0000000000401166 <+114>:   add    $0x4,%rax
   0x000000000040116a <+118>:   cmp    %rsi,%rax
   0x000000000040116d <+121>:   jne    0x401160 <phase_6+108>
   0x000000000040116f <+123>:   mov    $0x0,%esi
   0x0000000000401174 <+128>:   jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx
   0x000000000040117a <+134>:   add    $0x1,%eax
   0x000000000040117d <+137>:   cmp    %ecx,%eax
   0x000000000040117f <+139>:   jne    0x401176 <phase_6+130>
   0x0000000000401181 <+141>:   jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:   mov    $0x6032d0,%edx
   0x0000000000401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2)
   0x000000000040118d <+153>:   add    $0x4,%rsi
   0x0000000000401191 <+157>:   cmp    $0x18,%rsi
   0x0000000000401195 <+161>:   je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:   mov    (%rsp,%rsi,1),%ecx
   0x000000000040119a <+166>:   cmp    $0x1,%ecx
   0x000000000040119d <+169>:   jle    0x401183 <phase_6+143>
   0x000000000040119f <+171>:   mov    $0x1,%eax
   0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx
   0x00000000004011a9 <+181>:   jmp    0x401176 <phase_6+130>
   0x00000000004011ab <+183>:   mov    0x20(%rsp)rr,%rbx
   0x00000000004011b0 <+188>:   lea    0x28(%rsp),%rax
   0x00000000004011b5 <+193>:   lea    0x50(%rsp),%rsi
   0x00000000004011ba <+198>:   mov    %rbx,%rcx
   0x00000000004011bd <+201>:   mov    (%rax),%rdx
   0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx)
   0x00000000004011c4 <+208>:   add    $0x8,%rax
   0x00000000004011c8 <+212>:   cmp    %rsi,%rax
   0x00000000004011cb <+215>:   je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:   mov    %rdx,%rcx
   0x00000000004011d0 <+220>:   jmp    0x4011bd <phase_6+201>
   0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx)
   0x00000000004011da <+230>:   mov    $0x5,%ebp
   0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:   mov    (%rax),%eax
   0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:   callq  0x40143a <explode_bomb>
   0x00000000004011ee <+250>:   mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:   sub    $0x1,%ebp
   0x00000000004011f5 <+257>:   jne    0x4011df <phase_6+235> 
   0x0000000000401203 <+271>:   retq