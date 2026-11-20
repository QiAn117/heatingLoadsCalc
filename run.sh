#!/usr/bin/env bash
# ================================================================
#  一键编译并运行（Linux / macOS）
#  用法：在 03-Code 目录下执行 ./run.sh
#  说明：脚本会切换到自身所在目录编译；若没有输入数据，
#        自动从 ../04-Data/01_clean/ 复制一套标准数据。
# ================================================================
set -e
cd "$(dirname "$0")"

CC="${CC:-gcc}"
SRC=$(ls *.c 2>/dev/null || true)

if [ -z "$SRC" ]; then
    echo "错误：当前目录没有 .c 源文件。"
    exit 1
fi

echo "== 编译 =="
$CC -std=c99 -Wall -O2 $SRC -o app -lm

if [ ! -f envelope.txt ]; then
    if [ -f ../04-Data/01_clean/envelope.txt ]; then
        cp ../04-Data/01_clean/envelope.txt ../04-Data/01_clean/weather.txt .
        echo "== 当前目录没有输入数据，已复制 ../04-Data/01_clean 的标准数据 =="
    else
        echo "[警告] 未找到 envelope.txt / weather.txt。"
        echo "        请把 04-Data 中任意一套数据拷到本目录，或直接在该数据目录运行 app。"
        exit 1
    fi
fi

echo "== 编译通过，开始运行（建议依次输入 1 2 3 4 5 6 7 8 0） =="
./app
