This logs out which airspaces you are flying in!

Note to build this you need geographiclib for c++
and a postgres installation

Limitations: It will not report the CLASS A airspace above 18000 feet in the US, because it is not as such in the boundary data

Todo: 
P2 Make configurable which airspace it reports on based on simple values
P3 Make proper tool for reading data -> read headers of all bgl then only read those with boundary sections
P3 Proper DB setup installation
P1 Make it read some TFRs online if internet is available ... 