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

echo OK