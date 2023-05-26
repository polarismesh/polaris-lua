/**
 * Copyright (c) 2020, Tencent Inc. All rights reserved.
 *
 * author:  andymatthu@tencent.com
 * desc:    北极星SDK的lua封装
 *          约定返回给lua层的结果，前两个参数固定为result和msg
            result为0时返回结果正确
 */

#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "polaris/polaris_api.h"

#define ERR_CODE_ARGS_COUNT 1001                      // 传入参数个数有误
#define ERR_CODE_POLARIS_API_INIT 1002                // 北极星api初始化失败
#define ERR_CODE_GET_POLARIS_API 1003                 // 获取北极星api错误
#define ERR_CODE_GET_SERVICE_NAMESPACE 1004           // 解析传入参数-命名空间-失败
#define ERR_CODE_GET_SERVICE_NAME 1005                // 解析传入参数-服务名-失败
#define ERR_CODE_GET_ONE_INSTANCE_REQ_NEW 1006        // 创建获取单个服务实例请求对象失败
#define ERR_CODE_GET_ONE_INSTANCE 1007                // 获取单个服务实例失败
#define ERR_CODE_GET_LOG_DIR 1008                     // 解析传入参数-日志路径-失败
#define ERR_CODE_GET_INSTANCE_ID 1009                 // 解析传入参数-服务实例ID-失败
#define ERR_CODE_POLARIS_SERVICE_CALL_RESULT_NEW 1010 // 创建服务实例调用结果上报对象失败
#define ERR_CODE_CALL_RET_TIMEOUT 1011                // 获取单个服务实例超时
#define ERR_CODE_GET_CONFIG_FILE 1012                 // 解析传入参数-sdk配置文件路径-失败

static polaris_api *polaris_api_ptr = NULL;

/* 发生错误时报告错误 */
int report_error(lua_State *l, int result, const char *msg)
{
    lua_pushinteger(l, result);
    lua_pushstring(l, msg);

    return 2;
}

/* 检测函数调用参数个数是否正常 */
int check_args_count(lua_State *l, int args_count)
{
    /* lua_gettop获取栈中元素个数 */
    if (lua_gettop(l) != args_count)
    {
        return report_error(l, ERR_CODE_ARGS_COUNT, "error : num of args");
    }

    return 0;
}

/* 检测函数调用参数个数是否小于某个数值 (用于可变参数) */
int check_args_less(lua_State *l, int args_count)
{
    /* lua_gettop获取栈中元素个数 */
    if (lua_gettop(l) < args_count)
    {
        return report_error(l, ERR_CODE_ARGS_COUNT, "error : num of args");
    }

    return 0;
}

/* 填充正确返回时的必填参数 */
void fill_ok_result(lua_State *l)
{
    lua_pushinteger(l, 0);
    lua_pushstring(l, "ok");
}

/* 从栈上解析metadata table数据并设置 */
void set_metadata_from_stack(lua_State *l, int index, polaris_get_one_instance_req *get_one_instance_req)
{
    luaL_checktype(l, index, LUA_TTABLE);

    lua_pushnil(l);
    while (lua_next(l, index))

    {
        lua_pushvalue(l, -2);

        const char *key = lua_tostring(l, -1);
        const char *value = lua_tostring(l, -2);
        printf("%s => %s\n", key, value);

        /* 设置 metadata */
        polaris_get_one_instance_req_add_src_service_metadata(get_one_instance_req, key, value);
        lua_pop(l, 2);
    }
}

/**
 * 初始化polaris_api
 *
 * 传入参数:
 * 1 string 北极星yaml配置文件路径 (可选参数)
 *
 * 传出参数:
 * 1 integer result
 * 2 string errmsg
 */
static int polaris_api_init(lua_State *l)
{
    /* 已经初始化polaris_api */
    if (polaris_api_ptr != NULL)
    {
        fill_ok_result(l);
        return 2;
    }

    /* 指定了北极星yaml配置文件路径 */
    if (lua_gettop(l) == 1)
    {
        const char *config_file = lua_tostring(l, 1);
        if (config_file == NULL)
        {
            return report_error(l, ERR_CODE_GET_CONFIG_FILE, "get config_file failed");
        }
        polaris_api_ptr = polaris_api_new_from(config_file);
    }
    else
    {
        polaris_api_ptr = polaris_api_new();
    }

    /* api初始化出错处理 */
    if (polaris_api_ptr == NULL)
    {
        return report_error(l, ERR_CODE_POLARIS_API_INIT, "create polaris api failed");
    }

    fill_ok_result(l);
    return 2;
}

