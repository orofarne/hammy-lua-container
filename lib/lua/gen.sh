#!/bin/sh

echo "namespace hammy {"
echo
echo "extern const char lua_global_code[] ="

for f in $@; do
    cat $f | sed 's#\\#\\\\#g' | sed 's/"/\\"/g' | sed 's/^/    "/' | sed 's/$/\\n"/'
done
echo ";"

echo
echo "}"
