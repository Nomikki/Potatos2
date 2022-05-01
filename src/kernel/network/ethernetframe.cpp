#include <network/ethernetframe.hpp>

using namespace os;
using namespace os::net;
using namespace os::driver;

/*
  HANDLER
*/

EthernetFrameHandler::EthernetFrameHandler(EthernetFrameProvider *backend, uint16_t ethernetType)
{

  this->backend = backend;
  this->ethernetType_BE = ((ethernetType & 0x00FF) << 8) | ((ethernetType & 0xFF00) >> 8);
  backend->handlers[ethernetType_BE] = this;
}

EthernetFrameHandler::~EthernetFrameHandler()
{
  backend->handlers[ethernetType_BE] = 0;
}

bool EthernetFrameHandler::OnEthernetFrameReceivedData(uint8_t *ethernetFramePayload, uint32_t size)
{

  return false;
}

void EthernetFrameHandler::Send(uint64_t dstMAC_BE, uint8_t *data, uint32_t size)
{
  backend->Send(dstMAC_BE, ethernetType_BE, data, size);
}

/*
  PROVIDER
*/

EthernetFrameProvider::EthernetFrameProvider(am79c973 *backend)
    : RawDataHandler(backend)
{

  for (uint32_t i = 0; i < 65535; i++)
    handlers[i] = 0;
}

EthernetFrameProvider::~EthernetFrameProvider()
{
}

bool EthernetFrameProvider::OnRawDataReceived(uint8_t *buffer, uint32_t size)
{
  // use buffer data as a EthernetFrameHeader
  EthernetFrameHeader *frame = (EthernetFrameHeader *)buffer;
  bool sendBack = false;

  const uint64_t broadcastMacAddress = 0xFFFFFFFFFFFF;

  if (frame->dstMac_BE == broadcastMacAddress || frame->dstMac_BE == backend->GetMACAddress())
  {
    if (handlers[frame->ethernetType_BE] != 0)
    {
      sendBack = handlers[frame->ethernetType_BE]->OnEthernetFrameReceivedData(buffer + sizeof(EthernetFrameHeader), size - sizeof(EthernetFrameHeader));
    }
  }

  if (sendBack)
  {
    frame->dstMac_BE = frame->srcMac_BE;
    frame->srcMac_BE = backend->GetMACAddress();
  }
  return sendBack;
}
void EthernetFrameProvider::Send(uint64_t dstMAC_BE, uint16_t etherType_BE, uint8_t *buffer, uint32_t size)
{
  uint8_t *buffer2 = (uint8_t *)os::memory::MemoryManager::activeMemoryManager->malloc(sizeof(EthernetFrameHeader) + size);
  EthernetFrameHeader *frame = (EthernetFrameHeader *)buffer2;

  frame->dstMac_BE = dstMAC_BE;
  frame->srcMac_BE = backend->GetMACAddress();
  frame->ethernetType_BE = etherType_BE;

  uint8_t *src = buffer;
  uint8_t *dst = buffer2 + sizeof(EthernetFrameHeader);
  for (uint32_t i = 0; i < size; i++)
  {
    dst[i] = src[i];
  }

  backend->Send(buffer2, size + sizeof(EthernetFrameHeader));
}

uint32_t EthernetFrameProvider::GetIPAddress()
{
  return backend->GetIPAddress();
}

uint64_t EthernetFrameProvider::GetMACAddress()
{
  return backend->GetMACAddress();
}