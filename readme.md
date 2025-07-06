# Intro
This project is for a Raspberry Pi Zero with a small E-Ink display, for general information.
It is primarily an educational project intended to learn how C works. 
Since learning OOP this has been a learning curve, and I have created some abominal form of OOP in order to keep this modular so I can improve it later.
Why did I not use C++, I don't know, I wanted to use C since my next year at university would require knowledge. 

# features
- Main Page - Clock, Weather icon, date
- Performance/temps (Planned)
- Method for checking other processes (Planned)
- Internet statistics - Only ping works currently.


# Setup
To setup the display on the Pi Zero (2w).
`sudo apt install python3-setuptools` 

# Dependencies
This project is intended to be as simple as possible, though when I got into the web stuff, I had to include a couple of external libraries.
- curl, this is required for making http requests
- json-c, this is used to interpret API responses from weatherAPI.com


https://www.waveshare.com/wiki/2.13inch_Touch_e-Paper_HAT_Manual#Overview
