#!/bin/bash

if [ -d "out" ]; then
    read -p "Каталог out уже существует. Пересоздать его? (y/n) " choice
    if [ "$choice" != "y" ]; then
        echo "Выход из скрипта."
        exit 0
    else
        rm -rf out
    fi
fi

tar -czf proj.tar.gz -C proj .

mkdir out

mv proj.tar.gz out/

cd out

tar -xzf proj.tar.gz
rm proj.tar.gz

mkdir include src build

find . -maxdepth 1 -type f \( -name "*.h" -o -name "*.hpp" \) -exec mv {} include/ \;
find . -maxdepth 1 -type f -name "*.cpp" -exec mv {} src/ \;

g++ -Iinclude src/*.cpp -o build/program

echo -e "30\n12" | ./build/program > stdout.txt
