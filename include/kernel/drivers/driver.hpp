#ifndef __DRIVER_H__
#define __DRIVER_H__

namespace os::driver
{

#define DATA_PORT 0x60
#define COMMAND_PORT 0x64

#define ENABLE_AUXILIARY_DEVICE 0xA8
#define GET_COMPAQ_STATUS_BYTE 0x20
#define SET_COMPAQ_STATUS_BYTE 0x60
#define WAIT_COMMAND_PORT 0xD4

#define RESET 0xFF
#define RESEND 0xFE
#define SET_DEFAULTS 0xF6
#define SET_SAMPLERATE 0xF3
#define ENABLE_PACKED_STREAMING 0xF4
#define DISABLE_PACKED_STREAMING 0xF5

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
   
    int numDrivers;

  public:
   os::driver::Driver *mDrivers[255];
    DriverManager();
    ~DriverManager();
    void AddDriver(os::driver::Driver *driver);
    void ActivateAll();
  };

};

#endif // __DRIVER_H__