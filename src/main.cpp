#include <Arduino.h>
#include <stdlib.h>
#include <Wire.h>
#include <Adafruit_MCP9808.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MLX90640.h>


Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
Adafruit_LIS3DH accelsensor = Adafruit_LIS3DH();
Adafruit_MLX90640 tempcam = Adafruit_MLX90640();


int j=1;
bool a=0;
int s = 0;
float cameraFramebuf[32*24];

const byte MLX90640_address = 0x33;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while(!Serial);

  Serial.println("Sensor Module Test.\n");
  if(!tempcam.begin(MLX90640_I2CADDR_DEFAULT, &Wire))
  {
    Serial.println("temperature Camera Sensor not found.");
    a=1;
  }
  else
  {
    Serial.println("temperature Camera Sensor found.");
  }
  if(!accelsensor.begin(0x19))
  {
    Serial.println("Accelerometer Sensor not found.");
    a=1;
  }
  else
  {
    Serial.println("Accelerometer Sensor found.");
  }
  if(!tempsensor.begin(0x1A))
  {
    Serial.println("temperature Sensor not found.");
    a=1;
  }
  else
  {
    Serial.println("temperature Sensor found.\n");
  }
  if(a==1)
  {
    Serial.println("Sensor not fully functional, readings will not be displayed");
    exit(-1);
  }

  accelsensor.setRange(LIS3DH_RANGE_2_G);
  tempsensor.setResolution(3);
  tempcam.setMode(MLX90640_CHESS);
  tempcam.setResolution(MLX90640_ADC_18BIT);
  tempcam.setRefreshRate(MLX90640_8_HZ);
  Wire.setClock(1000000); //1Mhz

  Serial.print("Range level of the accelerometer: "); Serial.print(2 << accelsensor.getRange()); Serial.println("G\n");
  Serial.print("Resolution level of the temperature sensor: "); Serial.print(tempsensor.getResolution()); Serial.println(" (0.125 C)\n");
  Serial.print("Tempreature Camera Sensor Resolution: "); Serial.print(tempcam.getResolution()); Serial.print(" bits. Refresh Rate: "); Serial.print(tempcam.getRefreshRate()); Serial.println(" Hz.\n\n");

  Serial.println("Format of reading is: Number of the reading; Clock time at the start of the reading; temperature (from Sensor MCP9809); Acceleration on X, Y and Z axis (from Sensor LIS3DH); Termal Image data (from Termal Camera Sensor MLX90640)\n");

}

float printMatrix(float reading, int posh, int posw, int size) {
  int s=size;
  if(size <= 0) {
    Serial.println("Matrix needs to have size of at least 1.\n");
    return 0;
  }
  while(s!=0) {
    if(posh==0 && posw==0) {
      Serial.print(reading); Serial.print(" ");
      s--;
      break;
    }
    if(posh!=0 && posw==0) {
      Serial.println("\n");
      Serial.print(reading); Serial.print(" ");
      s--;
      break;
    }
    else {
      Serial.print(reading); Serial.print(" ");
      s--;
      break;
    }
  }

  if(s <= 1) {
    Serial.println("\n");
    return 0;
  }
}

void counter(void) {
  Serial.print(" #"); Serial.print(j); Serial.print(";  ");
}

void tempmesure(void) {
  tempsensor.wake();
  float tc = tempsensor.readTempC();
  Serial.print(tc, 3); Serial.print(" C;  "); 
}

void accelmesure(void) {
  sensors_event_t event;
  accelsensor.getEvent(&event);
  Serial.print("X: "); Serial.print(((float)event.acceleration.x), 3); Serial.print(" m/s^2  ");
  Serial.print("Y: "); Serial.print(((float)event.acceleration.y), 3); Serial.print(" m/s^2  ");
  Serial.print("Z: "); Serial.print(((float)event.acceleration.z), 3); Serial.print(" m/s^2;  ");
}

