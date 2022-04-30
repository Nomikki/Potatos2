#ifndef __PCI_H__
#define __PCI_H__

/*
  PCI:llä on 8 väylää. Jokaisella väylällä voi olla 32 laitetta.
  Laitteella voi olla 8 funktiota.

  PCI -> bus N ---> Device 1 -->  function 1
               \__> Device 2  \_> ....
                \_> ....       \_>function 8
                 \> Device 32

  Eli tarvitaan 3 bittiä enkoodaamaan väylänumero, 5 bittiä enkoodaamaan laitenumero ja taas
                3 bittiä enkoodaamaan funktionumero.
                Yhteensä 3 + 8 + 3 = 14 bittiä

  Luodaan funktio jolla voidaan kysyä vaikkapa: mikä laite on osoitteessa 1 bus, 3 device
  ja se palauttaa kaksi uint16_t-arvoa, joista ensimmäinen on vendorID ja toinen deviceID.

  PCI controller kertoo myös classID:n ja subclassID:n. Ne on myös uint16_t-tyyppisiä.
  ClassID kertoo minkälainen laite kyseessä, vaikkapa multimedialaite ja subclassID voi kertoa
  että kyse on VGA-kontrollerista. Näin ei tarvitse jokaiselle raudalle tehdä erikseen ajureita,
  vaan voidaan käyttää standardoitua ajuria (vaikka se ei välttämättä olisi se tehokkain ratkaisu)

  Näillä tiedoilla voidaan asettaa oikeat ajurit käyttöön


*/

#include <stdint.h>
#include <communication/ports.hpp>
#include <communication/idt.hpp>
#include <drivers/driver.hpp>
#include <memory/memorymanagement.hpp>

namespace os::communication
{
#define PCI_DATAPORT 0xCFC
#define PCI_COMMANDPORT 0xCF8

#define PCI_BUSMASK 0xFF      // 0b11111111 8 bits up
#define PCI_DEVICEMASK 0x1F   // 0b00011111 5 bits up
#define PCI_FUNCTIONMASK 0x07 // 0b00000111 3 bits up
#define PCI_OFFSETMASK 0xFF   // 0b11111111 8 bits up

  enum BaseAddressRegisterType
  {
    MemoryMapping = 0,
    InputOutput = 1,
  };

  class BaseAddressRegister
  {
  public:
    bool prefetchable;
    uint8_t *address;
    BaseAddressRegisterType type;

    /*
    BaseAddressRegister();
    ~BaseAddressRegister();
    */
  };

  class PCIDeviceDescriptor
  {
  public:
    uint32_t portBase;
    uint32_t interrupt;

    uint16_t bus;
    uint16_t device;
    uint16_t function;

    uint16_t vendorID;
    uint16_t deviceID;

    uint8_t classID;
    uint8_t subclassID;
    uint8_t interfaceID;

    uint8_t revision;

    PCIDeviceDescriptor();
    ~PCIDeviceDescriptor();
  };

  class PCI
  {
    os::communication::Port32Bit dataPort;
    os::communication::Port32Bit commandPort;

    uint32_t inline encodeID(uint16_t b, uint16_t d, uint16_t f, uint16_t r)
    {
      return 0x1 << 31 | ((b & PCI_BUSMASK /*0b11111111*/) << 16) | ((d & PCI_DEVICEMASK /*0b00011111*/) << 11) | ((f & PCI_FUNCTIONMASK /*0b00000111*/) << 8) | ((r & PCI_OFFSETMASK /*0b11111100*/));
    };

  public:
    PCI();
    ~PCI();

    uint32_t Read(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset);
    void Write(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset, uint32_t value);
    bool DeviceHasFucntions(uint16_t busNumber, uint16_t deviceNumber);

    void SelectDrivers(os::driver::DriverManager *driverManager, os::communication::InterruptManager *interrupts);
    os::driver::Driver *GetDriver(PCIDeviceDescriptor device, os::communication::InterruptManager *interrupts);
    PCIDeviceDescriptor GetDeviceDescriptor(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber);
    BaseAddressRegister GetBaseAddressRegister(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint16_t bar);
  };
};

#endif // __PCI_H__