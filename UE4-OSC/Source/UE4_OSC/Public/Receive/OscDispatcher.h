#pragma once

#include "UObject/Object.h"
#include "Common/OscDataElemStruct.h"

#include "Interfaces/IPv4/IPv4Address.h"
#include "Containers/CircularQueue.h"
#include "Common/UdpSocketReceiver.h"

#include <utility>
#include <tuple>

#include "OscDispatcher.generated.h"

struct IOscReceiverInterface;
class FSocket;
class FUdpSocketReceiver;
struct FIPv4Endpoint;

/// Singleton that dispatches the OSC messages to listeners.
UCLASS()
class UE4_OSC_API UOscDispatcher : public UObject
{
    GENERATED_BODY()

    friend class FOscModule;

    void Listen(FIPv4Address address, uint32_t port, FIPv4Address multicastAddress, bool multicastLoopback);
    void Stop();

public:
    /// Default constructor
    UOscDispatcher();

    /// Hot reload constructor
    UOscDispatcher(FVTableHelper & helper);

    /// Get the unique instance
    ///
    /// The instance is the UObject default instance.
    static UOscDispatcher * Get();

    /// Add the receiver in the listeners list
    void RegisterReceiver(IOscReceiverInterface * receiver);

    /// Remove the receiver in the listeners list
    void UnregisterReceiver(IOscReceiverInterface * receiver);

private:
    void Callback(const FArrayReaderPtr& data, const FIPv4Endpoint&);

    void CallbackMainThread();

    void BeginDestroy() override;
    
private:
    TArray<IOscReceiverInterface *> _receivers;
    struct SocketReceiver
    {
        FSocket * Socket;
        FUdpSocketReceiver * Receiver;
    };
    TArray<SocketReceiver> _socketReceivers;
    TQueue<std::tuple<FName, TArray<FOscDataElemStruct>, FIPv4Address>, EQueueMode::Mpsc> _pendingMessages;  // supports multiple-producers
    int32 _taskSpawned;

    /// Protects _receivers
    FCriticalSection _receiversMutex;
};
