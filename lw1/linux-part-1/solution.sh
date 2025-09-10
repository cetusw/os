#!/bin/bash
# TODO: узнать больше о #!/bin/bash. Что можно написать ещё

# TODO: выяснить как опредлелить, что вызванный скрипит завершился ошибкой
set -e

if test -d "out" ; then
    read -p "Каталог out уже существует. Пересоздать его? (y/n) " choice
    if [ "$choice" != "y" ]; then
        echo "Выход из скрипта."
        exit 0
    else
        rm -rf out
    fi
fi

# TODO: что можно использовать кроме gzip
tar -czf proj.tar.gz -C proj .
# bzip2 -j
# xz -J
# zstd -a


mkdir out

mv proj.tar.gz out/

cd out
# TODO: с помощью какой команды можно вывести текущую директорию
# TODO: как из текущей директории перейти в родительскую потом вернуться обратно

tar -xzf proj.tar.gz
rm proj.tar.gz

mkdir include src build

find . -maxdepth 1 -type f \( -name "*.h" -o -name "*.hpp" \) -exec mv {} include/ \;
find . -maxdepth 1 -type f -name "*.cpp" -exec mv {} src/ \;

g++ -I include src/*.cpp -o build/program

echo -e "30\n12" | ./build/program > stdout.txt

# TODO: выяснить что такое PATH
