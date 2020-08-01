# Drtd
A digital radio transmission decoder  
  
![Screenshot](https://raw.githubusercontent.com/tlmrgvf/drtd/master/rtty.gif)

## Currently supported
* AX.25/APRS
* RTTY _(*)_
* POCSAG 512/1200/2400 _(*)_ 
* DTMF _(*)_
* DCF77 _(*)_
  
_(*) Can be run in headless mode right now_
## Planned
* ADS-B
* BPSK/QPSK/8-PSK
* NAVTEX
* SSTV
  
## Headless mode
Some decoders can be run in headless mode. To do this, just pass the `-g` option and the decoder name, for example 
`-g RTTY`.  
The very last parameters to the program should be the decoder specific parameters, like the center frequency.  
  
For example, to decode a RTTY transmission at 1000 Hz received on the upper sideband at 50 bauds using a shift of
450 Hz, run:
````shell
java -jar drtd/drtd.jar -g RTTY 1000 450 50 USB
````  

The decoder parameters differ from decoder to decoder, and the specific parameters can be shown by not passing any
decoder parameters at all:  
````shell
$ java -jar drtd/drtd.jar -g RTTY
Invalid parameters! Available parameters: [Center frequency (Int)] [Shift (Int)] [Baud rate (Float)] [USB/LSB]
...
````  
_Note that the decoder parameters have to be the **very last** arguments to the program!_

To show other options, just pass the ``--help`` option

## Build
**Required:**
* Java JDK 8
* Maven

**To build:**  
````shell
git clone https://github.com/tlmrgvf/drtd.git
cd drtd
mvn clean package
````  
  
**And to run:**  
````shell
java -jar drtd/drtd.jar
````
