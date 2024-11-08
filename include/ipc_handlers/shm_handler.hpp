#ifndef IPC_HANDLERS_SHM_HANDLER_HPP
#define IPC_HANDLERS_SHM_HANDLER_HPP

#include <optional>
#include <expected>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstddef>
#include <memory>
#include <iostream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "timespec_chrono_conversions/timespec_chrono_conversions.hpp"

namespace shm_handler
{

enum class Mode
{
  CREATE,
  OPEN
};

enum class Error : int
{
  INVALID_SEGMENT_NAME,
  SHM_OPEN_FAILED,
  FTRUNCATE_FAILED,
  MMAP_FAILED,
  SEM_OPEN_FAILED
};

const std::unordered_map<Error, std::string_view> ErrorMap = {
  { Error::INVALID_SEGMENT_NAME, "Invalid segment name" },
  { Error::SHM_OPEN_FAILED, "Failed to open shared memory" },
  { Error::FTRUNCATE_FAILED, "Failed to truncate shared memory" },
  { Error::MMAP_FAILED, "Failed to map shared memory" },
  { Error::SEM_OPEN_FAILED, "Failed to open semaphore" }
};

template <class Data, std::size_t NumData = 1>
class SharedMemoryHandler
{
private:
  SharedMemoryHandler() : m_DataPtr{ nullptr }, m_SyncSemaphore{ nullptr }
  {
  }

public:
  SharedMemoryHandler(SharedMemoryHandler&& other) noexcept
    : m_Mode(other.m_Mode)
    , m_SegmentName(std::move(other.m_SegmentName))
    , m_SemaphoreName(std::move(other.m_SemaphoreName))
    , m_SharedMemoryFileDescriptor(other.m_SharedMemoryFileDescriptor)
    , m_SyncSemaphore(other.m_SyncSemaphore)
    , m_DataSize(other.m_DataSize)
    , m_DataPtr(other.m_DataPtr)
    , m_DataShPtr(std::move(other.m_DataShPtr))
    , m_LockWaitTimeoutSecs(other.m_LockWaitTimeoutSecs)
  {
    other.m_SharedMemoryFileDescriptor = -1;
    other.m_SyncSemaphore = nullptr;
    other.m_DataPtr = nullptr;
  }

  SharedMemoryHandler& operator=(SharedMemoryHandler&& other) noexcept
  {
    if (this != &other)
    {
      if (m_DataPtr != nullptr)
      {
        munmap(m_DataPtr, m_DataSize);
      }
      if (m_SyncSemaphore != nullptr)
      {
        sem_close(m_SyncSemaphore);
      }
      if (m_Mode == Mode::CREATE)
      {
        shm_unlink(m_SegmentName.data());
        sem_unlink(m_SemaphoreName.data());
      }

      close(m_SharedMemoryFileDescriptor);

      m_Mode = other.m_Mode;
      m_SegmentName = std::move(other.m_SegmentName);
      m_SemaphoreName = std::move(other.m_SemaphoreName);
      m_SharedMemoryFileDescriptor = other.m_SharedMemoryFileDescriptor;
      m_SyncSemaphore = other.m_SyncSemaphore;
      m_DataSize = other.m_DataSize;
      m_DataPtr = other.m_DataPtr;
      m_DataShPtr = std::move(other.m_DataShPtr);
      m_LockWaitTimeoutSecs = other.m_LockWaitTimeoutSecs;

      other.m_SharedMemoryFileDescriptor = -1;
      other.m_SyncSemaphore = nullptr;
      other.m_DataPtr = nullptr;
    }
    return *this;
  }
  ~SharedMemoryHandler()
  {
    if (m_DataPtr != nullptr)
    {
      munmap(m_DataPtr, m_DataSize);
    }
    if (m_SyncSemaphore != nullptr)
    {
      sem_close(m_SyncSemaphore);
    }
    if (m_Mode == Mode::CREATE)
    {
      shm_unlink(m_SegmentName.data());
      sem_unlink(m_SemaphoreName.data());
    }

    close(m_SharedMemoryFileDescriptor);
  }

