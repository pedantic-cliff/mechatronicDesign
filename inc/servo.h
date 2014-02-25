#ifndef _SERVO_H_
#define _SERVO_H_

#define BROADCAST_ID 0xFE

#define STATE_ENABLE 0x01
#define STATE_DISABLE 0x00


// Define the external Servo Type
typedef struct servo *Servo; 


// Defines a Servo Type
typedef struct servo {
  int id; 
  int direction; 

  // Methods
  int (*setTorque)(Servo, char en);
} servo_t;

void initServos(void);
void createServo(Servo s, char ID, char direction); 

#endif
