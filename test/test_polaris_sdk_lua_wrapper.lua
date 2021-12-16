package.cpath = "/data/app/openresty1.7.4.1/lua/polaris_test_demo/so/?.so;"
package.path = "/data/app/openresty1.7.4.1/lua/polaris_test_demo/?.lua;"..package.path

local polaris = require "polaris"

-- 显式初始化polaris_api
-- 注：需要指定polaris.yaml时需调用，否则可以直接调用get_server_host_port，包含了隐式初始化polaris_api
polaris.polaris_api_init("/data/app/openresty1.7.4.1/nginx/conf/polaris.yaml")

-- 设置北极星服务日志路径
polaris.polaris_log_settings("/data/app/openresty1.7.4.1/nginx/logs/polaris/log", 0)

-- 获取服务实例信息
local get_host_result, msg, host_info = polaris.get_server_host_port("default", "echosvr.Greeter")

if get_host_result == 0 then
    print(string.format("get host: %s, port %d, instance_id: %s", host_info.host, host_info.port, host_info.instance_id))
else
    print("get_server_host_port failed, get_host_result:"..get_host_result..", msg:"..msg)
end

-- 调用上报
local call_report_result, errmsg = polaris.call_report("default", "echosvr.Greeter", host_info.instance_id, get_host_result)
if call_report_result ~= 0 then
    print("polaris call_report failed, call_report_result:"..call_report_result..", errmsg:"..errmsg)
end
