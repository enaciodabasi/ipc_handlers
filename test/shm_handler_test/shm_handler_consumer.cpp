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

std::function<void(int)> shutdown_handler;
void signal_handler(int signal)
{
  shutdown_handler(signal);
}

int main(int argc, char** argv)
{
  
  auto result = shm_handler::SharedMemoryHandler<int, 1>::create("/TEST_SEGMENT", "/test_semaphore", Mode::OPEN);
  if(!result.has_value())
  {
    std::cout << "Could not create Shared Memory Handler: " << shm_handler::ErrorMap.at(result.error()) << std::endl;
    return 10;
  }
  
  shm_handler::SharedMemoryHandler<int, 1> handler = std::move(result.value());

  bool run = true;
  shutdown_handler = [&run](int signal)
  {
    run = false;
  };
  signal(SIGINT, signal_handler);
  
  std::cout << "Entering loop" << std::endl;
  int i = 0;
  while(run)
  {
    if(handler.lock()){
      handler.getDataPtr()[0] = i;
      i += 1; 
      handler.unlock();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  return 0;
}