/**
 * 初始化polaris_api并获取一个服务实例(如当前进程已经初始化polaris_api则复用)
 *
 * 传入参数:
 * 1 string 命名空间
 * 2 string 服务名
 * 3 table metadata kv (可选参数)
 * 4 string 主调服务命名空间 (可选参数，传了metadata则必填)
 * 5 string 主调服务名 (可选参数，传了metadata则必填)
 *
 * 传出参数:
 * 1 integer result
 * 2 string errmsg
 * 3 string 获取实例的ip
 * 4 integer 获取实例的端口号
 * 5 string 获取实例的实例id
 * 6 integer 本次调用是否复用了 polaris_api，1-复用 0-未复用
 */
static int polaris_get_one_node(lua_State *l)
{
    /* 参数校验 */
    int ret = check_args_less(l, 2);
    if (ret != 0)
    {
        return ret;
    }

    /* 获取命名空间 */
    const char *service_namespace = lua_tostring(l, 1);
    if (service_namespace == NULL)
    {
        return report_error(l, ERR_CODE_GET_SERVICE_NAMESPACE, "get service_namespace failed");
    }

    /* 获取服务名 */
    const char *service_name = lua_tostring(l, 2);
    if (service_name == NULL)
    {
        return report_error(l, ERR_CODE_GET_SERVICE_NAME, "get service_name failed");
    }

    /* 标记是否复用了 polaris_api */
    int if_reuse = 0;

    /* 判断 polaris_api 是否存在，不存在则初始化 */
    if (polaris_api_ptr == NULL)
    {
        polaris_api_ptr = polaris_api_new();

        /* api初始化出错处理 */
        if (polaris_api_ptr == NULL)
        {
            return report_error(l, ERR_CODE_POLARIS_API_INIT, "create polaris api failed");
        }
    }
    else
    {
        if_reuse = 1;
    }

    /* 准备请求 */
    polaris_get_one_instance_req *get_one_instance_req = polaris_get_one_instance_req_new(service_namespace, service_name);
    if (get_one_instance_req == NULL)
    {
        return report_error(l, ERR_CODE_GET_ONE_INSTANCE_REQ_NEW, "get_one_instance_req init failed");
    }

    /* 检查传入参数是否有metadata并在获取实例 */
    if (lua_gettop(l) >= 5)
    {
        /* 获取主调命名空间 */
        const char *source_namespace = lua_tostring(l, 4);
        if (source_namespace == NULL)
        {
            return report_error(l, ERR_CODE_GET_SERVICE_NAMESPACE, "get source service_namespace failed");
        }
        /* 获取主调服务名 */
        const char *source_service_name = lua_tostring(l, 5);
        if (source_service_name == NULL)
        {
            return report_error(l, ERR_CODE_GET_SERVICE_NAMESPACE, "get source_service_name failed");
        }
        /* 设置源服务用于匹配规则路由 */
        polaris_get_one_instance_req_set_src_service_key(get_one_instance_req, source_namespace, source_service_name);

        /* 填充匹配规则 */
        set_metadata_from_stack(l, 3, get_one_instance_req);
    }

    polaris_instance *instance = NULL;

    /* 获取实例 */
    ret = polaris_api_get_one_instance(polaris_api_ptr, get_one_instance_req, &instance);
    if (ret != 0)
    {
        /* 转换一下错误码 */
        if (ret == POLARIS_CALL_RET_TIMEOUT)
        {
            return report_error(l, ERR_CODE_CALL_RET_TIMEOUT, polaris_get_err_msg(ret));
        }
        else
        {
            return report_error(l, ERR_CODE_GET_ONE_INSTANCE, polaris_get_err_msg(ret));
        }
    }

    fill_ok_result(l);

    /* 从实例中解析ip、port、instance_id并返回 */
    lua_pushstring(l, polaris_instance_get_host(instance));
    lua_pushinteger(l, polaris_instance_get_port(instance));
    lua_pushstring(l, polaris_instance_get_id(instance));
    /* 返回是否复用了polaris_api的标志，可在lua层打印用于判断 */
    lua_pushinteger(l, if_reuse);

    /* 销毁实例&请求 */
    polaris_instance_destroy(&instance);
    polaris_get_one_instance_req_destroy(&get_one_instance_req);

    return 6;
}

/**
 * 设置北极星日志路径&级别
 *
 * 传入参数:
 * 1 string 设置的北极星sdk日志路径
 * 2 integer 设置的北极星日志级别
 *
 * 传出参数:
 * 1 integer result
 * 2 string errmsg
 */
