#include <communication/pci.hpp>

using namespace os::communication;

PCI::PCI() : dataPort(PCI_DATAPORT), commandPort(PCI_COMMANDPORT)
{
}

PCI::~PCI()
{
}

uint32_t PCI::Read(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset)
{
  return 0;
}

void PCI::Write(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset, uint32_t value)
{
}