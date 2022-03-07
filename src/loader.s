# Higher half kernel:
# Kerneli ladataan 1 megan kohdille, mutta siirrämme sen alkamaan 3 gigan paikkeilta, jotta kaiken muun voi laittaa
# alkamaan vaikkapa 0x0-osoitteesta, ja näin ei ole kerneli tiellä. Ja ei haittaa vaikka laitteessa ei ole kolmea gigaa muistia käytössä,
# sillä virtualisointi tulee siinä apuun. Käytännössä kerneli on edelleen fyysisesti 1 megan kohdalla, mutta tämä osio on mapattu alkamaan kolmesta gigasta.
#
#	* Varataan alkuun muutama sivu, jotta on mahdollista parsia GRUB-rakenteita ennen kuin laitetaan muistinhallinta käyttöön.
#	* Luodaan taulu joka sisältää sivut jotka osoittavat kerneliin (käytännössä koko kernelin alue mapataan ja määritellään näkyväksi - present)
#	* Luodaan juurisivuhakemisto joka sisältää edellämainitut sekä alhaiset osoitteet että kolmen gigan osoitteet
#	* Asetetaan sivutus päälle
#	* Hypätään higher halfin puolelle
#	* Poistetaan alemman tason muistiosoitteet
#	* Nyt kerneli sijaitsee pelkästään ylämuistissa!
#
# Toki kerneli edelleen sijaitsee fyysisesti alamuistissa, mutta pointtina onkin, ettei fyysistä muistia käytetä ellei siihen ole erikseen painavaa syytä.
#

