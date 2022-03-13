#include <communication/pci.hpp>
#include <stdio.h>

using namespace os::communication;
using namespace os::driver;

PCIDeviceDescriptor::PCIDeviceDescriptor()
{
}

PCIDeviceDescriptor::~PCIDeviceDescriptor()
{
}

PCI::PCI() : dataPort(PCI_DATAPORT), commandPort(PCI_COMMANDPORT)
{
  printf("Install PCI\n");
}

PCI::~PCI()
{
}

uint32_t PCI::Read(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset)
{
  uint32_t id = encodeID(busNumber, deviceNumber, functionNumber, registerOffset);
  commandPort.Write(id);

  uint32_t result = dataPort.Read();
  return result >> (8 * (registerOffset % 4));
}

void PCI::Write(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset, uint32_t value)
{
  uint32_t id = encodeID(busNumber, deviceNumber, functionNumber, registerOffset);
  commandPort.Write(id);
  dataPort.Write(value);
}

bool PCI::DeviceHasFucntions(uint16_t busNumber, uint16_t deviceNumber)
{
  return Read(busNumber, deviceNumber, 0, 0x0E) & (1 << 7);
}

Driver *PCI::GetDriver(PCIDeviceDescriptor device, os::communication::InterruptManager *interrupts)
{
  switch (device.vendorID)
  {
  case 0x1022: // AMD
    switch (device.deviceID)
    {
    case 0x2000: // AM79c973
      break;
    }
    break;
  case 0x8086: // Intel
    break;
  }

  // or just generic drivers
  switch (device.classID)
  {
  case 0x03: // graphics devices
    switch (device.subclassID)
    {
    case 0x00: // VGA
      break;
    };
    break;
  }

  return 0;
}

BaseAddressRegister PCI::GetBaseAddressRegister(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint16_t bar)
{
  BaseAddressRegister result;

  uint32_t headerType = Read(busNumber, deviceNumber, functionNumber, 0x0E) & 0x7F;
  int maxBars = 6 - (4 * headerType);
  if (bar >= maxBars)
    return result;

  uint32_t barValue = Read(busNumber, deviceNumber, functionNumber, 0x10 + 4 * bar);

  // is it memorymap or I/O?
  result.type = (barValue & 0x1) ? InputOutput : MemoryMapping;
  uint32_t temp;

  if (result.type == MemoryMapping)
  {
    switch ((barValue >> 1) & 0x3)
    {
    case 0: // 32 bit mode
    case 1: // 20 bit mode
    case 2: // 16 bit mode
      break;
    }
  }
  else
  {
    result.address = (uint8_t *)(barValue & ~0x3);
    // result.prefetchable = false;
    result.prefetchable = ((barValue >> 3) & 0x1) == 1;
  }

  return result;
}

