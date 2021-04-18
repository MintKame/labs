# 答案
Border relations with Canada have never been better. 
1 2 4 8 16 32
1 311
7 0
9?>567
4 3 2 1 6 5

# 思路
+ 可以用strings和objdump分别获得printable_string和symbol_table
+ 用objdump disassemble 整个程序
+ 查看main 函数，main分别调用6个phase，参数为输入的string，存于%rdi

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

6. （推测是存储值和next指针的数组，通过设置next指针，对他们进行由大到小的排序
+ phase： 
  从输入中读6个int: a0..a5，
  分别存在: rsp + 0x0, 0x4, 0x8, 0xc, 0x10, 0x14
  rsp的值存在 r13，r14
  r12d = 0
  // 6个int为[1, 6]的排列（在1，6之间且各不相等）
  do{ // 循环5次， 第i次 （0..4）
    rbp = r13 
    if (*(r13) - 1 < 0 || > 5) bomb // 第i个值在[1, 6] 
    r12d++  // i + 1
    if(r12d == 6){ 
      break
    } 
    ebx = r12d // i + 1
    do{ // j = [i+1 .. 5]
      rax = ebx (符号扩展)
      eax = *(rsp + 4 * rax) // 第 j 个值
      if(rax == *(rbp)) bomb // 不等于第 i 个值
      ebx++
    }while(ebx <= 5) 
    r13 += 4
  }while(1)
  
  // 每个值 ai 改为 (7 - ai), 范围为 [1, 6]
  rsi = 0x18 + rsp // 6个数之后的地址
  rax = r14 // rsp
  ecx = 7
  do{
    edx = ecx - *(rax)
    *(rax) = edx
    rax += 4  // 移到下一个值
  }while(rax != rsi) 

  // 对第i个值ai，设置stack中 bi *(sp + 0x20 + 8i)为: (0x6032d0 + (ai - 1) * 0x10)  
  rsi = 0
  goto 163
  do{
    do{
      rdx = *(rdx + 8)  
      eax++
    }while(ecx != eax) // ecx = 7
    /* 地址： 数据 
      0x6032d8: 0x6032e0
      0x6032e8: 0x6032f0
      0x6032f8: 0x603300
      0x603308: 0x603310
    */ 
    goto 148
  
    do{
      edx = 0x6032d0 
// 148
      *(rsp + 2 * rsi + 0x20) = rdx 
      rsi += 4  // 需要执行6次
      if(rsi == 0x18) goto 183
// 163
      ecx = *(rsp + rsi)            
    }while(ecx <= 1)
  
    eax = 1
    edx = 0x6032d0  
  }while(1)

  // 对i = 0..4，设置 *(bi + 8) 为 bi+1; 
  // i = 5, *(bi + 8) 设置 为 0
// 183 
  rbx = *(rsp + 0x20)
  rcx = rbx
  rax = rsp + 0x28
  rsi = rsp + 0x50
  do{
    rdx = *(rax)
    *(rcx + 8) = rdx
    rax += 8
    if(rax == rsi) break
    rcx = rdx
  }while(1)   
  *(rdx + 8) = 0
  
  // 循环5次
  ebp = 5
  do{
    eax = *(*(rbx + 8))
    if(*(rbx) < eax) bomb
    rbx = *(rbx + 8)
    ebp -= 1
  }while(ebp != 0)  
需要排列8结尾的行使得满足 低四位 *(ci) >= *(*(ci + 8)) // ci 为0结尾的行的地址

用a-f，标识指令，其中a-f的值由小到大， 
按顺序改为时，满足要求
0x6032d0 <node1>:       0x4c    0x01           <b> 
0x6032d8 <node1+8>:     0xe0    0x32    0x60  
0x6032e0 <node2>:       0xa8    0x00           <a>
0x6032e8 <node2+8>:     0xf0    0x32    0x60   
0x6032f0 <node3>:       0x9c    0x03           <f> <- head
0x6032f8 <node3+8>:     0x00    0x33    0x60 
0x603300 <node4>:       0xb3    0x02           <e>
0x603308 <node4+8>:     0x10    0x33    0x60  
0x603310 <node5>:       0xdd    0x01           <d>
0x603318 <node5+8>:     0x20    0x33    0x60   
0x603320 <node6>:       0xbb    0x01           <c>
0x603318 <node5+8>:     0                    