#!/bin/sh

for FILE in ./tests/input/*
do  
    filename=$(basename "$FILE")
    ./mydc < $FILE > stdout 2>stderr

    diff -q stdout ./tests/answer/${filename%.*}.stdout 
    diff -q stderr ./tests/answer/${filename%.*}.stderr

done

rm stderr stdout
