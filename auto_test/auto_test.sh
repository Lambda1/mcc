#!/bin/bash

assert()
{
	expected="$1"
	input="$2"

	./mcc "$input" > ./tmp.s
	cc -o ./tmp ./tmp.s func_test.o
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input -> $actual"
	else
		echo "$input -> $expected : actual -> $actual"
		exit 1
	fi

}

assert 47 '5 +6 *7;'
assert 15 '5*(  9- 6 );'
assert 4 '(3 +5 )/ 2;'
assert 10 '-10+20;'
assert 246 "1*+2-(3*-4*-5)/6;"

assert 1 "1+ 1== 2;"
assert 0 "1+ 1== 2/2;"
assert 1 "3- 2 == 2 / 2 == 10/2/ 5;"

assert 0 "3 != 3;"
assert 1 "3 != 2;"
assert 1 "(6/2 != 12/4) == 0;"

assert 1 "1+2*3 > 4/5;"
assert 0 "1+2*3+4 >= 5*6;"
assert 0 "1+2*3 < 4/5;"
assert 1 "1+2*3+4 <= 5*6;"

assert 1 "(5*6==30)>=(7!=21/3);"

assert 85 "a=12/4; b =4* 5-1; c = a+b; z = c*a+b;"
assert 1 "a=3;f=2; d=(a!=f);"
assert 121 "hoge = 1+2*3; foo = 4+(5-6); bar=(foo*hoge)+100;"

assert 12 "aiueo = 4*3/5; gdaga=(1+2)*aiueo; return gdaga*aiueo;"

assert 129 "a=1; if(a==1) return 129; return 4;"
assert 4 "a=4; if(a==1) return 129; return 4;"
assert 129 "a=1; if(a==1) return 129; else if(a==2) return 5; return 4;"
assert 5 "a=2; if(a==1) return 129; else if(a==2) return 5; return 4;"
assert 4 "a=3; if(a==1) return 129; else if(a==2) return 5; return 4;"
assert 9 "a=3; if(a==1) return 129; else if(a==2) return 5; else if (a==3) return 9; return 4;"
assert 111 "a=4; if(a==1) return 129; else if(a==2) return 5; else if (a==3) return 9; else return 111; return 4;"

assert 40 "a=100; while(a>40) a= a- 1; return a;"
assert 42 "aaa=0; nn = 3; while(aaa<40) aaa= aaa+nn; return aaa;"
assert 19 "i=0;j=0;while(i<9) while(j<10) if(i != j) i = i + 1; else j = j + 1; return (i+j);"

assert 6 "a=2;b=3; if(a==2) { a=a+1; b=a*2; } return b;"
assert 3 "a=3;b=3; if(a==2) { a=a+1; b=a*2; } return b;"
assert 9 "a=2;b=3; if(a==2) { a=a+1; b=a*2; } return (a+b);"
assert 202 "a=3;b=3;if(a==3) { a=100; b=a+2; } return a+b;"
assert 199 "a=0; b = 2; c = 0; while(a<100){ b = a + 1; a = b + 1; c = a + b;} return c;"

assert 10 "a=0; b=0; while(a<10){ if((a+b)/2 == 0) { foo(); } else { b = -a; } a = a+1; } return a;"

echo "OK"
