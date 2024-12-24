#include "ipc_handlers/tcp_client_template.hpp"

namespace tcp_client
{

std::expected<TcpClient, ErrorCode> TcpClient::create(
  std::string_view serverAddress,
  std::string_view serverPort,
  ProtocolType protocolType
)
{
  
  static TcpClient client;

  addrinfo addrCriteria;
  memset(&addrCriteria, 0, sizeof(addrCriteria));
  addrCriteria.ai_family = AF_UNSPEC;
  addrCriteria.ai_socktype = SOCK_STREAM;
  addrCriteria.ai_protocol = to_integral_value(protocolType);

  addrinfo *servAddr;
  if(getaddrinfo(serverAddress.data(), serverPort.data(), &addrCriteria, &servAddr) !=0)
  {
    
    freeaddrinfo(servAddr);
    return std::unexpected(ErrorCode::SOCKET_SEARCH_FAILED);
  }

  for(addrinfo* foundAddr = servAddr; foundAddr != nullptr; foundAddr = foundAddr->ai_next)
  {
    client.m_SocketFileDescriptor = socket(foundAddr->ai_family, foundAddr->ai_socktype, foundAddr->ai_protocol);
    if(client.m_SocketFileDescriptor < 0)
    {
      continue;
    }

    if(connect(client.m_SocketFileDescriptor, foundAddr->ai_addr, foundAddr->ai_addrlen) == 0)
    {
      break;
    }

    close(client.m_SocketFileDescriptor);
    client.m_SocketFileDescriptor = -1;
  }

  freeaddrinfo(servAddr);
  if(client.m_SocketFileDescriptor == -1)
  {
    return std::unexpected(ErrorCode::SOCKET_CREATION_FAILED);
  }

  freeaddrinfo(servAddr);


  return client;
}

} // namespace tcp_client