#!/bin/bash

CURRENT_DIR=$(pwd)
echo "Начальная директория: $CURRENT_DIR"

cd ..
cd ..

echo "Текущая директория: $(pwd)"

cd $CURRENT_DIR

echo "Директория, в которую вернулись: $(pwd)"


