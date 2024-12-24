/**
 * @file tcp_client_template.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef IPC_HANDLERS__TCP_CLIENT_TEMPLATE_HPP_
#define IPC_HANDLERS__TCP_CLIENT_TEMPLATE_HPP_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include <string_view>
#include <string>
#include <expected>

#include <ipc_handlers/common.hpp>

namespace tcp_client {


enum class ErrorCode
{
  SUCCESS,
  SOCKET_SEARCH_FAILED,
  SOCKET_CREATION_FAILED,
  SOCKET_CONNECTION_FAILED,
  SOCKET_SEND_FAILED,
  SOCKET_RECEIVE_FAILED
};

class TcpClient
{
  private:

  TcpClient() = default;

  public:

  enum class ProtocolType : int
  {
    TCP = 6,
    UDP = 22
  };

  static std::expected<TcpClient, ErrorCode> create(
    std::string_view serverAddress,
    std::string_view serverPort,
    ProtocolType protocolType = ProtocolType::TCP
  
  );

  template<typename T>
  ErrorCode sendData(const T& data) const
  {
    if(!isConnected())
    {
      return ErrorCode::SOCKET_CONNECTION_FAILED;
    }
    
    if(send(m_SocketFileDescriptor, &data, sizeof(T), 0) != sizeof(T))
    {
      return ErrorCode::SOCKET_SEND_FAILED;
    }

    return ErrorCode::SUCCESS;
  }

  template<typename T>
  std::expected<T, ErrorCode> receiveData() const
  {
    if(!isConnected())
    {
      return std::unexpected(ErrorCode::SOCKET_CONNECTION_FAILED);
    }

    T data;
    if(recv(m_SocketFileDescriptor, &data, sizeof(T), 0) != sizeof(T))
    {
      return std::unexpected(ErrorCode::SOCKET_RECEIVE_FAILED);
    }

    return data;
  }

  bool isConnected() const
  {
    return m_SocketFileDescriptor != -1;
  }

  private:
 
  int m_SocketFileDescriptor;

  sockaddr_in m_ServerAddress;
  

};


} // namespace ipc_handlers

#endif // IPC_HANDLERS__TCP_CLIENT_TEMPLATE_HPP_