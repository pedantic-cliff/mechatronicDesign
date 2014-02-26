#ifndef _SERVO_H_
#define _SERVO_H_

#define BROADCAST_ID  0xFE

#define STATE_ENABLE  0x01
#define STATE_DISABLE 0x00

#define SERVO_ID_LEFT   0xFE
#define SERVO_ID_RIGHT  0xFE

#define DIRECTION_FORWARD ( 1)
#define DIRECTION_REVERSE (-1)

// Define the external Servo Type
typedef struct servo *Servo; 


// Defines a Servo Type
typedef struct servo {
  int id; 
  int direction; 

  // Methods
  int (*setTorque)(Servo, char en);
  int (*setSpeed)(Servo, int speed);
  int (*setServo2WheelMode)(Servo s);
  int (*toggleServoLed)(Servo s,int LEDID,int en);
} Servo_t;

void initServos(void);
Servo createServo(Servo s, char ID, char direction); 

#endif
