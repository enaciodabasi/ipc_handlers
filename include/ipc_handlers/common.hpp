#ifndef IPC_HANDLERS_COMMON_HPP
#define IPC_HANDLERS_COMMON_HPP

/**

 class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual std::vector<uint8_t> serialize() const = 0;
    virtual void deserialize(const std::vector<uint8_t>& data) = 0;
};

// Type traits
template<typename T>
struct is_pod_serializable : std::is_trivially_copyable<T> {};

template<typename T>
struct is_custom_serializable {
    static constexpr bool value = std::is_base_of_v<ISerializable, T>;
};

// Add these methods to TcpClient class
template<typename T>
std::expected<T, ErrorCode> send_serialized(const T& data) const {
    if constexpr (is_pod_serializable<T>::value) {
        return send(data);
    }
    else if constexpr (is_custom_serializable<T>::value) {
        auto serialized = data.serialize();
        size_t size = serialized.size();
        
        // Send size first
        auto sizeResult = send(size);
        if (!sizeResult) {
            return std::unexpected(sizeResult.error());
        }
        
        // Send data
        if (send(m_SocketFileDescriptor, serialized.data(), size, 0) != size) {
            return std::unexpected(ErrorCode::SOCKET_SEND_FAILED);
        }
        
        return data;
    }
    else {
        static_assert(always_false<T>::value, "Type must be POD or implement ISerializable");
    }
}

template<typename T>
std::expected<T, ErrorCode> receive_serialized() const {
    if constexpr (is_pod_serializable<T>::value) {
        T data;
        if (recv(m_SocketFileDescriptor, &data, sizeof(T), 0) != sizeof(T)) {
            return std::unexpected(ErrorCode::SOCKET_RECEIVE_FAILED);
        }
        return data;
    }
    else if constexpr (is_custom_serializable<T>::value) {
        // Receive size first
        size_t size;
        if (recv(m_SocketFileDescriptor, &size, sizeof(size_t), 0) != sizeof(size_t)) {
            return std::unexpected(ErrorCode::SOCKET_RECEIVE_FAILED);
        }
        
        // Receive data
        std::vector<uint8_t> buffer(size);
        if (recv(m_SocketFileDescriptor, buffer.data(), size, 0) != size) {
            return std::unexpected(ErrorCode::SOCKET_RECEIVE_FAILED);
        }
        
        T data;
        data.deserialize(buffer);
        return data;
    }
    else {
        static_assert(always_false<T>::value, "Type must be POD or implement ISerializable");
    }
}

 */

enum class Mode
{
  CREATE,
  OPEN
};

template<typename E>
constexpr auto to_integral_value(E e) -> typename std::underlying_type<E>::type
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}


#endif // IPC_HANDLERS_COMMON_HPP