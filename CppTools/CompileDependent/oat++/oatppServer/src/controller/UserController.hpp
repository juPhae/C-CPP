
#ifndef UserController_hpp
#define UserController_hpp


#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * User REST controller.
 */
class UserController : public oatpp::web::server::api::ApiController {
public:
  UserController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
private:
public:

  static std::shared_ptr<UserController> createShared(
    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
  ){
    return std::make_shared<UserController>(objectMapper);
  }
  

  
  // ENDPOINT_INFO(getUsers) {
  //   info->summary = "get all stored users";

  //   // info->addResponse<oatpp::Object<UsersPageDto>>(Status::CODE_200, "application/json");
  //   // info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
  // }
  // ENDPOINT("GET", "users/offset/{offset}/limit/{limit}", getUsers,
  //          PATH(UInt32, offset),
  //          PATH(UInt32, limit))
  // {
  //   // return createDtoResponse(Status::CODE_200, );
  // }
  
  

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif /* UserController_hpp */