void PCI::SelectDrivers(DriverManager *driverManager, InterruptManager *interrupts)
{
  printf("b:d:f  vend:dev  [classID:subclassID]\n");
  for (int bus = 0; bus < 8; bus++)
  {
    for (int device = 0; device < 32; device++)
    {
      int numOfFunctions = DeviceHasFucntions(bus, device) ? 8 : 1;
      for (int function = 0; function < 8; function++)
      {

        uint32_t result = Read(bus, device, function, 0);
        if (result == 0x0 || result == 0xFFFFFFFF)
          continue;

        PCIDeviceDescriptor pciDevice = GetDeviceDescriptor(bus, device, function);

        for (int barNum = 0; barNum < 6; barNum++)
        {
          BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);
          if (bar.address && (bar.type == InputOutput))
          {
            pciDevice.portBase = (uint32_t)bar.address;

            Driver *driver = GetDriver(pciDevice, interrupts);

            if (driver != 0)
            {
              driverManager->AddDriver(driver);
            }
          }
        }

        printf("%i:%i:%i ", bus, device, function);
        if (device < 10)
          printf(" ");

        printf("%x%x:%x%x [%x:%x] = ", (pciDevice.vendorID & 0xFF00) >> 8, (pciDevice.vendorID & 0x00FF), (pciDevice.deviceID & 0xFF00) >> 8, pciDevice.deviceID & 0x00FF, pciDevice.classID, pciDevice.subclassID);

        if (pciDevice.vendorID == 0x1022) //AMD
        {
          printf("AMD, ");

          if (pciDevice.deviceID == 0x2000) //am79c973
            printf("79c970 [PCnet32 LANCE]\n");
          else 
            printf("unknow device\n"); 
        }

        else if (pciDevice.vendorID == 0x8086)
        {
          printf("Intel, ");
          if (pciDevice.deviceID == 0x7190)
            printf("Host bridge\n");
          else if (pciDevice.deviceID == 0x7191)
            printf("AGP bridge\n");
          else if (pciDevice.deviceID == 0x7110)
            printf("PIIX4 ISA\n");
          else if (pciDevice.deviceID == 0x100F)
            printf("82545EM Gigabit Ethernet Controller\n");
          else if (pciDevice.deviceID == 0x1237)
            printf("440FX - 82441FX PMC [Natoma]\n");
          else if (pciDevice.deviceID == 0x7000)
            printf("82371SB PIIX3 ISA [Natoma/Triton II]\n");
          else if (pciDevice.deviceID == 0x7010)
            printf("82371SB PIIX3 IDE [Natoma/Triton II]\n");
          else if (pciDevice.deviceID == 0x7020)
            printf("82371SB PIIX3 USB [Natoma/Triton II]\n");
          else if (pciDevice.deviceID == 0x7113)
            printf("82371AB/EB/MB PIIX4 ACPI\n");
          else if (pciDevice.deviceID == 0x7111)
            printf("82371AB/EB/MB PIIX4 IDE\n");
          else if (pciDevice.deviceID == 0x100E)
            printf("82540EM Gigabit Ethernet Controller\n");
          else if (pciDevice.deviceID == 0x2415)
            printf("82801AA AC'97 Audio Controller\n");
          else if (pciDevice.deviceID == 0x2829)
            printf("82801HM/HEM (ICH8M/ICH8M-E) SATA Controller [AHCI mode]\n");
          else
            printf("unknow device\n");
        }
        else if (pciDevice.vendorID == 0x80EE)
        {
          printf("InnoTek Systemberatung GmbH, ");
          
          if (pciDevice.deviceID == 0xCAFE)
            printf("VirtualBox Guest Service\n");
          else if (pciDevice.deviceID == 0xBEEF)
            printf("VirtualBox Graphics Adapter\n");
          else 
            printf("unknow device\n");
        }
        else if (pciDevice.vendorID == 0x106B)
        {
          printf("Apple Inc, ");

          if (pciDevice.deviceID == 0x003F)
            printf("KeyLargo/Intrepid USB\n");
          else 
            printf("unknow device\n");

        }
        else if (pciDevice.vendorID == 0x15AD)
        {
          printf("VMware, ");

          if (pciDevice.deviceID == 0x0405)
            printf("SVGA II Adapter\n");
          else if (pciDevice.deviceID == 0x0790)
            printf("PCI bridge\n");
          else if (pciDevice.deviceID == 0x07A0)
            printf("PCI Express Root Port\n");
          else if (pciDevice.deviceID == 0x0770)
            printf("USB2 EHCI Controller\n");
          else if (pciDevice.deviceID == 0x07E0)
            printf("SATA AHCI controller\n");
          else if (pciDevice.deviceID == 0x0774)
            printf("USB1.1 UHCI Controller\n");
          else
            printf("unknow device\n");
        }
        else if (pciDevice.vendorID == 0x1000)
        {
          printf("Broadcom, ");

          if (pciDevice.deviceID == 0x0030)
            printf("53c1030 PCI-X Fusion-MPT Dual Ultra320 SCSI\n");
          else
            printf("unknow device\n");
        }
        else if (pciDevice.vendorID == 0x1274)
        {
          printf("Ensoniq, ");
          if (pciDevice.deviceID == 0x1371)
            printf("ES1371/ES1373 / Creative Labs CT2518\n");
          else
            printf("unknow device\n");
        }
        else
        {
          printf("Unknow vendor and device\n");
        }
      }
    }
  }
}

PCIDeviceDescriptor PCI::GetDeviceDescriptor(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber)
{
  PCIDeviceDescriptor result;
  result.bus = busNumber;
  result.device = deviceNumber;
  result.function = functionNumber;

  result.vendorID = Read(busNumber, deviceNumber, functionNumber, 0x00);
  result.deviceID = Read(busNumber, deviceNumber, functionNumber, 0x02);

  result.classID = Read(busNumber, deviceNumber, functionNumber, 0x0B);
  result.subclassID = Read(busNumber, deviceNumber, functionNumber, 0x0A);
  result.interfaceID = Read(busNumber, deviceNumber, functionNumber, 0x09);

  result.revision = Read(busNumber, deviceNumber, functionNumber, 0x08);
  result.interrupt = Read(busNumber, deviceNumber, functionNumber, 0x3C);

  return result;
}
