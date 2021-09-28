<div align="right">Kryštof Buroň, 221441</div>


# Lab 1: Kryštof Buroň

Link to `Digital-electronics-2` GitHub repository:

   [https://github.com/christ-0ff/Digital-Electronics-2](https://github.com/christ-0ff/Digital-Electronics-2)


### Blink example

1. What is the meaning of the following binary operators in C?
   * `|`  - OR 
   * `&`  - AND
   * `^`  - XOR
   * `~`  - NOT
   * `<<` - Bit Shift to Left
   * `>>` - Bit Shift to Right

2. Complete truth table with operators: `|`, `&`, `^`, `~`

| **b** | **a** |**b or a** | **b and a** | **b xor a** | **not b** |
| :-: | :-: | :-: | :-: | :-: | :-: |
| 0 | 0 | 0 | 0 | 0 | 1 |
| 0 | 1 | 1 | 0 | 1 | 1 |
| 1 | 0 | 1 | 0 | 1 | 0 |
| 1 | 1 | 1 | 1 | 0 | 0 |


### Morse code

1. Listing of C code with syntax highlighting which repeats one "dot" and one "comma" on a LED:

```c
int main(void)
{
    // Set pin as output in Data Direction Register
    // DDRB = DDRB or 0010 0000
    DDRB = DDRB | (1<<LED_GREEN);

    // Set pin LOW in Data Register (LED off)
    // PORTB = PORTB and 1101 1111
    PORTB = PORTB & ~(1<<LED_GREEN);

    // Infinite loop
    while (1)
    {
        // Pause several milliseconds
        _delay_ms(SHORT_DELAY);
        
        // = .
        PORTB = PORTB ^ (1<<LED_GREEN); 
        _delay_ms(DOT_DELAY);
        PORTB = PORTB ^ (1<<LED_GREEN); 
        _delay_ms(SPACE_DELAY);
        // = -        
        PORTB = PORTB ^ (1<<LED_GREEN); 
        _delay_ms(DASH_DELAY);
        PORTB = PORTB ^ (1<<LED_GREEN); 
        _delay_ms(SPACE_DELAY);
    }

    // Will never reach this
    return 0;
}
```


2. Scheme of Morse code application (connection of AVR device, LED, resistor, and supply voltage). 

   ![figure](images/1.png)