static int polaris_log_settings(lua_State *l)
{
    /* 参数校验 */
    int ret = check_args_count(l, 2);
    if (ret != 0)
    {
        return ret;
    }

    /* 获取设置的日志路径 */
    const char *log_dir = lua_tostring(l, 1);
    if (log_dir == NULL)
    {
        return report_error(l, ERR_CODE_GET_LOG_DIR, "get log_dir failed");
    }

    /* 获取设置的日志级别 */
    int log_level = lua_tointeger(l, 2);

    /* 设置日志路径 */
    polaris_set_log_dir(log_dir);

    /* 设置日志级别 */
    if (log_level > 0)
    {
        polaris_set_log_level((PolarisLogLevel)log_level);
    }

    fill_ok_result(l);

    return 2;
}

/**
 * 服务实例调用结果上报
 *
 * 传入参数:
 * 1 string 命名空间
 * 2 string 服务名
 * 3 string instance_id
 * 4 integer 获取服务实例返回的错误码
 *
 * 传出参数:
 * 1 integer result
 * 2 string errmsg
 */
int polaris_service_call_report(lua_State *l)
{
    /* 参数校验*/
    int ret = check_args_count(l, 4);
    if (ret != 0)
    {
        return ret;
    }

    /* 获取命名空间 */
    const char *service_namespace = lua_tostring(l, 1);
    if (service_namespace == NULL)
    {
        return report_error(l, ERR_CODE_GET_SERVICE_NAMESPACE, "get service_namespace failed");
    }

    /* 获取服务名 */
    const char *service_name = lua_tostring(l, 2);
    if (service_name == NULL)
    {
        return report_error(l, ERR_CODE_GET_SERVICE_NAME, "get service_name failed");
    }

    /* 获取instance_id */
    const char *instance_id = lua_tostring(l, 3);
    if (instance_id == NULL)
    {
        return report_error(l, ERR_CODE_GET_INSTANCE_ID, "get instance_id failed");
    }

    /* 获取实例的返回码 */
    int result = lua_tointeger(l, 4);

    /* 创建服务实例调用结果上报对象 */
    polaris_service_call_result *service_call_result_ptr = polaris_service_call_result_new(service_namespace, service_name, instance_id);
    if (service_call_result_ptr == NULL)
    {
        return report_error(l, ERR_CODE_POLARIS_SERVICE_CALL_RESULT_NEW, "polaris_service_call_result_new failed");
    }

    /* 转换上报返回码 */
    int call_ret_status = 0;
    if (result == 0)
    {
        call_ret_status = POLARIS_CALL_RET_OK;
    }
    else if (result == ERR_CODE_CALL_RET_TIMEOUT)
    {
        call_ret_status = POLARIS_CALL_RET_TIMEOUT;
    }
    else
    {
        call_ret_status = POLARIS_CALL_RET_ERROR;
    }

    /* 设置上报参数 */
    polaris_service_call_result_set_ret_status(service_call_result_ptr, call_ret_status);
    polaris_service_call_result_set_ret_code(service_call_result_ptr, result);

    /* todo 设置delay */

    /* 判断 polaris_api 是否存在 */
    if (polaris_api_ptr == NULL)
    {
        polaris_api_ptr = polaris_api_new();

        /* api初始化出错处理 */
        if (polaris_api_ptr == NULL)
        {
            return report_error(l, ERR_CODE_POLARIS_API_INIT, "create polaris api failed");
        }
    }

    /* 调用结果上报 */
    ret = polaris_api_update_service_call_result(polaris_api_ptr, service_call_result_ptr);

    /* 释放服务实例调用结果上报对象 */
    polaris_service_call_result_destroy(&service_call_result_ptr);

    fill_ok_result(l);

    return 2;
}

static int polaris_get_quota(lua_State *l)
{
}

/* 导出函数列表 */
static luaL_Reg polariswrapper_libs[] =
    {
        /* 初始化polaris_api */
        {"polaris_api_init", polaris_api_init},
        /* 获取一个服务实例(如当前进程已经初始化polaris_api则复用，否则生成一个polaris_api) */
        {"polaris_get_one_node", polaris_get_one_node},
        /* 设置北极星日志路径&级别 */
        {"polaris_log_settings", polaris_log_settings},
        /* 服务实例调用结果上报(如当前进程已经初始化polaris_api则复用，否则生成一个polaris_api) */
        {"polaris_service_call_report", polaris_service_call_report},
        // 限流申请访问配额
        {"polaris_get_quota", polaris_get_quota},
        {NULL, NULL}};

/**
 * 该C库的唯一入口函数 其函数签名等同于上面的注册函数
 */
int luaopen_polariswrapper(lua_State *l)
{
    const char *lib_name = "polariswrapper";
    luaL_register(l, lib_name, polariswrapper_libs);
    return 1;
}