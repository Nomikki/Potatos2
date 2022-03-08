#include "memory/gdt.h"
#include "common/stdio.h"

struct GlobalDescriptorTableSructure
{
  uint32_t address;
  uint16_t size;
} __attribute__((packed));

static GlobalDescriptorTableSructure gdtPacket;
static uint64_t gdtEntries[5];

uint64_t gdt_create_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
{
uint64_t descriptor;

  // Create the high 32 bit segment
  descriptor = limit & 0x000F0000;         // set limit bits 19:16
  descriptor |= (flag << 8) & 0x00F0FF00;  // set type, p, dpl, s, g, d/b, l and avl fields
  descriptor |= (base >> 16) & 0x000000FF; // set base bits 23:16
  descriptor |= base & 0xFF000000;         // set base bits 31:24

  // Shift by 32 to allow for low part of segment
  descriptor <<= 32;

  // Create the low 32 bit segment
  descriptor |= base << 16;         // set base bits 15:0
  descriptor |= limit & 0x0000FFFF; // set limit bits 15:0

  
  printf("gdt entry: base:%X, limit:%X, flags:%X\n", base, limit, flag);



  return descriptor;
}

void gdt_setup()
{
  printf("init gdt\n");
 __asm__("cli");

  gdtEntries[0] = gdt_create_descriptor(0, 0, 0);
  gdtEntries[1] = gdt_create_descriptor(0, 0xFFFFFFFF, (GDT_CODE_PL0));
  gdtEntries[2] = gdt_create_descriptor(0, 0xFFFFFFFF, (GDT_DATA_PL0));
  gdtEntries[3] = gdt_create_descriptor(0, 0xFFFFFFFF, (GDT_CODE_PL3));
  gdtEntries[4] = gdt_create_descriptor(0, 0xFFFFFFFF, (GDT_DATA_PL3));

  gdtPacket.address = (uint32_t)&gdtEntries[0];
  gdtPacket.size = sizeof(gdtEntries);

  printf("gdt addr: 0x%X\n", gdtPacket.address);
  
  uint32_t i[2];
  i[1] = gdtPacket.address;
  i[0] = gdtPacket.size << 16;

  // Load GDT
  asm volatile("lgdt (%0)"
               :
               : "p"(((uint8_t *)i) + 2));

  gdt_flush();

  printf("gdt ok\n\n");
}
