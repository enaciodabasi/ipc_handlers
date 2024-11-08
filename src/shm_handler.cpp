/**
 * @file shm_handler.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "ipc_handlers/shm_handler.hpp"

namespace shm_handler {

  std::string_view getErrorString(Error error) {
    
    if(ErrorMap.find(error) == ErrorMap.end()) {
      return "Unknown error";
    }   
    
    return ErrorMap.at(error);
  };


} // namespace shm_handler