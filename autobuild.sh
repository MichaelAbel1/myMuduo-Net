#!/bin/bash

# 确保出错时不会继续执行后续的命令
set -e

# 如果没有build目录，创建该目录
# -d 用来检查目录是否存在
if [ ! -d $(pwd)/build ]; then
    mkdir $(pwd)/build
fi

# -r 表示递归删除,-f 表示强制删除
rm -rf $(pwd)/build/*

# 进入 build 目录。
# 运行 cmake .. 命令,生成 Makefile 文件。
# 运行 make 命令,编译项目。
cd $(pwd)/build &&
    cmake .. &&
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/mymuduo so库拷贝到 /usr/lib  PATH

if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

# 将头文件安装到 /usr/include/mymuduo目录
# 这是为了让编译器能够找到这些头文件。
for header in $(ls *.h)
do
    cp $header /usr/include/mymuduo
done

# 将动态库 libmymuduo.so 安装到 /usr/lib 目录:
# 这是为了让链接器能够找到这个动态库文件
cp $(pwd)/lib/libmymuduo.so /usr/lib

# 更新动态链接库缓存
ldconfig