void cameraTemp(void) {

  tempcam.getFrame(cameraFramebuf);

  if(tempcam.getFrame(cameraFramebuf) != 0) {
    Serial.println("Frame aquisition failed.");
    return;
  }

  float maxtemp = 0;
  int pos;

  for(int fh=0; fh<24; fh++) { //fh = frame height
    for(int fw=0; fw<32; fw++) { //fw = Frame width

      float fp= cameraFramebuf[fw+fh*32];
    
      if(maxtemp<fp) {
        maxtemp = fp;
        pos = (fw+fh*32);
      }
      int CamSize = (32*24);
      // printMatrix(fp, fh, fw, CamSize); //Ver em casa pq dá um valor a mais na ultima linha.
    }
    

  }

  float temperature[9];
  float mesurment;

  temperature[4]=maxtemp;

  switch(pos) {
    case 0:
      temperature[5] = cameraFramebuf[pos+1];
      temperature[7] = cameraFramebuf[pos+32];
      temperature[8] = cameraFramebuf[pos+33];
      mesurment = (temperature[4]+temperature[5]+temperature[7]+temperature[8])/4;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel LTC: "); Serial.println(pos); Serial.println("; \n");
      return;
      break;

    case 31:
      temperature[3] = cameraFramebuf[pos-1];
      temperature[6] = cameraFramebuf[pos+32];
      temperature[7] = cameraFramebuf[pos+31];
      mesurment = (temperature[3]+temperature[4]+temperature[6]+temperature[7])/4;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel RTC: "); Serial.println(pos); Serial.println("; \n");
      return;
      break;

    case 736:
      temperature[1] = cameraFramebuf[pos-32];
      temperature[2] = cameraFramebuf[pos-31];
      temperature[5] = cameraFramebuf[pos+1];
      mesurment = (temperature[1]+temperature[2]+temperature[4]+temperature[5])/4;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel LBC: "); Serial.println(pos); Serial.println("; \n");
      return;
      break;

    case 767:
      temperature[0] = cameraFramebuf[pos-33];
      temperature[1] = cameraFramebuf[pos-32];
      temperature[3] = cameraFramebuf[pos-1];
      mesurment = (temperature[0]+temperature[1]+temperature[3]+temperature[4])/4;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel RBC: "); Serial.println(pos); Serial.println("; \n");
      return;
      break;
  }

  for(int eh=0; eh<32; eh++) { //eh = edge height

    if(pos==eh) {
      temperature[3] = cameraFramebuf[pos-1];
      temperature[5] = cameraFramebuf[pos+1];
      temperature[6] = cameraFramebuf[pos+31];
      temperature[7] = cameraFramebuf[pos+32];
      temperature[8] = cameraFramebuf[pos+33];
      mesurment = (temperature[3]+temperature[4]+temperature[5]+temperature[6]+temperature[7]+temperature[8])/6;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel TOP: "); Serial.println(pos); Serial.println("; \n");
      return;
    }

    if(pos==(736 + eh)) {
      temperature[0] = cameraFramebuf[pos-33];
      temperature[1] = cameraFramebuf[pos-32];
      temperature[2] = cameraFramebuf[pos-31];
      temperature[3] = cameraFramebuf[pos-1];
      temperature[5] = cameraFramebuf[pos+1];
      mesurment = (temperature[0]+temperature[1]+temperature[2]+temperature[3]+temperature[4]+temperature[5])/6;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel BTTM: "); Serial.println(pos); Serial.println("; \n");
      return;
    }
  }
  for(int ew=0; ew<24; ew++) { //ew = edge width
    if(pos==(ew*32)) {
      temperature[1] = cameraFramebuf[pos-32];
      temperature[2] = cameraFramebuf[pos-31];
      temperature[5] = cameraFramebuf[pos+1];
      temperature[7] = cameraFramebuf[pos+32];
      temperature[8] = cameraFramebuf[pos+33];
      mesurment = (temperature[1]+temperature[2]+temperature[4]+temperature[5]+temperature[7]+temperature[8])/6;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel LEFT: "); Serial.println(pos); Serial.println("; \n");
      return;
    }

    if(pos==(31 + ew*32)) {
      temperature[0] = cameraFramebuf[pos-33];
      temperature[1] = cameraFramebuf[pos-32];
      temperature[3] = cameraFramebuf[pos-1];
      temperature[6] = cameraFramebuf[pos+31];
      temperature[7] = cameraFramebuf[pos+32];
      mesurment = (temperature[0]+temperature[1]+temperature[3]+temperature[4]+temperature[6]+temperature[7])/6;
      Serial.print("Highest Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C \n");
      Serial.print("Highest Pixel RIGHT: "); Serial.println(pos); Serial.println("; \n");
      return;
    }
  }

  temperature[0] = cameraFramebuf[pos-33];
  temperature[1] = cameraFramebuf[pos-32];
  temperature[2] = cameraFramebuf[pos-31];
  temperature[3] = cameraFramebuf[pos-1];
  temperature[5] = cameraFramebuf[pos+1];
  temperature[6] = cameraFramebuf[pos+31];
  temperature[7] = cameraFramebuf[pos+32];
  temperature[8] = cameraFramebuf[pos+33];

  mesurment = (temperature[0]+temperature[1]+temperature[2]+temperature[3]+temperature[4]+temperature[5]+temperature[6]+temperature[7]+temperature[8])/9;

  Serial.print("Camera temperature average: "); Serial.print(mesurment, 2); Serial.println(" C;  \n");
  Serial.print("Highest Pixel MDDL: "); Serial.print(pos); Serial.println("; \n");
  

  return;
}

void loop() {
  counter();
  tempmesure();
  accelmesure();
  cameraTemp();
  j++;
}