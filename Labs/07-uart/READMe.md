# Lab 7: Kryštof Buroň

Link to this file in GitHub repository:

[https://github.com/christ-0ff/Digital-Electronics-2/tree/main/Labs/07-uart](https://github.com/christ-0ff/Digital-Electronics-2/tree/main/Labs/07-uart)


### Analog-to-Digital Conversion

1. Completed table with voltage divider, calculated, and measured ADC values for all five push buttons.

   | **Push button** | **PC0[A0] voltage** | **ADC value (calculated)** | **ADC value (measured)** |
   | :-: | :-: | :-: | :-: |
   | Right  | 0&nbsp;V | 0   | 0 |
   | Up     | 0.495&nbsp;V | 101 | 99 |
   | Down   | 1.202&nbsp;V | 246 | 257 |
   | Left   | 1.969&nbsp;V | 403 | 409 |
   | Select | 3.181&nbsp;V | 651 | 639 |
   | none   | 5&nbsp;V | 1023 | 1023 |

2. Code listing of ACD interrupt service routine for sending data to the LCD/UART and identification of the pressed button:

```c
/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Display value on LCD and send it to UART.
 **********************************************************************/
ISR(ADC_vect)
{
    uint16_t value = 0;
    char lcd_string[] = "0000";
    char lcd_string_hex[] = "000";

    value = ADC;                     // Copy ADC result to 16-bit variable
    itoa(value, lcd_string, 10);     // Convert decimal value to string
    itoa(value, lcd_string_hex, 16); // Convert hexadecimal value to string
    
    // Puts decimal string to LCD
    lcd_gotoxy(8,0);
    lcd_puts("    ");
    lcd_gotoxy(8,0);
    lcd_puts(lcd_string);
    
    // Puts hexadecimal string to LCD
    lcd_gotoxy(13,0);
    lcd_puts("   ");
    lcd_gotoxy(13,0);
    lcd_puts(lcd_string_hex);
    
    //UART communication
    uart_puts("ADC Value: ");
    uart_puts(lcd_string);
    uart_puts(" bits  ");
    //uart_putc('\n');
    uart_putc('\r');    
}
```


### UART communication

1. (Hand-drawn) picture of UART signal when transmitting three character data `De2` in 4800 7O2 mode (7 data bits, odd parity, 2 stop bits, 4800&nbsp;Bd).

   ![UART communication](images/uart.png)

2. Flowchart figure for function `uint8_t get_parity(uint8_t data, uint8_t type)` which calculates a parity bit of input 8-bit `data` according to parameter `type`:

   ![Function parity flowchart](images/flowchart.png)


### Temperature meter

An application for temperature measurement and display. Used temperature sensor [TC1046](http://ww1.microchip.com/downloads/en/DeviceDoc/21496C.pdf), LCD, one LED and a push button. After pressing the button, the temperature is measured, its value is displayed on the LCD and data is sent to the UART. When the temperature is too high, the LED will start blinking.

1. Scheme of temperature meter:

   ![Temperature Meter](images/temp_meter.png)
