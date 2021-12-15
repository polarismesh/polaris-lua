#!/bin/bash

# 识别当前系统环境,选用合适的北极星依赖库
sys_version=`cat /etc/motd | grep "Welcome" | sed 's/Welcome to tlinux \([0-9]\.[0-9]*\) 64bit.*/\1/g'`

# 未能正确识别，让用户输入
if [ ! $sys_version ]; then
	read -p "Enter your tlinux version, please [1.2/2.2]: " sys_version
fi

echo "make symbolic link:"

# 删除已有软链接
polaris_slib_link="./third_party/polaris_cpp_sdk/slib/libpolaris_api.a"
protobuf_slib_link="./third_party/polaris_cpp_sdk/slib/libprotobuf.a"
polaris_dlib_link="./third_party/polaris_cpp_sdk/dlib/libpolaris_api.so"

if [ -f "$polaris_slib_link" ]; then
	rm $polaris_slib_link
fi

if [ -f "$protobuf_slib_link" ]; then
	rm $protobuf_slib_link
fi

if [ -f "$polaris_dlib_link" ]; then
	rm $polaris_dlib_link
fi

echo "cd third_party/polaris_cpp_sdk/slib"
cd third_party/polaris_cpp_sdk/slib

echo "ln -s libprotobuf.a.tlinux$sys_version libprotobuf.a"
ln -s libprotobuf.a.tlinux$sys_version libprotobuf.a
echo "ln -s libpolaris_api.a.tlinux$sys_version libpolaris_api.a"
ln -s libpolaris_api.a.tlinux$sys_version libpolaris_api.a

echo "cd ../dlib"
cd ../dlib

echo "ln -s libpolaris_api.so.tlinux$sys_version libpolaris_api.so"
ln -s libpolaris_api.so.tlinux$sys_version libpolaris_api.so
