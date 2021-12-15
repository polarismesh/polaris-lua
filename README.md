# Polaris Lua

[中文文档](./README-zh.md)

## 1. Project Introduction 

Based on the existing C++ sdk [polaris-cpp](https://github.com/polarismesh/polaris-cpp)  project of Polaris, this project implements the Lua layer encapsulation of Polaris service discovery, which can be integrated into OpenResty and other systems that use Lua scripts. 
## 2. Get started quickly 
### 2.1 Directory Structure 
```
.
├── lua                                     // Lua C static library, header file 
│   ├── include
│   └── lib
│
`-- third_party
    `-- polaris_cpp_sdk                         // Compile the generated polaris-cpp static library & dynamic library, header file, you need to compile it yourself 
│
├── polaris_sdk_lua_wrapper.c               // C API wrapper and entry function for lua 
├── polaris.lua                             // Polaris service discovery module package at the lua layer 
├── test                                    // Test case 
│   └── test_polaris_sdk_lua_wrapper.lua
│
├── pre_build.sh                            // Execute before compiling, identify the current system environment, link the appropriate Polaris SDK library 
```

### 2.2 Instructions

1. Pull the latest code of Polaris c++ sdk, `git clone https://github.com/polarismesh/polaris-cpp`
2. Compile `polaris-cpp` and package 
    ```
    make
    make test
    make package # package_name=polaris_cpp_sdk
    ```
3. Copy the product `polaris_cpp_sdk` compiled by `polaris-cpp` to `third_party/polaris_cpp_sdk` under this project 
4. Execute `bash pre_build.sh` 
5. Execute `make` to generate `polariswrapper.so` 
6. 将`polariswrapper.so`，`polaris.lua` 文件上传到 OpenResty 项目的合适路径，编写自己的业务 lua 文件使用，业务lua的编写示例可参考测试用例 `test_polaris_sdk_lua_wrapper.lua`

