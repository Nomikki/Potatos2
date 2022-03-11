#include <drivers/keyboard.hpp>
#include <stdio.h>

using namespace os::driver::Keyboard;

KeyboardEventHandler::KeyboardEventHandler()
{
}

KeyboardEventHandler::~KeyboardEventHandler()
{
}

void KeyboardEventHandler::OnKeyDown(char key)
{
}

void KeyboardEventHandler::OnKeyUp(char key)
{
}

KeyboardDriver::KeyboardDriver(os::communication::InterruptManager *manager, KeyboardEventHandler *eventHandler)
    : InterruptHandler(0x21, manager), // which interrupt number?
      dataPort(DATA_PORT),
      commandPort(COMMAND_PORT)
{
  this->eventHandler = eventHandler;
}

KeyboardDriver::~KeyboardDriver() {}

void KeyboardDriver::Activate()
{
  printf("Install keyboard... ");
  // wait until ready
  while (commandPort.Read() & 0x1)
    dataPort.Read();

  commandPort.Write(0xAE); // Activate interrupts
  commandPort.Write(0x20); // get current status
  uint8_t status = (dataPort.Read() | 1) & ~0x10;
  commandPort.Write(0x60); // set state
  dataPort.Write(status);

  dataPort.Write(0xF4); // finally activate keyboard
  printf("ok\n");
}

void KeyboardDriver::HandleKey(uint8_t key, bool pressedOrReleased)
{
  if (pressedOrReleased)
    eventHandler->OnKeyDown(key);
  else
    eventHandler->OnKeyUp(key);
}

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
  // if there is keystroke we want to fetch it
  uint8_t key = dataPort.Read();

  if (eventHandler == 0)
    return esp;

  static bool shift = false;
  bool pressedOrReleased = key < 0x80;

  if (key != 0)
  {
    switch (key)
    {
    case 0x01:
      HandleKey(0, pressedOrReleased);
      break; // esc

    case 0x29:
      HandleKey('?', pressedOrReleased);
      break;
    case 0x02:
      HandleKey(shift ? '!' : '1', pressedOrReleased);
      break;
    case 0x03:
      HandleKey(shift ? '\"' : '2', pressedOrReleased);
      break;
    case 0x04:
      HandleKey(shift ? '#' : '3', pressedOrReleased);
      break;
    case 0x05:
      HandleKey(shift ? '?' : '4', pressedOrReleased);
      break;
    case 0x06:
      HandleKey(shift ? '%' : '5', pressedOrReleased);
      break;
    case 0x07:
      HandleKey(shift ? '&' : '6', pressedOrReleased);
      break;
    case 0x08:
      HandleKey(shift ? '/' : '7', pressedOrReleased);
      break;
    case 0x09:
      HandleKey(shift ? '(' : '8', pressedOrReleased);
      break;
    case 0x0A:
      HandleKey(shift ? ')' : '9', pressedOrReleased);
      break;
    case 0x0B:
      HandleKey(shift ? '=' : '0', pressedOrReleased);
      break;
    case 0x0C:
      HandleKey(shift ? '?' : '+', pressedOrReleased);
      break;
    case 0x0D:
      HandleKey('?', pressedOrReleased);
      break;
    case 0x0E:
      HandleKey(0, pressedOrReleased);
      break; // backspace

    case 0x0F:
      HandleKey(0, pressedOrReleased);
      break; // tab
    case 0x10:
      HandleKey(shift ? 'Q' : 'q', pressedOrReleased);
      break;
    case 0x11:
      HandleKey(shift ? 'W' : 'w', pressedOrReleased);
      break;
    case 0x12:
      HandleKey(shift ? 'E' : 'e', pressedOrReleased);
      break;
    case 0x13:
      HandleKey(shift ? 'R' : 'r', pressedOrReleased);
      break;
    case 0x14:
      HandleKey(shift ? 'T' : 't', pressedOrReleased);
      break;
    case 0x15:
      HandleKey(shift ? 'Y' : 'y', pressedOrReleased);
      break;
    case 0x16:
      HandleKey(shift ? 'U' : 'u', pressedOrReleased);
      break;
    case 0x17:
      HandleKey(shift ? 'I' : 'i', pressedOrReleased);
      break;
    case 0x18:
      HandleKey(shift ? 'O' : 'o', pressedOrReleased);
      break;
    case 0x19:
      HandleKey(shift ? 'P' : 'p', pressedOrReleased);
      break;
    case 0x1A:
      HandleKey(shift ? '?' : '?', pressedOrReleased);
      break;
    case 0x1C:
      HandleKey('\n', pressedOrReleased);
      break; // return

    case 0x3A:
      HandleKey(0, pressedOrReleased);
      break; // capslock
    case 0x1E:
      HandleKey(shift ? 'A' : 'a', pressedOrReleased);
      break;
    case 0x1F:
      HandleKey(shift ? 'S' : 's', pressedOrReleased);
      break;
    case 0x20:
      HandleKey(shift ? 'D' : 'd', pressedOrReleased);
      break;
    case 0x21:
      HandleKey(shift ? 'F' : 'f', pressedOrReleased);
      break;
    case 0x22:
      HandleKey(shift ? 'G' : 'g', pressedOrReleased);
      break;
    case 0x23:
      HandleKey(shift ? 'H' : 'h', pressedOrReleased);
      break;
    case 0x24:
      HandleKey(shift ? 'J' : 'j', pressedOrReleased);
      break;
    case 0x25:
      HandleKey(shift ? 'K' : 'k', pressedOrReleased);
      break;
    case 0x26:
      HandleKey(shift ? 'L' : 'l', pressedOrReleased);
      break;
    case 0x2B:
      HandleKey(shift ? '*' : '\'', pressedOrReleased);
      break;

    case 0x56:
      HandleKey(shift ? '>' : '<', pressedOrReleased);
      break;
    case 0x2C:
      HandleKey(shift ? 'Z' : 'z', pressedOrReleased);
      break;
    case 0x2D:
      HandleKey(shift ? 'X' : 'x', pressedOrReleased);
      break;
    case 0x2E:
      HandleKey(shift ? 'C' : 'c', pressedOrReleased);
      break;
    case 0x2F:
      HandleKey(shift ? 'V' : 'v', pressedOrReleased);
      break;
    case 0x30:
      HandleKey(shift ? 'B' : 'b', pressedOrReleased);
      break;
    case 0x31:
      HandleKey(shift ? 'N' : 'n', pressedOrReleased);
      break;
    case 0x32:
      HandleKey(shift ? 'M' : 'm', pressedOrReleased);
      break;
    case 0x33:
      HandleKey(shift ? ';' : ',', pressedOrReleased);
      break;
    case 0x34:
      HandleKey(shift ? ':' : '.', pressedOrReleased);
      break;
    case 0x35:
      HandleKey(shift ? '_' : '-', pressedOrReleased);
      break;

    case 0x39:
      HandleKey(' ', pressedOrReleased);
      break;

    case 0x2A:
    case 0x36:
      shift = true;
      break;
    case 0xAA:
    case 0xB6:
      shift = false;
      break;

    default:
      if (key < 0x80)
        printf("0x%x ", key);
    }
  }

  return esp;
}