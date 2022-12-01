#!/bin/bash

assert()
{
	expected="$1"
	input="$2"

	./mcc "$input" > ./tmp.s
	cc -o ./tmp ./tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input -> $actual"
	else
		echo "$input -> $expected : actual -> $actual"
		exit 1
	fi

}

assert 255 "5+120+130-1-1+2"
assert 41 " 12 + 34-5 "
assert 15 "5*(9-6)"
assert 6 "(3+5)/2+4/2"

echo "OK"
