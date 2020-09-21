#!/bin/sh

old_suffix="cpp"
new_suffix="txt"

for file in $(ls . | grep \\.${old_suffix})
    do
        name=$(find "${file}" | cut -d. -f1)
        sed -n '3,$p' "$file" > "${name}".${new_suffix}
    done