.set MAGIC,			0x1badb002
.set FLAGS,			(1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

# Aloitetaan multiboot-osuudella. Eli alkuun muutamat olennaiset tieodt, jotta grub löytää tämän.
.section .multiboot
	.long MAGIC
	.long FLAGS
	.long CHECKSUM

# Varataan paikka tauluille ja sivutukselle. Pidetään se 4k:n järjestyksessä ja tyhjänä.
# Tämä osio sijaitsee BSS:ssä (Basic service set), jossa yleensä sijaitsee paikallisesti varattu tieto ja tieto jolle ei ole annettu arvoa.
.section .bss, "aw", @nobits
	.align 4096
.global boot_page_directory
.global boot_page_table1

boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096

.section .multiboot
loader:

	# push %ebx				 # multibooti-headerin osoite
	# Napataan vain tieto uppermemoryn koosta ja asetetaan se EBX:ään.
	movl 0x10100, %ebx

	# Juurisivutaulun fyysinen osoite (lineaarinen/ns. oikea osoite)
	# Siirretään se EDI-rekisteriin. EDI on destination register.
	movl $(boot_page_table1 - 0xC0000000), %edi

	# Ensiksi mapataan osoite 0. ESI on source register
	# Todo: Mapataan 1 mega, jotta saadaan muutakin hyödylistä tietoa talteen, ei pelkästään vga-bufferi
	movl $0, %esi

	# Mapataan 1023 sivua. 1024. sivu on VGA-bufferi.
	# ECX toimii monesti laskurina ja se on tässäkin ideana (kts. seuraavat osiot).
	movl $1023, %ecx

1:
	# Mapataan vain kerneli
	# Verrataan kernelin aloitus- ja lopetusosoitteen ja ESI-rekisterin sisältöjä.
	# Ideana on mapata vain kerneli.
	# Jos ESI:n sisältö on < kuin kernelin aloitusosoite, hypätään kohtaan 2.
	# JOS ESI:n sisältö on >= kuin kernelin lopetusosoite - offset, hypätään kohtaan 3. Eli olemma löytäneet sopivan kohdan.
	cmpl $_kernel_start, %esi
	jl 2f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f

	# Mapataan fyysinen osoite tageilla PRESENT, WRITABLE
	# Eli laitetaan ESI EDX-rekisteriin, lisätään sille luku 3 (kaksi ylös-bittiä)
	# ei laiteta tietoa enää takaisin ESI-rekisteriin, vaan suoraan sijaintiin jonka EDI määrittelee.
	# Nyt tässä sijainnissa on ESI:n sisältö + 0x3.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)
2:
	# ESIn luku on pienempi kuin kernelin aloitusosoite, jotenka päädyttiin tänne.
	# Lisätään ESIin 4096 
	addl $4096, %esi
	# Lisätään EDIin 4, koska juurisivutaulu on 4 tavua.
	addl $4, %edi
	# Voisimme käyttää jmp:tä jolla hypätä taas 1. osioon, mutta loopilla vähennetään ECX:stä yksi.
	# Jos ECX on jo nolla, loop skipataan ja jatketaan normaalisti.
	loop 1b
3:
	# Päädyimme tänne, eli ESI sisältää osoitteen josta kernelimme alkaa.
	# Nyt ECX:n sisältö on 0x2EA, eli 746. Olemme siis mapanneet näin monta sivua. Karkeasti tämä tarkoittaa sitä, jos kerromme luvun 4096:lla,
	# olemme mapanneet 3 megaa (jokainen sivu edustaa 4 kilotavua)

	# mapataan vielä VGA:n muistialue ja merkataan sen olevan esillä ja kirjoitettavana.
	# Eli normaalisti VGA:n muistialue on osoitteessa 0xB8000, mutta koska mappaus, siirretään se ylös myös ja laitetaan se tauluun #1022
	 movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1022 * 4
	
	# Luodaan kaksi sivutauluosoitetta jotka molemmat osoittaa samoihin osoitteisiin
	# Ensimmäinen on 1:1 fyysisen osoitteen kanssa
	# Ja toinen on mapattu alkamaan kolmen gigan paikkeilta.
	# Tarvitsemme tämmöisen kikan, koska laitamme sivutuksen päälle, ja jos hyppäämme suoraan muistiin jota ei ole, aiheutuu siitä page fault.
	# Sen sijaan asetamme sivutukseen tämän identify-osoitteen (vastaa 1:1 fyysistä osoitetta) ja teemme absoluuttisen loikan.
	
	# Mapataan molemmat virtuaaliosoitteet osoittamaan samaan kohtaan muistia.
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4


	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + (1023 * 4)
	movl $(boot_page_directory - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + (1023 * 4)
	# Annetaan CR3-rekisterille juurisivuhakemiston osoite
	# Koska emme tarvitse nyt ECX:n laskuria enää, voimme hyödyntää sen tähän.
	# Sisältönä on 0x10E000
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	# CR3-Rekisteri sisältää nyt tiedon missä sivutuksen juurihakemistot sijaitsee ja voimme laittaa sivutuksen päälle write-protect-bitin kanssa.	
	# Huom. vaikka käytämme sivutusta, vielä fyysinen ja virtuaalinen osoite vastaa toisiaan 1:1.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Hypätään higher halfiin pienen tempun voimin!
	# LEA = Load effective address. Eli ladataan ECX:ään 4f:n osoite, joka on (tällä hetkellä): 0xC010007A.
	# Ja hypätään siihen. Normaalisti hypyt on relatiivisia, mutta pienellä kikalla saamme siitä absoluuttisen.
	lea 4f, %ecx
	jmp *%ecx
	
.section .text
.extern kernel_main
.extern call_constructors
.global loader

# Higher half kernelin sisääntulo!
# Tämä osio on linkkerissä määritelty alkamaan kohdasta 0xC0000000.
4:
	# Nyt sivutus on täysin kunnossa ja käytössä.
	# Voimme unmapata identitymappauksen, koska emme tarvitse sitä enää.
	# Eli asetamme vain sisällöksi pelkkää nollaa
	movl $0, boot_page_directory + 0

	# Ladataan nyt CR3 uudestaan (otetaan sisältö ECX-rekisteriin ja siirretään ECX CR3-rekisteriin)
	# Tämä aiheuttaa TLB-flushauksen, jolloinka MMU päivittyy.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Annetaan pinolle osoitteeksi kernel_stackin kohta.
	mov $kernel_stack, %esp 

	# Koska käytämme C++:aa, vaatii se tämän.
	# Eli kutsumme hieman C++-koodia ennen kuin hyppäämme main-funktioon.
	call call_constructors


	# addl $0xC03DF000, %ebx
	# movl $0x00010000, %ebx 

	# Annetaan parametrit kernelin main-funktiolle, alkaen oikealta vasemmalle.
	push $kernel_stack_start # mistä alkaa pino
	push %esp				 # pinon 'pohja'
	push %eax				 # magic number
	push %ebx				 # upper memoryn koko

	xor %ebp, %ebp

	call kernel_main
	
_stop:
	cli
	hlt
	jmp _stop

.section .bootstrap_stack, "aw", @nobits
kernel_stack_start:
# .space 1*1024*1024 #1 MiB
.space 16*1024 # 16 KiB
kernel_stack:
