Čtení hodnot ze senzoru BME 280 je uskutečněno pomocí I2C(TWI) komunikace. Pomocí I2C čteme hodnoty z paměťových registrů -- určených pro uložení naměřenýh dat -- daných datasheetem, které následně musíme přepočítat pomocí funkcí, a pomocí kompenzačních dat uložených v senzoru na správná data určující teplotu, tlak a vlhkost.

*Připojení senzoru BME 280 k Arduino UNO*
[scheme](images/scheme.png)

#### Čtení a kombinování kompenzačních hodnot
Čtení kompenzačních hodnot je uskutečněno pomocí I2C komunikace za pomocí `twi` knihovny od pana Tomáše Frýzy, Petera Fleuryho. Čtení dat je provedeno jednorázovým čtením všech hodnot. Vzhledem k tomu, že paměť kompenzačních dat je rozdělena na dvě části, tak i čtení dat musí proběhnout dvakrat i přes to, že všechna data čteme jednorázově.
První čtení je uskutečněno na paměťových adresách `0x88` až `0xA1`. Druhé čtení začíná na adrese `0xE1` a končí na adrese `0xE7`. 
Z toho důvodu, že všechna kompenzační data jsou uložena v 8 bitových slovech, a přepočítávací funkce většinu z těchto dat požadují v určitém datovém typu, tak musíme většinu přečtených 8 bitových slov spojit do jednotlivých 16 bitových určených datových typů. Ty poté můžeme použít k přepočtu dat ze senzoru na reálné hodnoty teploty, tlaku a vlhkosti. Následující tabulka ukazuje jak adresy registrů, kde jsou jednotlivá datová slova uložena, tak i výsledné spojení do daného datového typu.

*Tabulka kompenzačních parametru s názvem a datovým typem*
| **Adresa registru** | **Obsah registru** | **Datový typ** |
| :-: | :-: | :-- | 
| 0x88/0x89 | dig_T1 [7:0]/[15:8] | unsigned short |
| 0x8A/0x8B | dig_T2 [7:0]/[15:8] | signed short |
| 0x8C/0x8D | dig_T3 [7:0]/[15:8] | signed short |
| 0x8E/0x8F | dig_P1 [7:0]/[15:8] | unsigned short |
| 0x90/0x91 | dig_P2 [7:0]/[15:8] | signed short |
| 0x92/0x93 | dig_P3 [7:0]/[15:8] | signed short |
| 0x94/0x95 | dig_P4 [7:0]/[15:8] | signed short |
| 0x96/0x97 | dig_P5 [7:0]/[15:8] | signed short |
| 0x98/0x99 | dig_P6 [7:0]/[15:8] | signed short |
| 0x9A/0x9B | dig_P7 [7:0]/[15:8] | signed short |
| 0x9C/0x9D | dig_P8 [7:0]/[15:8] | signed short |
| 0x9E/0x9F | dig_P9 [7:0]/[15:8] | signed short |
| 0xA1 | dig_H1 [7:0] | unsigned char |
| Změna paměti |
| 0xE1/0xE2 | dig_H2 [7:0]/[15:8] | signed short |
| 0xE3 | dig_H3 [7:0] | unsigned char |
| 0xE4/0xE5[3:0] | dig_H4 [11:4]/[3:0] | signed short |
| 0xE5[7:4]/0xE6 | dig_T5 [3:0]/[11:4] | signed short |
| 0xE7 | dig_H6 | signed char |

#### Inicializace senzoru
K inicializaci je opět použita I2C komunikace pomocí knihovny `twi`. Senzor má několik nastavitelných módů, a to `Sleep mode`,`Forced mode` a `Normal mode`.

