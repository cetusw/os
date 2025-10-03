#!/bin/bash
set -e

if test -d out; then
    rm -rf out
fi

mkdir out

cd out

touch me.txt
# TODO: выяснить как дописать информацию, выводисмую программеой в файл
whoami > me.txt

cp me.txt metoo.txt

man wc > wchelp.txt

cat wchelp.txt

wc -l wchelp.txt | cut -d" " -f1 > wchelp-lines.txt

tac wchelp.txt > wchelp-reversed.txt

cat wchelp.txt wchelp-reversed.txt me.txt metoo.txt wchelp-lines.txt > all.txt

tar -cf result.tar *.txt

gzip result.tar -S .gz

cd ..

if test -f result.tar.gz; then
    rm result.tar.gz
fi

mv out/result.tar.gz .

rm -rf out/*
rmdir out


