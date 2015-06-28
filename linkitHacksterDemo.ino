#include <LGPS.h>

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

#define WIFI_AP "THE FARM WiFi"
#define WIFI_PASSWORD "organicfarm"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "hacksterdemo.azurewebsites.net"
#define SITE_PATH "gps"

gpsSentenceInfoStruct info;
char buff[256];

LWiFiClient c;



//GPS STUFF
	//Meters
	double minUpdateDistance = 25;
	double lastLon = 0;
	double lastLat = 0;
	const float Pi = 3.14159;
	double toRad(double angle){
	    return angle * PI/180;
	}
	static double milesApart(double lon1, double lat1, double lon2, double lat2){
	    double R = 6371000; // meters
	    double phi1 = toRad(lat1);
	    double phi2 = toRad(lat2);
	    double dPhi = toRad(lat2-lat1);
	    double dLambda = toRad(lon2-lon1);

	    double a = sin(dPhi/2) * sin(dPhi/2) + cos(phi1) * cos(phi2) * sin(dLambda/2) * sin(dLambda/2);
	    double c = 2 * atan2(sqrt(a), sqrt(1-a));

	    double d = R * c;

	    return d;
	}

	static unsigned char getComma(unsigned char num,const char *str)
	{
	  unsigned char i,j = 0;
	  int len=strlen(str);
	  for(i = 0;i < len;i ++)
	  {
	     if(str[i] == ',')
	      j++;
	     if(j == num)
	      return i + 1; 
	  }
	  return 0; 
	}

	static double getDoubleNumber(const char *s)
	{
	  char buf[10];
	  unsigned char i;
	  double rev;
	  
	  i=getComma(1, s);
	  i = i - 1;
	  strncpy(buf, s, i);
	  buf[i] = 0;
	  rev=atof(buf);
	  return rev; 
	}

	static double getIntNumber(const char *s)
	{
	  char buf[10];
	  unsigned char i;
	  double rev;
	  
	  i=getComma(1, s);
	  i = i - 1;
	  strncpy(buf, s, i);
	  buf[i] = 0;
	  rev=atoi(buf);
	  return rev; 
	}

	double latitude;
	double longitude;
	int tmp, hour, minute, second, num, fix, latitude_dir, longitude_dir;
	void parseGPGGA(const char* GPGGAstr){
		if(GPGGAstr[0] == '$'){
			int tmp;
			tmp = getComma(1, GPGGAstr);
			hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
			minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
			second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');

			//get time
			sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
			Serial.print(buff);
			//get lat/lon coordinates
			double latitudetmp;
			double longitudetmp;
			tmp = getComma(2, GPGGAstr);
			latitudetmp = getDoubleNumber(&GPGGAstr[tmp]);
			tmp = getComma(4, GPGGAstr);
			longitudetmp = getDoubleNumber(&GPGGAstr[tmp]);
			// need to convert format
			convertCoords(latitudetmp, longitudetmp, latitude, longitude);
			//get lat/lon direction
			tmp = getComma(3, GPGGAstr);
			latitude_dir = (GPGGAstr[tmp]);
			tmp = getComma(5, GPGGAstr);		
			longitude_dir = (GPGGAstr[tmp]);
			
			sprintf(buff, "latitude = %10.4f-%c, longitude = %10.4f-%c", latitude, latitude_dir, longitude, longitude_dir);
			Serial.println(buff); 

			//get GPS fix quality
			tmp = getComma(6, GPGGAstr);
			fix = getIntNumber(&GPGGAstr[tmp]);    
			sprintf(buff, "  -  GPS fix quality = %d", fix);
			Serial.print(buff);   
			//get satellites in view
			tmp = getComma(7, GPGGAstr);
			num = getIntNumber(&GPGGAstr[tmp]);    
			sprintf(buff, "  -  %d satellites", num);
			Serial.println(buff);

			if(milesApart(lastLat, lastLon, latitude, longitude) > minUpdateDistance){
				sprintf(buff, "%10.4f", longitude);
				String strLon = String(buff);
				if(longitude_dir == 'W') {
					strLon = "-" + strLon;
				}
				sprintf(buff, "%10.4f", latitude);
				String strLat = String(latitude);
				if(latitude_dir == 'S') {
					strLat = "-" + strLat;
				}
				updateWeb(strLon, strLat);

				lastLat = latitude;
				lastLon = longitude;

			}
		}
		else{
			Serial.println("No GPS data"); 
		}
	}

	void convertCoords(double latitude, double longitude, double &lat_return, double &lon_return){
		int lat_deg_int = int(latitude/100);
		int lon_deg_int = int(longitude/100);
		double latitude_double = latitude - lat_deg_int * 100;
		double longitude_double = longitude - lon_deg_int * 100;
		lat_return = lat_deg_int + latitude_double / 60 ;
		lon_return = lon_deg_int + longitude_double / 60 ;
	}

//GPS STUFF END


void updateWeb(String lon, String lat){

	Serial.println("Sending value to Dude where's my car...");

	int attempts = 3;
	while (!c.connect(SITE_URL, 80) && attempts > 0)
	{
		Serial.println("Retrying to connect...");
		delay(100);
		attempts--;
		if(attempts == 0) return;
	}
	String deviceId = "Device1";
	String data = "{\"device\":\"" + deviceId + "\", \"lon\":\"" + lon + "\", \"lat\":\"" + lat + "\"}";
	String thisLength = String(data.length());

	
	c.print("POST /gps");
	c.println(" HTTP/1.1");
	c.println("Content-Type: application/json");
	c.println("Content-Length: " + thisLength);
	c.print("Host: ");
	c.println(SITE_URL);
	c.print("\n" + data);
	c.print(char(26));

	// read server response
	// while (c){
	// 	Serial.print((char)c.read());
	// }

	c.stop();
}

void setup()
{
	LWiFi.begin();
	Serial.begin(115200);

  // keep retrying until connected to AP
	Serial.println("Connecting to AP");
	while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
	{
		delay(1000);
	}
	// Serial.println("Attach to GPRS network by auto-detect APN setting");
	// while (!LGPRS.attachGPRS()) {
	// 	delay(500);
	// }

	Serial.println("Connected to wifi");
	LGPS.powerOn();
	Serial.println("LGPS Power on, and waiting ..."); 
	delay(3000);
}

void loop()
{
	LGPS.getData(&info);
	Serial.println((char*)info.GPGGA);
	parseGPGGA((const char*)info.GPGGA);
	delay(3000);
}