Funkce jednotlivých módů:
* `Sleep mode` - Tento mód je nastaven jako výchozí mód po zapnutí senzoru. V tomto módu jsou přístupny všechny registry, a proto ještě před inicializací senzoru čteme kompenzační data. V tomto módu senzor neprovádí žádná měření, a tak je jeho odběr energie na jeho minimu.
* `Forced mode` - Ve "vynuceném módu" je provedeno jedno měření podle jeho nastavení. Jakmile je měření dokončeno, tak se senzor vrátí do `Sleep mode`, a data z měření jsou dostupná v paměťových registrech. Jakmile chceme provést další měření, musíme znovu zvolit `Forced mode`.
* `Normal mode` - `Normal mode` je mód, kdy je prováděno opakované automatické meření. Senzor se v tomhle módu přepíná ze stavu `Sleep mode` do stavu `Normal mode`, kde senzor naměří hodnoty, a následně se přepně do stavu `Sleep mode`. Můžeme také nastavit jak často se budou jednotlivá měření opakovat. Jednotlivá nastavení se pohybují od 0,5 do 1000 ms.

*Obrázek stavového diagramu ukazuje přechod mezi jednotlivými módy*
[stavovy_diagram](images/stavovy_diagram.png)

Dále můžeme nastavit "oversampling" naměřených ADC dat, a také jejich filtraci IIR filtrem.
* Oversampling ADC dat slouží k redukci šumu, a k zpřesnění naměřených dat. Může nabývat hodnot `1×`, `2×`, `4×`, `8×` a `16×`. Jestliže je oversampling u některého z dat vypnutý, tak senzor tato data neměří.
* Filtr slouží k odfiltrování "skokových hodnot". Takové hodnoty mohou vzniknout například když na senzor zafouká vítr, tím se zvedne i změřený tlak, který už bude nepřesný, protože senzor nebude měřit tlak, ale nárazový vítr. Proto je dobré mít IIR filtr zapnutý. Filtr se dá zapnout jen pro teplotní a tlaková data. Zapnutím filtru se zvyšuje jejich rozlišení, ale také se zvyšuje odezva senzoru. Filtr může nabývat několika koeficientů, které zvyšují jeho přesnost, a to `1 - Vypnutý filtr`, `2`, `4`, `8` a `16`.

 I přes to, že sensor oplývá několika módy nastavení, tak je natvrdo nastavený do módu `Normal mode`, s nastaveným `16× oversamplingem` a s filtrem, jehož koeficient je nastavený na `16`.
 
#### Čtení paměťových registrů ukládající naměřená data a jejich kombinování
Čtení naměřených dat je uskutečněno pomocí `twi` knihovny. Podle datasheetu, je doporučenu jej udělat jednorázově. Je to kvůli toho, aby jsme náhodou nepomíchali data z jednotlivých měření, a taky abychom zredukovali provoz na I2C rozhraní. I kdybychom měli vypnutá nějaká měření, tak je stále lepší všechny registry přečíst jednorázově.
Jednotlivá data jsou uložena na adresách v paměti od adresy `0xF7` do adresy `0xFE`. Na následující tabulce můžeme vidět na jakých adresách jsou uložena která data.

*Tabulka naměřených dat*
| **Název registru** | **Adresa registru** | **Obash registru** |
| :-: | :-: | :-- | 
| hum_lsb | 0xFE | hum_lsb[7:0] |
| hum_msb | 0xFD | hum_msb[7:0] |
| temp_xlsb | 0xFC | temp_xlsb[7:4] |
| temp_lsb | 0xFB | temp_lsb[7:0] |
| temp_msb | 0xFA | temp_msb[7:0] |
| press_xlsb | 0xF9 | press_xlsb[7:4] |
| press_lsb | 0xF8 | press_lsb[7:0] |
| press_msb | 0xF7 | press_msb[7:0] |

Data vyčtená z tabulky musíme následně spojit do datasheetem určených datových typů. Pro vlhkost je to neznaménkový 16 bitový formát uložený v znaménkovém 32 bitovém formátu. Pro teplotu i tlak je to 20 bitový formát uložený ve znaménkovém 32 bitovém formátu. Takto spojená data můžeme následně přepočítat do správných změřených hodnot.

#### Přepočet dat
Přepočet dat je uskutečněn pomocí kompenzačních dat stažených z registrů senzoru a z naměřených dat. Všechna tato data jsou vložena do funkcí definovaných výrobcem, které jsou obsažené v datasheetě.

