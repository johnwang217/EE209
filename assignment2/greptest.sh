#!/bin/bash
echo "enter test #: "
read num
chmod +x ./tests/input/find0$num.sh
./tests/input/find0$num.sh >stdout 2>stderr
diff ./tests/answer/find0$num.stdout stdout
diff ./tests/answer/find0$num.stderr stderr
