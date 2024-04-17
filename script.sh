#!/bin/bash

file_path="$1"
destinatie="$2"

chmod 777 "$file_path"

if grep -q -P '[^\x00-\x7F]' "$file_path"
then 
    mv "$file_path" "$destinatie"
    exit 1
fi
if grep -q -E 'malware|dangerous|risk|attack' "$file_path"
then
    mv "$file_path" "$destinatie"
    exit 1
else
    exit 0
fi

