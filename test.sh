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
echo "subtraction OK"

echo OK