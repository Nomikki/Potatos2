/*
  https://en.wikipedia.org/wiki/Address_Resolution_Protocol
*/

#ifndef __ARP_H__
#define __ARP_H__

#include <stdint.h>
#include <network/ethernetframe.hpp>
#include <stdio.h>

namespace os
{
  namespace net
  {

    struct AddressResolutionProtocolMessage
    {
      uint16_t hardwareType;
      uint16_t protocol;
      uint8_t hardwareAddressSize; // 6
      uint8_t protocolAddressSize; // 4
      uint16_t command;

      uint64_t srcMAC : 48;
      uint32_t srcIP;
      uint64_t dstMAC : 48;
      uint32_t dstIP;

    } __attribute__((packed));

    class AddressResolutionProtocol : public os::net::EthernetFrameHandler
    {
#define sizeOfCache 128
      uint32_t IPcache[sizeOfCache];
      uint64_t MACcache[sizeOfCache];
      int numCacheEntries;

    public:
      AddressResolutionProtocol(os::net::EthernetFrameProvider *backend);
      ~AddressResolutionProtocol();

      virtual bool OnEthernetFrameReceivedData(uint8_t *ethernetFramePayload, uint32_t size);
      uint64_t RequestMACAddress(uint32_t IP_BE);
      uint64_t GetMACFromCache(uint32_t IP_BE);

      void AddToCache(uint32_t ip, uint64_t mac);
      uint64_t Resolve(uint32_t IP_BE);
      void PrintCache();

    };
  }
}

#endif // __ARP_H__