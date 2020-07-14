# Drtd
A digital radio transmission decoder  
![Screenshot](https://raw.githubusercontent.com/tlmrgvf/drtd/master/rtty.gif)

## Currently supported
* AX.25/APRS
* RTTY
* POCSAG 512/1200/2400
* DTMF
* DCF77

## Planned
* ADS-B
* BPSK/QPSK/8-PSK
* NAVTEX
* SSTV

## Build
**Required:**
* Java JDK 11
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
Pass ``--help`` to show command line options
