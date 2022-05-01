#include <network/arp.hpp>

using namespace os;
using namespace os::driver;
using namespace os::net;

AddressResolutionProtocol::AddressResolutionProtocol(EthernetFrameProvider *backend)
    : EthernetFrameHandler(backend, 0x806)
{
  numCacheEntries = 0;
}

AddressResolutionProtocol::~AddressResolutionProtocol()
{
}

bool AddressResolutionProtocol::OnEthernetFrameReceivedData(uint8_t *ethernetFramePayload, uint32_t size)
{
  if (size < sizeof(AddressResolutionProtocolMessage))
    return false;

  AddressResolutionProtocolMessage *arp = (AddressResolutionProtocolMessage *)ethernetFramePayload;
  if (arp->hardwareType == 0x0100)
  {
    // if we understand this message and its for us, handle it.
    if (arp->protocol == 0x0008 && arp->hardwareAddressSize == 6 && arp->protocolAddressSize == 4 && arp->dstIP == backend->GetIPAddress())
    {
      switch (arp->command)
      {
      case 0x0100: // request
        arp->command = 0x0200;
        arp->dstIP = arp->srcIP;
        arp->dstMAC = arp->srcMAC;
        arp->srcIP = backend->GetIPAddress();
        arp->srcMAC = backend->GetMACAddress();

        return true;
        break;
      case 0x0200: // response to message that we made
        AddToCache(arp->srcIP, arp->srcMAC);
        break;

      default:
        break;
      }
    }
  }

  // default behavior is: do not sent anything back
  return false;
}

void AddressResolutionProtocol::AddToCache(uint32_t ip, uint64_t mac)
{
  IPcache[numCacheEntries] = ip;
  MACcache[numCacheEntries] = mac;
  numCacheEntries = (numCacheEntries + 1) % sizeOfCache;
}

uint64_t AddressResolutionProtocol::RequestMACAddress(uint32_t IP_BE)
{
  AddressResolutionProtocolMessage arp;
  arp.hardwareType = 0x0100;   // ethernet
  arp.protocol = 0x0008;       // ipv4
  arp.hardwareAddressSize = 6; // mac address
  arp.protocolAddressSize = 4; // ipv4
  arp.command = 0x0100;        // 1 = request

  arp.srcMAC = backend->GetMACAddress();
  arp.srcIP = backend->GetIPAddress();
  arp.dstMAC = 0xFFFFFFFFFFFF; // broadcast address
  arp.dstIP = IP_BE;

  this->Send(arp.dstMAC, (uint8_t *)&arp, sizeof(AddressResolutionProtocolMessage));
}

uint64_t AddressResolutionProtocol::GetMACFromCache(uint32_t IP_BE)
{
  for (int i = 0; i < sizeOfCache; i++)
    if (IPcache[i] == IP_BE)
      return MACcache[i];

  return 0xFFFFFFFFFFFF; // broadcast address
}

uint64_t AddressResolutionProtocol::Resolve(uint32_t IP_BE)
{
  uint8_t ip[4];
  ip[3] = (IP_BE >> 24) & 0xFF;
  ip[2] = (IP_BE >> 16) & 0xFF;
  ip[1] = (IP_BE >> 8) & 0xFF;
  ip[0] = (IP_BE) & 0xFF;

  printf("ARP resolving %i.%i.%i.%i... ", ip[0], ip[1], ip[2], ip[3]);
  uint64_t result = GetMACFromCache(IP_BE);
  if (result == 0xFFFFFFFFFFFF)
    RequestMACAddress(IP_BE);

  while (result == 0xFFFFFFFFFFFF)
    result = GetMACFromCache(IP_BE);

  printf("solved!\n");
  return result;
}

void AddressResolutionProtocol::PrintCache() {
  for (int i = 0; i < numCacheEntries; i++)
  {
    uint8_t ip[4];
    uint8_t mac[6];
    ip[3] = (IPcache[i] >> 24) & 0xFF;
    ip[2] = (IPcache[i] >> 16) & 0xFF;
    ip[1] = (IPcache[i] >> 8) & 0xFF;
    ip[0] = (IPcache[i]) & 0xFF;

    mac[5] = (MACcache[i] >> 40) & 0xFF;
    mac[4] = (MACcache[i] >> 32) & 0xFF;
    mac[3] = (MACcache[i] >> 24) & 0xFF;
    mac[2] = (MACcache[i] >> 16) & 0xFF;
    mac[1] = (MACcache[i] >> 8) & 0xFF;
    mac[0] = (MACcache[i]) & 0xFF;


    printf("ip: %u.%u.%u.%u -> %x:%x:%x:%x:%x:%x\n", ip[0], ip[1], ip[2], ip[3], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }
}