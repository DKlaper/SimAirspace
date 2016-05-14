
CREATE EXTENSION postgis;

DROP TABLE IF EXISTS airspace;

CREATE TABLE airspace (
	id SERIAL PRIMARY KEY,
	name VARCHAR(150),
	astype integer, -- airspace type
	minMSLalt float,
	minAGLalt float,
	maxMSLalt float,
	maxAGLalt float,
	extent geography(POLYGON,4326)  NOT NULL

);

CREATE INDEX geogIDX ON Airspace USING GIST ( extent ); 
CREATE INDEX alt1 ON Airspace (minMSLalt);
CREATE INDEX alt2 ON Airspace (minAGLalt);
CREATE INDEX alt3 ON Airspace (maxMSLalt);
CREATE INDEX alt4 ON Airspace (maxAGLalt);
CREATE INDEX tp ON Airspace (astype);