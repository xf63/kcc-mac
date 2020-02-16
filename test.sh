#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./kcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else
      echo "$input => $expected is expected, but got $actual"
      exit 1
    fi
}

try 0 0
try 43 43
echo "single value OK"
try 5 '2+3'
echo "addition OK"
try 4 '10-6'
try 10 '14 + 3 - 7'
echo "subtraction OK"
try 40 '5 * 4 * 2'
try 16 '20 - 2 * 2'
echo "multiplication OK"
try 10 '5 * 4 / 2'
try 13 '20 - 14 / 2'
echo "division OK"
try 12 '6 * (4 - 2)'
echo "parentheses OK"
try 11 '-9 + 20'
try 5 '+5'
echo "unary OK"
try 1 "5==2+3"
try 0 "5==3+3"
try 0 "5!=2+3"
try 1 "5!=3+3"
echo "equality OK"

echo OK