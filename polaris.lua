--[[
    Author: andymatthu#tencent.com
    Create date: 2020/06/11
    Update date: 2020/11/04
    Desc: 北极星sdk lua层封装
]]

local _M = {}

local polariswrapper = require "polariswrapper"

-- 显式初始化polaris_api(单进程只会初始化一次)
-- 注：需要指定polaris.yaml时需调用，否则可以直接调用get_server_host_port，包含了隐式初始化polaris_api
function _M.polaris_api_init(config_file)
    local result, msg
    if config_file ~= "" then
        result, msg = polariswrapper.polaris_api_init(config_file)
    else
        result, msg = polariswrapper.polaris_api_init()
    end

    return result, msg
end

-- 设置北极星服务日志路径
function _M.polaris_log_settings(log_dir, log_level)
    polariswrapper.polaris_log_settings(log_dir, log_level)
end


-- 通过北极星获取服务ip和端口
function _M.get_server_host_port(service_namespace, service_name)
    -- 查询host port
    local result, msg, host, port, instance_id, if_reuse = polariswrapper.polaris_get_one_node(service_namespace, service_name)
    if result ~= 0 then
        ngx.log(ngx.ERR, "polaris_get_one_node failed, result:"..result..", msg:"..msg)
        return result, msg
    end
    ngx.log(ngx.INFO, "polaris_api if_reuse:"..if_reuse)

    return 0, "ok", {host=host, port=port, instance_id=instance_id}
end

-- 上报北极星调用结果
function _M.call_report(service_namespace, service_name, instance_id, result)
    local result, msg = polariswrapper.polaris_service_call_report(service_namespace, service_name, instance_id, result)
    if result ~= 0 then
        ngx.log(ngx.ERR, "polaris_service_call_report failed, result:"..result..", msg:"..msg)
        return result, msg
    end

    return 0, "ok"
end

return _M
