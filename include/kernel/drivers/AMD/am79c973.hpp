#ifndef __AM79C973_HPP
#define __AM79C973_HPP

/*
  This device can have multiple send and receive buffers
  Check this pdf: https://www.amd.com/system/files/TechDocs/21510.pdf
  or https://www.amd.com/system/files/TechDocs/20550.pdf
  Also https://wiki.osdev.org/AMD_PCNET

*/

#include <stdint.h>
#include <communication/ports.hpp>
#include <communication/idt.hpp>
#include <drivers/driver.hpp>
#include <communication/pci.hpp>

namespace os
{
  namespace driver
  {
    // almost same concept as interrupt handler
    // but this is middle of hardware and software protocols
    // thats why there is backend, because we want to communicate with 
    // with hardware also

    class am79c973;

    class RawDataHandler
    {
    protected:
      am79c973 *backend;

    public:
      RawDataHandler(am79c973 *backend);
      ~RawDataHandler();

      virtual bool OnRawDataReceived(uint8_t* buffer, uint32_t size);
      void Send(uint8_t* buffer, uint32_t size);

    };

    class am79c973 : public os::driver::Driver, public os::communication::InterruptHandler
    {
      struct InitializationBlock
      {
        uint16_t mode;
        unsigned reserdved1 : 4;
        unsigned numSendBuffers : 4;
        unsigned reserved2 : 4;
        unsigned numRecvBuffers : 4;
        uint64_t physicalAddress : 48; // 48 bits actually
        uint16_t reserved3;
        uint64_t logicalAddress;
        uint32_t recvBufferDescrAddress;
        uint32_t sendBufferDescrAddress;
      } __attribute__((packed));

      struct BufferDescriptor
      {
        uint32_t address;
        uint32_t flags;
        uint32_t flags2;
        uint32_t available;
      } __attribute__((packed));

      os::communication::Port16Bit MacAddress0Port;
      os::communication::Port16Bit MacAddress2Port;
      os::communication::Port16Bit MacAddress4Port;
      os::communication::Port16Bit registerDataPort;
      os::communication::Port16Bit registerAddressPort;
      os::communication::Port16Bit resetPort;
      os::communication::Port16Bit busControlRegisterDataPort;

      /*
        Main purpose of init block is to hold a pointer to the array of buffer descriptors,
        which hold the pointers to the buffers
      */
      InitializationBlock initBlock;

      // 8 buffers, size of 2 kilobytes per buffer and 15 bytes for alignment
      BufferDescriptor *sendBufferDescr;
      uint8_t sendBufferDescMemory[2048 + 15];
      uint8_t sendBuffers[2 * 1024 + 15][8];
      // Tells which buffer is currently active
      uint8_t currentSendBuffer;

      BufferDescriptor *recvBufferDescr;
      uint8_t recvBufferDescMemory[2048 + 15];
      uint8_t recvBuffers[2 * 1024 + 15][8];
      // Tells which buffer is currently active
      uint8_t currentRecvBuffer;

      RawDataHandler *handler;

    public:
      am79c973(os::communication::PCIDeviceDescriptor *device, os::communication::InterruptManager *interrupts);
      ~am79c973();

      // Inherit Activate and Reset from Driver class
      void Activate();
      int Reset();

      // Inherit HandleInterrupt from InterruptHandler class
      uint32_t HandleInterrupt(uint32_t esp);

      void Send(uint8_t *buffer, int size);
      void Receive();

      void SetHandler(RawDataHandler *handler);
      uint64_t GetMACAddress();
    };
  }
}

#endif // __AM79C973_HPP