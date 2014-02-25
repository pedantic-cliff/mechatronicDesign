#ifndef _SERVO_H_
#define _SERVO_H_

#define BROADCAST_ID  0xFE

#define STATE_ENABLE  0x01
#define STATE_DISABLE 0x00

#define SERVO_ID_LEFT   0x01
#define SERVO_ID_RIGHT  0x02

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
} Servo_t;

void initServos(void);
Servo createServo(Servo s, char ID, char direction); 

#endif
