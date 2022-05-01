# Potatos2

![Alt text](screenshots/1.png?raw=true "sceenshot")
![Alt text](screenshots/2.png?raw=true "sceenshot")


Rough TODO
- ✔ PCI scanner
- ✔ Scheluder
  - ✔ pit based
- ✔ VGA graphic mode, double buffering
- custom terminal font
- ! Interrupt exception handlers, awesome panic screen, stack trace
  - ✔ exception handles
  - awesome panic screen
  - stack trace
- ✔ Memory allocations/free
  - ✔ new and delete operators
- Memory mapping (and move kernel to higher half memory), paging
- Ethernet 
  - ✔ am79c9c73 driver
  - rtl8193 driver
  - ✔ ethernet frame
  - ✔ arp
  - icmp
  - ivp4
  - udp
  - tcp
- Serial port (write to)
- Standard libary
  - support com files
  - support elf files
- ata driver
- filesystems
  - fat16/32
  - extN
- User space