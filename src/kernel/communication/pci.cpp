#include <communication/pci.hpp>
#include <stdio.h>

using namespace os::communication;
using namespace os::driver;

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
  printf("bus, device, function, result (hex, bin):\n");
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
          printf("%i:%i:%i", bus, device, function);
          if (device < 10)
            printf(" ");
          
          printf(" -> %X %B\n", result, result);

          break;
        }
      }
    }
  }
}