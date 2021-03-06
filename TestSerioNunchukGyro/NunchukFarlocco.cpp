#include "NunchukFarlocco.h"
/*actual accel best value:
#define ACC_ZERO_X 484
#define ACC_ZERO_Y 540
#define ACC_ZERO_Z 310
*/
#define ACC_ZERO_X 484
#define ACC_ZERO_Y 530
#define ACC_ZERO_Z 405

int NunchukFarlocco::nunchuck_get_data(){
  int cnt=0;
  Wire.requestFrom (0x52, 6);	// request data from nunchuck
  while (Wire.available ()) {
    // receive byte as an integer
    nunchuck_buf[cnt] = nunchuk_decode_byte(Wire.receive());
    cnt++;
  }
  nunchuck_send_request();  // send request for next data payload
  // If we recieved the 6 bytes, then go elaborate them
  if (cnt >= 5) {
    nunchuk_elaborate_data();
    return 1;   // success
  }
  return 0; //failure
}

float gSquared=0, g, minE, maxE;
void NunchukFarlocco::nunchuk_elaborate_data(){
  int accel_x_axis = nunchuck_buf[2] * 2 * 2; 
  int accel_y_axis = nunchuck_buf[3] * 2 * 2;
  int accel_z_axis = nunchuck_buf[4] * 2 * 2;

  // byte nunchuck_buf[5] contains bits for z and c buttons
  // it also contains the least significant bits for the accelerometer data
  // so we have to check each bit of byte outbuf[5]
  /*
  if ((nunchuck_buf[5] >> 0) & 1) 
    z_button = 1;
  if ((nunchuck_buf[5] >> 1) & 1)
    c_button = 1;
  */
  if ((nunchuck_buf[5] >> 2) & 1) 
    accel_x_axis += 2;
  if ((nunchuck_buf[5] >> 3) & 1)
    accel_x_axis += 1;

  if ((nunchuck_buf[5] >> 4) & 1)
    accel_y_axis += 2;
  if ((nunchuck_buf[5] >> 5) & 1)
    accel_y_axis += 1;

  if ((nunchuck_buf[5] >> 6) & 1)
    accel_z_axis += 2;
  if ((nunchuck_buf[5] >> 7) & 1)
    accel_z_axis += 1;
  
  x=accel_x_axis-ACC_ZERO_X;
  y=accel_y_axis-ACC_ZERO_Y;
  z=accel_z_axis-ACC_ZERO_Z;
  /*
  float t = x*x+y*y+z*z;  
  Serial.print( "t:" );
  Serial.print( sqrt(t) );
  
  Serial.print("x:");
  Serial.print(x);
  Serial.print("y:");
  Serial.print(y);
  Serial.print("z:");
  Serial.println(z);  
  */
  /*
  if (gSquared==0){
    g = x*x+y*y+z*z;
    gSquared=sqrt(g);
    minE = g-(g/5);
    maxE = g+(g/5);
  }else{
    float t = x*x+y*y+z*z;
    
    /*
    Serial.print( "t:" );
    Serial.print( sqrt(t) );
    Serial.print(" should be:");
    Serial.println( gSquared );
    
    if (t!=g){
      if (t >= minE && t <= maxE){
        double phi = atan2(y, x);
        double theta = acos( z/sqrt(t) );
        x=gSquared*sin(theta)*cos(phi);
        y=gSquared*sin(theta)*sin(phi);
        z=gSquared*cos(theta);
        /*
        Serial.print("old distance:");
        Serial.print( sqrt(t) );
        Serial.print(" new distance:");
        Serial.print( sqrt(x*x+y*y+z*z) );
        Serial.print(" should be:");
        Serial.println( gSquared );
        
        
      }else{
        /*
        Serial.print( "acc no good data; t:" );
        Serial.print( sqrt(t) );
        Serial.print(" should be:");
        Serial.println( gSquared );
        
        
      }
    }
  }
  */
}

// Encode data to format that most wiimote drivers except
// only needed if you use one of the regular wiimote drivers
char NunchukFarlocco::nunchuk_decode_byte (char x){
  x = (x ^ 0x17) + 0x17;
  return x;
}

void NunchukFarlocco::nunchuck_send_request(){
  Wire.beginTransmission(0x52);	// transmit to device 0x52
  Wire.send(0x00);		// sends one byte
  Wire.endTransmission();	// stop transmitting
}

void NunchukFarlocco::nunchuck_init(){
    /*
    //
    // FOR ORIGINAL NUNCHUCK
    //
  Wire.begin();	                // join i2c bus as master
  Wire.beginTransmission(0x52);	// transmit to device 0x52
  Wire.send(0x40);		// sends memory address
  Wire.send(0x00);		// sends sent a zero.  
  Wire.endTransmission();	// stop transmitting
    //
    // END FOR ORIGINAL NUNCHUCK
    //
  */
  
  //
  // FOR FAKE NUNCHUCK
  //
  byte cnt;
  //Serial.print ("Begin3\n");
  Wire.begin();
//  Serial.print ("Begin4\n");
  // init controller
  delay(1);
  Wire.beginTransmission(0x52);	// device address
//  Serial.print ("Begin40\n");
  Wire.send(0xF0);		        // 1st initialisation register
//  Serial.print ("Begin40\n");
  Wire.send(0x55);		        // 1st initialisation value
  Serial.print ("Begin40\n");
  Wire.endTransmission();
  Serial.print ("Begin41\n");
  delay(1);
  Wire.beginTransmission(0x52);
  Wire.send(0xFB);		        // 2nd initialisation register
  Wire.send(0x00);		        // 2nd initialisation value
  Serial.print ("Begin42\n");
  Wire.endTransmission();
  delay(1);
  Serial.print ("Begin5\n");
  // read the extension type from the register block        
  Wire.beginTransmission(0x52);
  Wire.send(0xFA);		        // extension type register
  Wire.endTransmission();
  Wire.beginTransmission(0x52);
  Wire.requestFrom(0x52, 6); 	        // request data from controller
  Serial.print ("Begin6\n");
  for (cnt = 0; cnt < 6; cnt++) {
    if (Wire.available()) {
        ctrlr_type[cnt] = Wire.receive(); // Should be 0x0000 A420 0101 for Classic Controller, 0x0000 A420 0000 for nunchuck
    }
    Serial.print ("Begin7\n");
  }
  Wire.endTransmission();
  delay(1);
            
  // send the crypto key (zeros), in 3 blocks of 6, 6 & 4.
  Wire.beginTransmission(0x52);
  Wire.send(0xF0);		        // crypto key command register
  Wire.send(0xAA);		        // sends crypto enable notice
  Wire.endTransmission();
  delay(1);
  Wire.beginTransmission(0x52);
  Wire.send(0x40);		        // crypto key data address
  for (cnt = 0; cnt < 6; cnt++) {
    Wire.send(0x00);		        // sends 1st key block (zeros)
  }
  Wire.endTransmission();
  Wire.beginTransmission(0x52);
  Wire.send(0x40);		        // sends memory address
  for (cnt = 6; cnt < 12; cnt++) {
    Wire.send(0x00);		        // sends 2nd key block (zeros)
  }
  Wire.endTransmission();
  Wire.beginTransmission(0x52);
  Wire.send(0x40);		        // sends memory address
  for (cnt = 12; cnt < 16; cnt++) {
    Wire.send(0x00);		        // sends 3rd key block (zeros)
  }
  Wire.endTransmission();
  delay(1);
  //end device init 
  
  //
  // END FAKE NUNCHUCK
  //
  
  nunchuck_send_request();
}
