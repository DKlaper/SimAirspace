This logs out which airspaces you are flying in!

Note to build this you need geographiclib for c++
and a postgres installation

Limitations: It will not report the CLASS A airspace above 18000 feet in the US, because it is not as such in the boundary data
It will not report on STADIUM TFRs as for those there are no shapefiles available.
It only downloads TFR data once a day. Definitely not 100% accurate but much better than nothing :D

Todo: 
P2 Make configurable which airspace it reports on based on simple values
P1 Proper DB setup installation/ starting db on startup
P3 Make it read stadium tfrs from the internet (this will not happen for a long while --- )


Used Libs:
unzip.exe -> infozip: bsd style custom license
postgres -> postgres: bsd style custom license
postgis -> gplv3 (but since we don't modify or distribute it no problem. That's why you need to install it manually)
