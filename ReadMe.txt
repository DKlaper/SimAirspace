SimAirspace is an FSX add-on that reports when you enter or exit specific controlled airspace. As an addition it also reports while you are flying inparticularly restricted airspace that requires additional caution or where flying might even be prohibited. This all is based on the data delivered with FSX. In addition it also downloads TFRs from the faa's site and reports if you fly into them. Note that Stadium TFRs around major league games or races are not currently supported, because the FAA does not publish shapefiles for them.

The idea really is that you can verify your flightplanning, avoid prohibited areas and be more aware of what kind of airspace you are flying through. For me it is mainly an addition to compensate poor VFR support in many ATC addons. This makes me request clearance for entering class B airspace or read up on restricted areas.


Installation:
A slight warning: Due to license concerns (I want my software to be reusable even in a commercial product) it involves some manual installation steps.
1. Unzip the data into it's own folder let's say C:\SimAirspace
2. Configure paths (TODO need to be made configurable)
3. Download postgis from here http://download.osgeo.org/postgis/windows/pg95/
4. unzip it into the postgres folder in your SimAirspace folder so the structure matches (shp2pgsql.exe must be in SimAirspace/postgres/bin).
(5a set up postgres db with superuser SimAirspace, db name simairspacedb, and port 18200, also run the createTables.sql. This step will later be automated with step 5)
5. Run the initialization, open a command line and enter 'SimAirspace.exe aspinit', this will in the future create all the db. Currently it just parses the bgl files for airspace data and writes it into the db.
6. Put the path to SimAirspace.exe into your exe.xml in your AppData/Roaming/Microsoft/FSX folder. This makes sure the add-on is started when fsx is started
7. It might take a bit when you start the add-on before it starts reporting as it tries to download the current TFR data from the internet. (Later you will be able to disable this download and update so if you have no internet, that there's no delay.)

Then try to start somewhere close to a MOA or Restricted airspace and see if you get text messages when you enter it on top of your FSX screen.

Technical Basics:
It is based on the data in FSX in BNXWorld0.bgl, BNXWorld1.bgl and bvcf.bgl, it parses those files (currently the names and locations are hardcoded), writes them in a postgres/postgis database. Also it uses the Nasa's combined TFR shapefile to track TFRs those are directly imported into the database as well if your current data is older than 12 hours.


Limitations: It will not report the CLASS A airspace above 18000 feet in the US, because it is not as such in the boundary data
It will not report on STADIUM TFRs as for those there are no shapefiles available.
It only downloads TFR data once a day. Definitely not 100% accurate but much better than nothing :D

Todo: 
P2 Make configurable which airspace it reports on based on simple values
P1 Proper DB setup installation/ starting db on startup
P3 Make it read stadium tfrs from the internet (this will not happen for a long while. It would require a database of stadiums and schedules like skyvector.com uses for its game TFRs --- )

