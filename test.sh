#!/bin/bash
build_binary() {
    gcc -o tmp tmp.s
}

build_binary_with_cfuncs() {
    gcc -c test/clang_funcs.c -o test/clang_funcs.o
    gcc -o tmp tmp.s test/clang_funcs.o
}

try() {
    expected="$1"
    input="$2"
    with_funcs="$3"

    ./kcc "$input" > tmp.s
    if [ "$with_funcs" = "cf" ]; then
        build_binary_with_cfuncs
    else
        build_binary
    fi
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else
      echo "$input => $expected is expected, but got $actual"
      exit 1
    fi
}

try 0 '0;'
try 43 '43;'
echo "single value OK"
try 5 '2+3;'
echo "addition OK"
try 4 '10-6;'
try 10 '14 + 3 - 7;'
echo "subtraction OK"
try 40 '5 * 4 * 2;'
try 16 '20 - 2 * 2;'
echo "multiplication OK"
try 10 '5 * 4 / 2;'
try 13 '20 - 14 / 2;'
echo "division OK"
try 12 '6 * (4 - 2);'
echo "parentheses OK"
try 11 '-9 + 20;'
try 5 '+5;'
echo "unary OK"
try 1 "5==2+3;"
try 0 "5==3+3;"
try 0 "5!=2+3;"
try 1 "5!=3+3;"
echo "equality OK"
try 1 '10 > 3 * 3;'
try 0 '14 >= 3 * 5;'
try 0 '9 < 3 * 3;'
try 1 '7 <= 2 * 4;'
echo "relational OK"
try 43 '42; 43;'
echo "multiple statement OK"
try 5 'a = 5; a;'
try 23 'foo = 11; bar = 12; foo+bar;'
echo "local variable OK"
try 3 'return 3; return 2;'
echo "return syntax OK"
try 10 'a=0; if (a==0) return 10; return 11;'
echo "if syntax OK"
try 11 'a=0; if (a==1) return 10; else return 11; return 12;'
echo "if-else syntax OK"
try 6 'a=0; while (a<5) a = a + 2; return a;'
echo "while syntax OK"
try 5 'loop=0;for(i=0;i<5;i=i+1) loop=loop+1; return loop;'
echo "for syntax OK"
try 15 'l1=0;l2=0;for (a=0; a<5; a=a+1) {l1=l1+1;l2=l2+2;} return l1+l2;'
echo "braces block OK"
try 11 'return10() + 1;' cf
try 28 '7 + add6(5,4,3,2,1,0);' cf
echo OK