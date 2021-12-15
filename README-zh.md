# Polaris Lua based on C++

[English Document](./README.md)

## 1. 项目介绍

本项目在北极星已有的C++ sdk [polaris-cpp](https://github.com/polarismesh/polaris-cpp) 项目的基础上，实现了北极星服务发现的lua层封装，可集成到OpenResty等使用Lua脚本的系统中使用。

## 2. 快速上手

### 2.1 目录结构
```
.
├── lua                                     // lua的 C 静态库，头文件
│   ├── include
│   └── lib
│
`-- third_party
    `-- polaris_cpp_sdk                         // 编译生成的 polaris-cpp 静态库 & 动态库，头文件，需要自行编译
│
├── polaris_sdk_lua_wrapper.c               // 供 lua 使用的 C API 封装以及入口函数
├── polaris.lua                             // lua 层的北极星服务发现模块封装
├── test                                    // 测试用例
│   └── test_polaris_sdk_lua_wrapper.lua
│
├── pre_build.sh                            // 编译前执行，识别当前系统环境，链接合适的北极星sdk库
├── Makefile                                // Makefile 文件
└── README.md                               // 说明文档
```

### 2.2 使用方法

1. 拉取北极星c++ sdk的最新代码，`git clone https://github.com/polarismesh/polaris-cpp`
2. 编译`polaris-cpp`及打包
    ```
    # 编译
    make
    # 测试
    make test
    # 打包，可添加参数package_name指定打包名，默认polaris_cpp_sdk
    make package # package_name=polaris_cpp_sdk
    ```
3. 将`polaris-cpp`编译出来的产物`polaris_cpp_sdk`拷贝到本项目下的`third_party/polaris_cpp_sdk`
4. 执行 `bash pre_build.sh`
5. 执行 `make` 生成 `polariswrapper.so`
6. 将`polariswrapper.so`，`polaris.lua` 文件上传到 OpenResty 项目的合适路径，编写自己的业务 lua 文件使用，业务lua的编写示例可参考测试用例 `test_polaris_sdk_lua_wrapper.lua`
