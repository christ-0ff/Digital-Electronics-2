Čtení hodnot ze senzoru BME 280 je uskutečněno pomocí I2C(TWI) komunikace. Pomocí I2C čteme hodnoty z paměťových registrů -- určených pro uložení naměřenýh dat -- daných datasheetem, které následně musíme přepočítat pomocí funkcí a pomocí kompenzačních dat daných datasheetem na správná data určující teplotu, tlak a vlhkost.

#### Čtení a kombinování kompenzačních hodnot

Čtení kompenzačních hodnot je uskutečněno pomocí I2C komunikace za pomocí `twi` knihovny. Čtení dat je provedeno jednorázovým čtením všech hodnot. Vzhledem k tomu, že paměť kompenzačních dat je rozdělena na dvě části, tak i čtení dat musí proběhnout dvakrat i přes to, že všechna data čteme jednorázově.
První čtení je uskutečněno na paměťových adresách `0x88` až `0xA1`. Druhé čtení začíná na adrese `0xE1` a končí na adrese `0xE7`. 
Vzhledem k tomu, že všechna kompenzační data jsou uložena v 8 bitových slovech, a přepočítávací funkce většinu z těchto dat požadují v určitém datovém typu, tak musíme většinu přečtených 8 bitových slov spojit do jednotlivých 16 bitových určených datových typů. Ty poté můžeme použít k přepočtu dat ze senzoru na reálné hodnoty teploty, tlaku a vlhkosti. Následující tabulka ukazuje jak adresy registrů, kde jsou jednotlivá datová slova uložena, tak i výsledné spojení do daného datového typu.

*Tabulka kompenzačních parametru s názvem a datovým typem*
| **Adresa registrů** | **Obsah registrů** | **Datový typ** |
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
| Změna paměti |||
| 0xE1/0xE2 | dig_H2 [7:0]/[15:8] | signed short |
| 0xE3 | dig_H3 [7:0] | unsigned char |
| 0xE4/0xE5[3:0] | dig_H4 [11:4]/[3:0] | signed short |
| 0xE5[7:4]/0xE6 | dig_T5 [3:0]/[11:4] | signed short |
| 0xE7 | dig_H6 | signed char |




#### Inicializace senzoru
#### Čtení paměťových registrů ukládající naměřená data
#### Přepočet dat
