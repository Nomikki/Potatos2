#ifndef __PCI_H__
#define __PCI_H__

/*
  PCI:llä on 8 väylää. Jokaisella väylällä voi olla 32 laitetta.
  Laitteella voi olla 8 funktiota.

  PCI -> bus N ---> Device 1 -->  function 1
               \__> Device 2  \_> ....
                \_> ....       \_>function 8 
                 \> Device 32

  Eli tarvitaan 3 bittiä enkoodaamaan väylänumero, 5 bittiä enkoodaamaan laitenumero ja taas
                3 bittiä enkoodaamaan funktionumero.
                Yhteensä 3 + 8 + 3 = 14 bittiä

  Luodaan funktio jolla voidaan kysyä vaikkapa: mikä laite on osoitteessa 1 bus, 3 device
  ja se palauttaa kaksi uint16_t-arvoa, joista ensimmäinen on vendorID ja toinen deviceID.

  PCI controller kertoo myös classID:n ja subclassID:n. Ne on myös uint16_t-tyyppisiä.
  ClassID kertoo minkälainen laite kyseessä, vaikkapa multimedialaite ja subclassID voi kertoa
  että kyse on VGA-kontrollerista. Näin ei tarvitse jokaiselle raudalle tehdä erikseen ajureita,
  vaan voidaan käyttää standardoitua ajuria (vaikka se ei välttämättä olisi se tehokkain ratkaisu)

  Näillä tiedoilla voidaan asettaa oikeat ajurit käyttöön


*/

#include <stdint.h>
#include <communication/ports.hpp>
#include <communication/idt.hpp>

namespace os::communication
{
  #define PCI_DATAPORT 0xCFC
  #define PCI_COMMANDPORT 0xCF8

  class PCI
  {
    os::communication::Port32Bit dataPort;
    os::communication::Port32Bit commandPort;

  public:
    PCI();
    ~PCI();

    uint32_t Read(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset);
    void Write(uint16_t busNumber, uint16_t deviceNumber, uint16_t functionNumber, uint32_t registerOffset, uint32_t value);
  };
  
  
  
  
  
  

};

#endif // __PCI_H__