# Drtd
A digital radio transmission decoder  
![Screenshot](https://raw.githubusercontent.com/tlmrgvf/drtd/master/rtty.gif)

## Currently supported
* AX.25/APRS
* RTTY
* POCSAG 512/1200/2400
* DTMF

## Planned to be added
* ADS-B
* DCF77
* NAVTEX
* SSTV


## Build
**Required:**
* Java JDK 11
* Maven

**To build:**  
``mvn clean package``  
  
**To run:**  
``java -jar drtd/drtd.jar``, pass ``--help`` to show command line options