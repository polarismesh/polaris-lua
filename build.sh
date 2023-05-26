#!/bin/bash

make clean

workdir=$(pwd)

rm -rf polaris-cpp
cd ${workdir}/third_party
rm -rf polaris_cpp_sdk

cd ${workdir}

polaris_cpp_tag=$1

git clone https://github.com/polarismesh/polaris-cpp

cd polaris-cpp

if [ "${polaris_cpp_tag}" != "" ]; then
    git checkout ${polaris_cpp_tag}
fi

make package

mv polaris_cpp_sdk.tar.gz ../third_party

cd ${workdir}/third_party

tar -zxvf polaris_cpp_sdk.tar.gz

cd ${workdir}

make
