// main.cpp
#include "SwaggerComponent.hpp"
#include "handler/handler.h"
#include "logger/fnLog.hpp"
#include "oatpp-swagger/Controller.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "utils/getHostInfo.hpp"
#include "version.h"

#include "controller/UserController.hpp"
//  配置文件路径
#define YAML_FILE_OF_LOGGER "/app/oatpp-server/config/logger.yaml"

#define VERSIONS VERSION_MAJOR "." VERSION_MINOR "." VERSION_PATCH " " BUILD_TIMESTAMP
/*
版本格式：主版本号.次版本号.修订号 时间戳

版本号递增规则如下：
      主版本号：做了不兼容的 API 修改，
      次版本号：做了向下兼容的功能性新增，
      修订号：做了向下兼容的问题修正。
      先行版本号及版本编译元数据可以加到“主版本号.次版本号.修订号”的后面，作为延伸。

*/

void run(std::string ipAddress, int port) {
    LogInfoStream(0, 0, 0) << "server is runing ";  //

    // 为 HTTP 请求创建路由器
    auto router = oatpp::web::server::HttpRouter::createShared();
    LogInfoStream(0, 0, 0) << "create route";  //

    // 路由 GET - "/hello" 请求到处理程序
    router->route("GET", "/hello", std::make_shared<Handler>());

    // 创建 HTTP 连接处理程序
    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    LogInfoStream(0, 0, 0) << "create HTTP connect";  //

    // 创建 TCP 连接提供者
    auto connectionProvider =
        oatpp::network::tcp::server::ConnectionProvider::createShared({ipAddress, port, oatpp::network::Address::IP_4});

    // 创建服务器，它接受提供的 TCP 连接并将其传递给 HTTP 连接处理程序
    oatpp::network::Server server(connectionProvider, connectionHandler);
    LogInfoStream(0, 0, 0) << "create Server ";  //

    // 打印服务器端口
    OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

    // 运行服务器
    server.run();
}

// api doc 注释样例
/**
 * @api {Get} /user/get getUserInfo
 * @apiGroup User
 *
 * @apiParam {String} name 文章名
 * @apiParamExample {json} Request-Example
 * {
 *  "userName": "Eve"
 * }
 *
 * @apiSuccessExample  {json} Response-Example
 * {
 *   "userName": "Eve",
 *   "createTime": "1568901681"
 *   "updateTime": "1568901681"
 * }
 */
int main() {
    // 启动日志记录器
    int ret = FNLog::LoadAndStartDefaultLogger(YAML_FILE_OF_LOGGER);
    if (ret != 0) {
        return ret;
    }
    LogInfoStream(0, 0, 0) << "System V" << VERSIONS;  //

    std::string hostName, ipAddress;
    bool retsult = GetHostInfo(hostName, ipAddress);
    if (true == retsult) {
        LogInfoStream(0, 0, 0) << "Hostname:" << hostName << " IP:" << ipAddress;  //
    }
    // 初始化 oatpp 环境
    oatpp::base::Environment::init();
    std::string ip = "0.0.0.0";
    // 运行应用
    run(ip, 8888);

    // 销毁 oatpp 环境
    oatpp::base::Environment::destroy();
    LogInfoStream(0, 0, 0) << "Destroy oatpp environment";  //

    return 0;
}