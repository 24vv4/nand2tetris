#!/bin/bash
for file in "Array" "Keyboard" "Math" "Screen" "Output" "String" "Sys" "Memory"; do
    mv "${file}.jack" "${file}Test"
done
