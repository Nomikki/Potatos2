#ifndef __ETHERFRAME_HPP
#define __ETHERFRAME_HPP

#include <stdint.h>
#include <drivers/AMD/am79c973.hpp>
#include <memory/memorymanagement.hpp>

namespace os
{
  namespace net
  {

    struct EthernetFrameHeader
    {
      // because mac addresses have only 6 bytes, we use this ': 48'
      uint64_t dstMac_BE : 48;
      uint64_t srcMac_BE : 48;
      uint16_t ethernetType_BE;
    } __attribute__((packed));

    typedef uint32_t EthernetFrameFooter;

    class EthernetFrameProvider;

    class EthernetFrameHandler
    {
    protected:
      EthernetFrameProvider *backend;
      uint16_t ethernetType_BE;

    public:
      EthernetFrameHandler(EthernetFrameProvider *backend, uint16_t ethernetType);
      ~EthernetFrameHandler();

       virtual bool OnEthernetFrameReceivedData(uint8_t *ethernetFramePayload, uint32_t size);
       void Send(uint64_t dstMAC_BE, uint8_t *data, uint32_t size);
    };

    class EthernetFrameProvider : public os::driver::RawDataHandler
    {
      friend class EthernetFrameHandler;

    protected:
      EthernetFrameHandler *handlers[65535];

    public:
      EthernetFrameProvider(os::driver::am79c973 *backend);
      ~EthernetFrameProvider();

      bool OnRawDataReceived(uint8_t *buffer, uint32_t size);
      void Send(uint64_t dstMAC_BE, uint16_t etherType_BE, uint8_t *buffer, uint32_t size);

      uint32_t GetIPAddress();
      uint64_t GetMACAddress();
    };
  }
}

#endif