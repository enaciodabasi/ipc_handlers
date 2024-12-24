/**
 * @file shm_handler_test.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <iostream>
#include <signal.h>
#include <bits/this_thread_sleep.h>
#include <functional>

#include "ipc_handlers/shm_handler.hpp"
static bool run = true;

void signal_handler(int signal)
{
  run = false;
}

int main(int argc, char** argv)
{
  
  std::expected<shm_handler::SharedMemoryHandler<int, 1>, shm_handler::Error> result = shm_handler::SharedMemoryHandler<int, 1>::create("/TEST_SEGMENT", "/test_semaphore", Mode::CREATE);
  if(!result.has_value())
  {
    std::cout << "Could not create Shared Memory Handler: " << shm_handler::ErrorMap.at(result.error()) << std::endl;
    return 10;
  }
  
  shm_handler::SharedMemoryHandler<int, 1> handler = std::move(result.value());
  signal(SIGINT, signal_handler);

  

  while(run)
  {
    
    if(handler.lock()){
      std::cout << "Data: " << *handler.getDataPtr() << std::endl;
      
      handler.unlock();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  return 0;
}