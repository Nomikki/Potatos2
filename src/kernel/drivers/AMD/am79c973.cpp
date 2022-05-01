#include <drivers/AMD/am79c973.hpp>
#include <stdio.h>

using namespace os;
using namespace os::communication;
using namespace os::driver;

// RAW DATA HANDLER

RawDataHandler::RawDataHandler(am79c973 *backend)
{

  this->backend = backend;
  backend->SetHandler(this);
}

RawDataHandler::~RawDataHandler()
{
  backend->SetHandler(0);
}

bool RawDataHandler::OnRawDataReceived(uint8_t *buffer, uint32_t size)
{
  return false;
}

void RawDataHandler::Send(uint8_t *buffer, uint32_t size)
{
  backend->Send(buffer, size);
}

// AM79c973

am79c973::am79c973(PCIDeviceDescriptor *device, InterruptManager *interrupts)
    : Driver(),
      InterruptHandler(device->interrupt + 0x20 /*offset*/, interrupts),
      MacAddress0Port(device->portBase),
      MacAddress2Port(device->portBase + 0x02),
      MacAddress4Port(device->portBase + 0x04),
      registerDataPort(device->portBase + 0x10),
      registerAddressPort(device->portBase + 0x12),
      resetPort(device->portBase + 0x14),
      busControlRegisterDataPort(device->portBase + 0x016)
{

  currentSendBuffer = 0;
  currentRecvBuffer = 0;

  this->handler = 0;

  uint64_t MAC0 = MacAddress0Port.Read() % 256;
  uint64_t MAC1 = MacAddress0Port.Read() / 256;
  uint64_t MAC2 = MacAddress2Port.Read() % 256;
  uint64_t MAC3 = MacAddress2Port.Read() / 256;
  uint64_t MAC4 = MacAddress4Port.Read() % 256;
  uint64_t MAC5 = MacAddress4Port.Read() / 256;

  uint64_t MAC = MAC5 << 40 | MAC4 << 32 | MAC3 << 24 | MAC2 << 16 | MAC1 << 8 | MAC0;

  uint8_t M0 = MAC & 0xFF;
  uint8_t M1 = (MAC >> 8) & 0xFF;
  uint8_t M2 = (MAC >> 16) & 0xFF;
  uint8_t M3 = (MAC >> 24) & 0xFF;
  uint8_t M4 = (MAC >> 32) & 0xFF;
  uint8_t M5 = (MAC >> 40) & 0xFF;

  printf("+ MAC ADDR: %x:%x:%x:%x:%x:%x\n", M0, M1, M2, M3, M4, M5);

  // set device to 32 bit mode
  registerAddressPort.Write(20);
  busControlRegisterDataPort.Write(0x102);

  // stop reset
  registerAddressPort.Write(0);
  registerDataPort.Write(0x04);

  // set init block
  initBlock.mode = 0x0000; // promiscuous mode = false
  initBlock.reserdved1 = 0;
  initBlock.numSendBuffers = 3; // 8 buffers
  initBlock.reserved2 = 0;
  initBlock.numRecvBuffers = 3;
  initBlock.physicalAddress = MAC;
  initBlock.reserved3 = 0;
  initBlock.logicalAddress = 0;

  sendBufferDescr = (BufferDescriptor *)((((uint32_t)&sendBufferDescMemory[0]) + 15) & ~((uint32_t)0xF));
  initBlock.sendBufferDescrAddress = (uint32_t)sendBufferDescr;

  recvBufferDescr = (BufferDescriptor *)((((uint32_t)&recvBufferDescMemory[0]) + 15) & ~((uint32_t)0xF));
  initBlock.recvBufferDescrAddress = (uint32_t)recvBufferDescr;
  for (uint8_t i = 0; i < 8; i++)
  {
    sendBufferDescr[i].address = (((uint32_t)&sendBuffers[i]) + 15) & ~(uint32_t)0xF;
    sendBufferDescr[i].flags = 0x7FF | 0xF000;
    sendBufferDescr[i].flags2 = 0;
    sendBufferDescr[i].available = 0;

    recvBufferDescr[i].address = (((uint32_t)&recvBuffers[i]) + 15) & ~(uint32_t)0xF;
    recvBufferDescr[i].flags = 0xF7FF | 0x80000000;
    recvBufferDescr[i].flags2 = 0;
    sendBufferDescr[i].available = 0;
  }

  registerAddressPort.Write(1);
  registerDataPort.Write((uint32_t)(&initBlock) & 0xFFFF);
  registerAddressPort.Write(2);
  registerDataPort.Write(((uint32_t)(&initBlock) >> 16) & 0xFFFF);
}

