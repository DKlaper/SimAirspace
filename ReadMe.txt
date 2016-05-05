This logs out which airspaces you are flying in!

Note to build this you need geographiclib for c++
and a postgres installation

Limitations: It will not report the CLASS A airspace above 18000 feet in the US, because it is not coded as such in the corrsponding files.
Rather it is hard coded.

Todo: 
P3 Make configurable which airspace it reports on
P1 Properly report in game via text
P2 Make proper tool for reading data -> read headers of all bgl then only read those with boundary sections
P2 Proper DB setup installation
P2 Make Gui instead of console
P3 configurable setup for what it reports on
P3 Publish simconnect events