#include <drivers/driver.hpp>

using namespace os::driver;

Driver::Driver()
{
}

Driver::~Driver()
{
}

void Driver::Activate()
{
}

int Driver::Reset()
{
  return 0;
}

void Driver::Deactivate()
{
}

DriverManager::DriverManager()
{
  numDrivers = 0;
}

DriverManager::~DriverManager()
{
}

void DriverManager::AddDriver(Driver *driver)
{
  mDrivers[numDrivers] = driver;
  numDrivers++;
}

void DriverManager::ActivateAll()
{
  for (int i = 0; i < numDrivers; i++)
  {
    mDrivers[i]->Activate();
  }
}