am79c973::~am79c973()
{
}

void am79c973::Activate()
{
  // printf("Activate AMD am79c973\n");
  // enable interrupts
  registerAddressPort.Write(0);
  registerDataPort.Write(0x41);

  registerAddressPort.Write(4);
  uint32_t temp = registerDataPort.Read();
  registerAddressPort.Write(4);
  registerDataPort.Write(temp | 0xC00);

  registerAddressPort.Write(0);
  registerDataPort.Write(0x42);
}

int am79c973::Reset()
{
  resetPort.Read();
  resetPort.Write(0);
  return 10; // wait 10ms
}

uint32_t am79c973::HandleInterrupt(uint32_t esp)
{
  // printf("Interrupt from AMD am79c973\n");

  registerAddressPort.Write(0);
  uint32_t temp = registerDataPort.Read();

  if ((temp & 0x8000) == 0x8000)
    printf("General error\n");
  if ((temp & 0x2000) == 0x2000)
    printf("Collision error\n");
  if ((temp & 0x1000) == 0x1000)
    printf("Missed frame\n");
  if ((temp & 0x0800) == 0x0800)
    printf("Memory error\n");

  if ((temp & 0x400) == 0x400)
    Receive();

  if ((temp & 0x200) == 0x0200)
  {
    // printf("Data sent\n");
  }

  // ACK
  registerAddressPort.Write(0);
  registerDataPort.Write(temp);

  if ((temp & 0x100) == 0x0100)
    printf("Init done\n");

  // printf("%X\n", temp);
  return esp;
}

void am79c973::Send(uint8_t *buffer, int size)
{
  int sendDescriptor = currentSendBuffer;
  currentSendBuffer = (currentSendBuffer + 1) % 8;

  if (size > 1518)
    size = 1518;

  // copy data to sendbuffer
  for (uint8_t *src = buffer + size - 1,
               *dst = (uint8_t *)(sendBufferDescr[sendDescriptor].address + size - 1);
       src >= buffer; src--, dst--)
  {
    *dst = *src;
  }

  /*
printf("Send:\n");
for (int i = 0; i < size; i++)
      printf("%x ", buffer[i]);
    printf("\n");
    */

  sendBufferDescr[sendDescriptor].available = 0;
  sendBufferDescr[sendDescriptor].flags2 = 0; // clear error messages
  sendBufferDescr[sendDescriptor].flags = 0x8300F000 | ((uint16_t)((-size) & 0xFFF));

  registerAddressPort.Write(0);
  registerDataPort.Write(0x48); // send command
}

void am79c973::Receive()
{
  // printf("Data received\n");
  for (; (recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0; currentRecvBuffer = (currentRecvBuffer + 1) % 8)
  {
    if (!(recvBufferDescr[currentRecvBuffer].flags & 0x40000000)                  // check error bit
        && (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000) // check start of packet (stp) and end of packet (enp) bits
    {                                                                             // check page 184
      uint32_t size = recvBufferDescr[currentRecvBuffer].flags & 0xFFF;
      if (size > 64)
        size -= 4; // remove checksum

      uint8_t *buffer = (uint8_t *)(recvBufferDescr[currentRecvBuffer].address);

      /*
    size = 64;
    for (int i = 0; i < size; i++)
      printf("%x ", buffer[i]);
    printf("\n");
    */
      /*
      for (int i = 0; i < size; i++)
        printf("%c ", buffer[i]);
        */

      if (handler != 0)
        if (handler->OnRawDataReceived(buffer, size))
          Send(buffer, size);

      // tell device we are ready with this data
      recvBufferDescr[currentRecvBuffer].flags2 = 0;
      recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;
    }
  }
}

void am79c973::SetHandler(RawDataHandler *handler)
{
  this->handler = handler;
}

uint64_t am79c973::GetMACAddress()
{
  return initBlock.physicalAddress;
}

void am79c973::SetIPAddress(uint32_t ip)
{
  initBlock.logicalAddress = ip;
}

uint32_t am79c973::GetIPAddress()
{
  return initBlock.logicalAddress;
}
