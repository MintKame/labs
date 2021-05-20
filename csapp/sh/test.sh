#! /bin/bash
###
 # @Author: your name
 # @Date: 2021-05-20 13:53:12
 # @LastEditTime: 2021-05-20 14:59:59
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: \sh\test.sh
### 
rm tsh.out
rm tshr.out
rm diff.out
for ((i = 1; i <= 9; i++))
do 
	./sdriver.pl -t trace0$i.txt -s ./tsh >> tsh.out
	./sdriver.pl -t trace0$i.txt -s ./tsh >> tshr.out
done

for ((; i <= 16; i++))
do 
	./sdriver.pl -t trace$i.txt -s ./tsh >> tsh.out
	./sdriver.pl -t trace$i.txt -s ./tsh >> tshr.out
done

diff tsh.out tshr.out > diff.out

