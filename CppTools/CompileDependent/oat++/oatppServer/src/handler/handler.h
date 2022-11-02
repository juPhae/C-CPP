// handler.h
#ifndef HANDLER_H
#define HANDLER_H

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/web/server/HttpRequestHandler.hpp"

#define O_UNUSED(x) (void)x;

// 自定义请求处理程序
class Handler : public oatpp::web::server::HttpRequestHandler {
   public:

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
    // 处理传入的请求，并返回响应
    std::shared_ptr<OutgoingResponse> handle(const std::shared_ptr<IncomingRequest>& request) override {
        O_UNUSED(request);

        return ResponseFactory::createResponse(Status::CODE_200, "Hello, World!");
    }
};


#endif  // HANDLER_H