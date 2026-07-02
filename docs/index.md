---
title: Home
description: Home page of team 13 STMP project
icon: material/home
---

# Home

## Project Overview

The project is concerned with implementing geofencing, the following is a
definition of what is geofencing:

_Geofencing is a location-based technology that creates a virtual boundary around a specific, real-world geographic area. When a connected mobile device, vehicle, or asset crosses this invisible perimeter, it triggers a pre-programmed automated action like a notification, ad, or security alert.[^1]_

We'll be using Waveshare and the Tenstar device to produce geofencing, 
the Tenstar device is used to display our current location and construct a shape around our point. 
If we travel outside the boundary a status update is sent to github. 
We're able to connect this using wifi or mobile data. 
We'll enable the geofencing device to work without wifi so our data isn't lost.

## Achknowledgements
This was created by, 
**Matas Noreika**
**Cristina Sfîca**
**Christoph**
**Jiaqiang Chen**
**Nery Iwuala**

## Hardware

The following is a list of hardware required to reproduce this project:

* [Tenstar ESP32_S3](https://www.aliexpress.com/item/1005008500815036.html?pdp_npi=6%40dis%21EUR%2134.51%2116.56%21%21%21260.49%21125.04%21%402101611117824851243051027e1134%2112000045439248285%21affd%21%21%21%211%210%21&dp=EAIaIQobChMIkZT5ndKtlQMVEpRQBh0B6RXVEAQYASABEgIqy_D_BwE%7C0AAAAAC5ena2dpSa-XTXsm0ThMyIJ9SGpj%7CCj4KCAjwjIPSBhBeEi4AlMGF8ImMCW9-rdlgpwfgbAhnECoxjQbm6oXxxZG2ut4DdVZbyTpNGW1ZbxZdGgK-dw%7Cv1&cn=ie_a&gad_source=1&aff_fcid=09832cfa753144118bb522a3197f074f-1782777102033-07399-_onKPRpM&aff_fsk=_onKPRpM&aff_platform=api-new-product-query&sk=_onKPRpM&aff_trace_key=09832cfa753144118bb522a3197f074f-1782777102033-07399-_onKPRpM&terminal_id=dc9ddf851a9b4005b87484176445975d&afSmartRedirect=y)
* [Waveshare GNSS Hat](https://www.aliexpress.com/item/1005006039603955.html?pdp_npi=6%40dis%21EUR%2154.75%2149.28%21%21%2160.74%2154.67%21%402151fd2c17824830918857479e0f52%2112000035440298330%21affd%21%21%21%211%210%21&dp=EAIaIQobChMI_rjlrNKtlQMVlp1QBh1TkD3_EAQYAiABEgJ-WvD_BwE&cn=bravoie&gad_source=1&gad_campaignid=23465405014&gbraid=0AAAABClxYHR30p8V9fLPrW-M_wf1SCwvu&aff_fcid=9e8a287a2c8f4c4abd96638d29a11599-1782777134856-05175-_ooeXU2A&aff_fsk=_ooeXU2A&aff_platform=bops-tool&sk=_ooeXU2A&aff_trace_key=9e8a287a2c8f4c4abd96638d29a11599-1782777134856-05175-_ooeXU2A&terminal_id=dc9ddf851a9b4005b87484176445975d&afSmartRedirect=y)

## Software Overview
1. It creates a point and draws a circle around that point on the arduino, this is the boundary, it performs like a sonar
2. The first message it starts with is "[zone}: TUD ENTER"
3. The message will change to print out the location of where the the device is
4. If we exit this boundary it'll send an issue (message) to github saying "[zone}: TUD EXIT"
5. When you move around it updates the location of the device saying if you're in the zone or out of the zone.
6. At the end of the device it'll join all the points to create a shape
7. The data collected is sent and stored to the github website. 

## Testing
- We performed tests to test and setup the Waveshare and Tenstar device, We tested if the Tenstar was connected to the satilite by printing the latitude and longitude to the serial monitor, this was a success.
- Tested to see if the Tenstar and Waveshare would send a messsage to the github which was a success.
- Tested to print the location of where we are located which was a success.
- Tested the distance range of 500m, it was a success.

## Logbook
### Day 1
We decided on creating geofencing. We programed the Tenstar and Waveshare to calibrate our location using the satilite.
### Day 2
We drafted up code to create a program to send a message to github to test to see if the Tenstar is talking to the website.
### Day 3
We programmed our device to print a message on the Tenstar to tell us if we´re in or out of our boundary, if we´re outside it´ll send a message to github saying TUD: EXIT
### Day 4
Tested our project and created a box to fit our project into.

## References

[^1]: R. Awati, “What is geofencing and how is it used? – TechTarget Definition,” TechTarget, Dec. 21, 2022. [Online]. Available: <https://www.techtarget.com/whatis/definition/geofencing>. [Accessed: Jun. 30, 2026].
