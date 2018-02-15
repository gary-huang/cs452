#ifndef TRAIN_DEFINES_H
#define TRAIN_DEFINES_H

#define TRAIN_SIZE 81
#define SWITCH_SIZE 257
	#define NORMAL_SWITCH_SIZE_LOW 1
	#define NORMAL_SWITCH_SIZE_HIGH 18
	#define SPECIAL_SWITCH_SIZE_LOW 153
	#define SPECIAL_SWITCH_SIZE_HIGH 156
#define DECODER_SIZE 5
#define SENSOR_SIZE DECODER_SIZE*16

typedef enum TR_Command{
	TR_QUIT = -1,
	TR_NOTHING = 0,
	TR_TRAIN = 1,
	TR_REVERSE = 2,
	TR_SWITCH = 3,
	TR_FUNCTION = 4,
	TR_HORN = 5
} Command;

typedef enum TR_Direction{
	TR_FORWARD = -1,
	TR_BACKWARD = 1
} Direction;

typedef enum TR_Switch{
	TR_STRAIGHT = 33,
	TR_CURVE = 34
} Switch;

#endif //TRAIN_DEFINES_H