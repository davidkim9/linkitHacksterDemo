#include <LGPS.h>

gpsSentenceInfoStruct info;
char buff[256];



#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

#define WIFI_AP "THE FARM WiFi"
#define WIFI_PASSWORD "organicfarm"
#define WIFI_AUTH LWIFI_WEP  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "hacksterdemo.azurewebsites.net"

LWiFiClient c;





//----------------------------------------------------------------------
//!\brief	return position of the comma number 'num' in the char array 'str'
//!\return  char
//----------------------------------------------------------------------
static unsigned char getComma(unsigned char num,const char *str){
	unsigned char i,j = 0;
	int len=strlen(str);
	for(i = 0;i < len;i ++){
		if(str[i] == ',')
			j++;
		if(j == num)
			return i + 1; 
		}
	return 0; 
}

//----------------------------------------------------------------------
//!\brief	convert char buffer to float
//!\return  float
//----------------------------------------------------------------------
static float getFloatNumber(const char *s){
	char buf[10];
	unsigned char i;
	float rev;

	i=getComma(1, s);
	i = i - 1;
	strncpy(buf, s, i);
	buf[i] = 0;
	rev=atof(buf);
	return rev; 
}

//----------------------------------------------------------------------
//!\brief	convert char buffer to int
//!\return  float
//----------------------------------------------------------------------
static float getIntNumber(const char *s){
	char buf[10];
	unsigned char i;
	float rev;

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
		float latitudetmp;
		float longitudetmp;
		tmp = getComma(2, GPGGAstr);
		latitudetmp = getFloatNumber(&GPGGAstr[tmp]);
		tmp = getComma(4, GPGGAstr);
		longitudetmp = getFloatNumber(&GPGGAstr[tmp]);
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
	}
	else{
		Serial.println("No GPS data"); 
	}
}

//----------------------------------------------------------------------
//!\brief	Convert GPGGA coordinates (degrees-mins-secs) to true decimal-degrees
//!\return  -
//----------------------------------------------------------------------
void convertCoords(float latitude, float longitude, double &lat_return, double &lon_return){
	int lat_deg_int = int(latitude/100);		//extract the first 2 chars to get the latitudinal degrees
	int lon_deg_int = int(longitude/100);		//extract first 3 chars to get the longitudinal degrees
    // must now take remainder/60
    //this is to convert from degrees-mins-secs to decimal degrees
    // so the coordinates are "google mappable"
    float latitude_float = latitude - lat_deg_int * 100;		//remove the degrees part of the coordinates - so we are left with only minutes-seconds part of the coordinates
    float longitude_float = longitude - lon_deg_int * 100;     
    lat_return = lat_deg_int + latitude_float / 60 ;			//add back on the degrees part, so it is decimal degrees
    lon_return = lon_deg_int + longitude_float / 60 ;
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
  
  LGPS.powerOn();
  Serial.println("LGPS Power on, and waiting ..."); 
  delay(3000);
  
  
  
  
//  
//  
//  // keep retrying until connected to website
//  Serial.println("Connecting to WebSite");
//  while (0 == c.connect(SITE_URL, 80))
//  {
//    Serial.println("Re-Connecting to WebSite");
//    delay(1000);
//  }
//
//  // send HTTP request, ends with 2 CR/LF
//  Serial.println("send HTTP GET request");
//  c.println("GET / HTTP/1.1");
//  c.println("Host: " SITE_URL);
//  c.println("Connection: close");
//  c.println();
//
//  // waiting for server response
//  Serial.println("waiting HTTP response:");
//  while (!c.available())
//  {
//    delay(100);
//  }
}

boolean disconnectedMsg = false;

void loop()
{
//  // Make sure we are connected, and dump the response content to Serial
//  while (c)
//  {
//    int v = c.read();
//    if (v != -1)
//    {
//      Serial.print((char)v);
//    }
//    else
//    {
//      Serial.println("no more content, disconnect");
//      c.stop();
//      while (1)
//      {
//        delay(1);
//      }
//    }
//  }
//
//  if (!disconnectedMsg)
//  {
//    Serial.println("disconnected by server");
//    disconnectedMsg = true;
//  }
//  delay(500);
  
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA);
  parseGPGGA((const char*)info.GPGGA);
  delay(2000);
}

