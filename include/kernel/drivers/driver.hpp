#ifndef _DRIVER_HPP
#define _DRIVER_HPP

class Driver
{
public:
  Driver();
  ~Driver();
  virtual void Activate();
  virtual int Reset();
  virtual void Deactivate();
};

class DriverManager
{
private:
  Driver *mDrivers[255];
  int numDrivers;

public:
  DriverManager();
  ~DriverManager();
  void AddDriver(Driver *driver);
  void ActivateAll();
};

#endif