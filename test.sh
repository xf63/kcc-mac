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

try 0 'int main() 0;'
try 43 'int main() 43;'
echo "single value OK"
try 5 'int main() 2+3;'
echo "addition OK"
try 4 'int main() 10-6;'
try 10 'int main() 14 + 3 - 7;'
echo "subtraction OK"
try 40 'int main() 5 * 4 * 2;'
try 16 'int main() 20 - 2 * 2;'
echo "multiplication OK"
try 10 'int main() 5 * 4 / 2;'
try 13 'int main() 20 - 14 / 2;'
echo "division OK"
try 12 'int main() 6 * (4 - 2);'
echo "parentheses OK"
try 11 'int main() -9 + 20;'
try 5 'int main() +5;'
echo "unary OK"
try 1 "int main() 5==2+3;"
try 0 "int main() 5==3+3;"
try 0 "int main() 5!=2+3;"
try 1 "int main() 5!=3+3;"
echo "equality OK"
try 1 'int main() 10 > 3 * 3;'
try 0 'int main() 14 >= 3 * 5;'
try 0 'int main() 9 < 3 * 3;'
try 1 'int main() 7 <= 2 * 4;'
echo "relational OK"
try 43 'int main() {42; 43;}'
echo "multiple statement OK"
try 5 'int main() {int a; a = 5; a;}'
try 23 'int main() {int foo; int bar; foo = 11; bar = 12; foo+bar;}'
echo "local variable OK"
try 3 'int main() {return 3; return 2;}'
echo "return syntax OK"
try 10 'int main() {int a;a=0; if (a==0) return 10; return 11;}'
echo "if syntax OK"
try 11 'int main() {int a;a=0; if (a==1) return 10; else return 11; return 12;}'
echo "if-else syntax OK"
try 6 'int main() {int a;a=0; while (a<5) a = a + 2; return a;}'
echo "while syntax OK"
try 5 'int main() {int loop;loop=0;int i;for(i=0;i<5;i=i+1) loop=loop+1; return loop;}'
echo "for syntax OK"
try 15 'int main() {int l1;int l2;l1=0;l2=0;int a;for (a=0; a<5; a=a+1) {l1=l1+1;l2=l2+2;} return l1+l2;}'
echo "braces block OK"
try 11 'int return10(); int main() {return10() + 1;}' cf
try 28 'int add6(); int main() {7 + add6(5,4,3,2,1,0);}' cf
echo "calling C-language functions OK"
try 10 'int f() {return 10;} int main() {return f();}'
try 120 'int f(int a,int b,int c,int d) {return a*b*c*d;} int main() {return f(2,3,4,5);}'
echo "define function OK"
try 3 'int main() {int x;int *y;x=3;y=&x;return *y;}'
try 14 'int main() {int x;int y;int *z;x=14;y=5;z=&y+1;return *z;}'
try 2 'int main() {int a;int *b; b=&a; *b=2; return a;}'
echo "adress access OK"
try 4 'int main() {int x; return sizeof(x);}'
try 8 'int main() {int *y; return sizeof(y);}'
try 40 'int main() {int a[2][5]; return sizeof(a);}'
echo "sizeof syntax OK"
try 7 'int main() {int x[5];*x=2;*(x+1)=5;int *p;p=x; return *p+*(p+1);}'
try 10 'int main() {int x[5];x[0]=6;x[1]=4;int *p;p=x; return *p+*(p+1);}'
echo "array access OK"
try 12 'int a; int main() {a=12;return a;}'
echo "global variable OK"
try 1 'int main() {char c; return sizeof(c);}'
try 3 'int main() {char c[4];c[0]=-1;int i;i=4;return i+c[0];}' 
echo "char type OK"
try 4 'int main() {return sizeof("abc");}'
try 97 'int main() {char *s; s = "abcde"; return s[0];}'
echo OK