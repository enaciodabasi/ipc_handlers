#ifndef IPC_HANDLERS_MQ_HANDLER_HPP
#define IPC_HANDLERS_MQ_HANDLER_HPP

#include <mqueue.h>
#include <memory>
#include <expected>

#include "ipc_handlers/common.hpp"

namespace ipc_handlers
{

namespace mq_handler
{

  enum class Error
  {
    MQ_OPEN_FAILED,
    MQ_GETATTR_FAILED
  };

template <class DataType>
class MessageQueueHandler
{
private:

  MessageQueueHandler() = default;

public:

  using SharedPtr = std::shared_ptr<MessageQueueHandler<DataType>>;
  using UniquePtr = std::unique_ptr<MessageQueueHandler<DataType>>;

  MessageQueueHandler(const MessageQueueHandler&) = delete;

  MessageQueueHandler& operator=(const MessageQueueHandler&) = delete;

  MessageQueueHandler(MessageQueueHandler&&)
  {

  }

  MessageQueueHandler& operator=(MessageQueueHandler&&)
  {
    
  }

  ~MessageQueueHandler()
  {

  }

  std::expected<MessageQueueHandler<DataType>, Error> create(std::string_view queue_name, mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = sizeof(DataType)}, Mode mode = Mode::CREATE)
  {
    
    MessageQueueHandler<DataType> mqHandler;
    int oflag = O_RDWR;
    if (mode == Mode::CREATE)
    {
      oflag = O_CREAT | O_RDWR;
    }
    
    if(mode == Mode::CREATE){
      mqHandler.m_MessageQueueAttributes = attr;
      mqHandler.m_MessageQueueDescriptor = mq_open(queue_name.data(), oflag, 0666, &mqHandler.m_MessageQueueAttributes);
    }
    else{
      mqHandler.m_MessageQueueDescriptor = mq_open(queue_name.data(), oflag);
      if(mq_getattr(mqHandler.m_MessageQueueDescriptor, &mqHandler.m_MessageQueueAttributes) == -1)
      {
        return std::unexpected(Error::MQ_GETATTR_FAILED);
      }
      
    }

    if(mqHandler.m_MessageQueueDescriptor == -1)
    {
      return std::unexpected(Error::MQ_OPEN_FAILED);
    }
    
  }

private:

  mqd_t m_MessageQueueDescriptor;
  mq_attr m_MessageQueueAttributes;

};
}  // namespace mq_handler
}  // namespace ipc_handlers

#endif  // IPC_HANDLERS_MQ_HANDLER_HPP