  static std::expected<SharedMemoryHandler<Data, NumData>, Error>
  create(std::string_view segment_name, std::string_view semaphore_name = std::string_view(), Mode mode = Mode::CREATE)
  {
    static SharedMemoryHandler<Data, NumData> shm_handler;
    shm_handler.m_Mode = mode;
    if (segment_name.empty())
    {
      return std::unexpected(Error::INVALID_SEGMENT_NAME);
    }
    if (semaphore_name.empty())
    {
      semaphore_name = std::string(segment_name.data()) + "_sem";
    }
    shm_handler.m_SegmentName = segment_name;
    shm_handler.m_SemaphoreName = semaphore_name;
    std::size_t data_size = sizeof(Data) * NumData;
    shm_handler.m_DataSize = data_size;
    if (mode == Mode::CREATE)
    {
      shm_handler.m_SharedMemoryFileDescriptor = shm_open(segment_name.data(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
      if (shm_handler.m_SharedMemoryFileDescriptor == -1)
      {
        return std::unexpected(Error::SHM_OPEN_FAILED);
      }

      if (ftruncate(shm_handler.m_SharedMemoryFileDescriptor, data_size) != 0)
      {
        return std::unexpected(Error::FTRUNCATE_FAILED);
      }
    }
    else
    {
      shm_handler.m_SharedMemoryFileDescriptor = shm_open(segment_name.data(), O_RDWR, S_IRUSR | S_IWUSR);
      if (shm_handler.m_SharedMemoryFileDescriptor == -1)
      {
        return std::unexpected(Error::SHM_OPEN_FAILED);
      }
    }

    shm_handler.m_DataPtr = (Data*)mmap(NULL, shm_handler.m_DataSize, PROT_READ | PROT_WRITE, MAP_SHARED,
                                        shm_handler.m_SharedMemoryFileDescriptor, 0);
    if (shm_handler.m_DataPtr == MAP_FAILED)
    {
      return std::unexpected(Error::MMAP_FAILED);
    }

    if (mode == Mode::CREATE)
    {
      shm_handler.m_SyncSemaphore = sem_open(shm_handler.m_SemaphoreName.data(), O_CREAT, S_IRUSR | S_IWUSR, 1);
    }
    else
    {
      shm_handler.m_SyncSemaphore = sem_open(shm_handler.m_SemaphoreName.data(), 0);
    }

    if (shm_handler.m_SyncSemaphore == SEM_FAILED)
    {
      return std::unexpected(Error::SEM_OPEN_FAILED);
    }

    return std::move(shm_handler);
  }

  Data* getDataPtr()
  {
    return m_DataPtr;
  }

  /**
   * @brief Tries to lock the semaphore, doesnt block if the semaphore is already locked by another process
   *
   * @return true if the semaphore was locked successfully
   * @return false if the semaphore was not locked
   */
  bool tryLock()
  {
    return sem_trywait(m_SyncSemaphore) == 0;
  }

  /**
   * @brief Tries to lock the semaphore, blocks if the semaphore is already locked by another process
   *
   * @return true
   * @return false
   */
  bool lock()
  {
    return sem_wait(m_SyncSemaphore) == 0;
  }

  template <typename Rep, typename Period>
  bool timedLock(std::chrono::duration<Rep, Period> lock_wait_timeout = std::chrono::milliseconds(1))
  {
    struct timespec timeout;
    timespec_from_duration(timeout, lock_wait_timeout);
    return sem_timedwait(m_SyncSemaphore, &timeout) == 0;
  }

  /**
   * @brief Tries to unlock the semaphore
   *
   * @return true
   * @return false
   */
  bool unlock()
  {
    return sem_post(m_SyncSemaphore) == 0;
  }

private:
  Mode m_Mode;

  std::string_view m_SegmentName;
  std::string_view m_SemaphoreName;

  int m_SharedMemoryFileDescriptor;
  sem_t* m_SyncSemaphore;

  std::size_t m_DataSize;
  Data* m_DataPtr;
  std::shared_ptr<Data> m_DataShPtr;

  std::chrono::duration<double, std::ratio<1>> m_LockWaitTimeoutSecs;
};

}  // namespace shm_handler
#endif  // IPC_HANDLERS_SHM_HANDLER_HPP