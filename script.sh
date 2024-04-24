#!/bin/bash

file_path="$1"

chmod 777 "$file_path"

result="SAFE"

if grep -q -P '[^\x00-\x7F]' "$file_path"
then 
    result=""
fi

if grep -q -E 'malware|dangerous|risk|attack' "$file_path"
then
    result=""
fi

line_count=$(wc -l < "$file_path")
word_count=$(wc -w < "$file_path")
ch_count=$(wc -c < "$file_path")

if (( line_count < 3 && word_count > 1000 && ch_count > 2000 ))
then
    result=""
fi

if [ -z "$result" ]
then
    result="$file_path"
fi

echo "$result"