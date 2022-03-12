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

void PCI::SelectDrivers(DriverManager *driverManager)
{
  printf("b:d  vend:dev  [classID:subclassID]\n");
  for (int bus = 0; bus < 8; bus++)
  {
    for (int device = 0; device < 32; device++)
    {
      int numOfFunctions = DeviceHasFucntions(bus, device) ? 8 : 1;
      for (int function = 0; function < 8; function++)
      {

        uint32_t result = Read(bus, device, function, 0);
        if (result != 0xFFFFFFFF)
        {

          PCIDeviceDescriptor pciDevice = GetDeviceDescriptor(bus, device, function);

          printf("%i:%i ", bus, device);
          if (device < 10)
            printf(" ");

          printf("%x%x:%x%x [%x:%x] = ", (pciDevice.vendorID & 0xFF00) >> 8, (pciDevice.vendorID & 0x00FF), (pciDevice.deviceID & 0xFF00) >> 8, pciDevice.deviceID & 0x00FF, pciDevice.classID, pciDevice.subclassID);

          if (pciDevice.vendorID == 0x8086)
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

          break;
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