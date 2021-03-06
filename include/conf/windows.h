#ifndef WINDOWS_INTERFACE_H
#define WINDOWS_INTERFACE_H

// logging window config
#define LOG_OFF_X  72
#define LOG_OFF_Y   1
#define LOG_WIDTH     40
#define LOG_HEIGHT    60

#define CAL_OFF_X  72
#define CAL_OFF_Y  43
#define CAL_WIDTH     40
#define CAL_HEIGHT    2


// timer window config
#define TIME_OFF_X  2
#define TIME_OFF_Y  1
#define TIME_WIDTH     10
#define TIME_HEIGHT    2

#define NPROCS_OFF_X   TIME_OFF_X + TIME_WIDTH + 1
#define NPROCS_OFF_Y   1
#define NPROCS_WIDTH   15
#define NPROCS_HEIGHT  2

#define MEM_OFF_X NPROCS_OFF_X + NPROCS_WIDTH + 1
#define MEM_OFF_Y      1
#define MEM_WIDTH      10
#define MEM_HEIGHT     2

// shell window config
#define SH_OFF_X  2
#define SH_OFF_Y  4
#define SH_WIDTH    69
#define SH_HEIGHT    9

// idle
#define IDLE_OFF_X  MEM_OFF_X + MEM_WIDTH + 1
#define IDLE_OFF_Y  1
#define IDLE_WIDTH    11
#define IDLE_HEIGHT   2

// sensor window config
#define SENSOR_OFF_X  IDLE_OFF_X + IDLE_WIDTH + 1
#define SENSOR_OFF_Y  1
#define SENSOR_WIDTH    19
#define SENSOR_HEIGHT   2


// sensor window config
#define M1_RESET_OFF_X   SH_OFF_X
#define M1_RESET_OFF_Y   SH_OFF_Y + SH_HEIGHT + 1
#define M1_RESET_WIDTH   SH_WIDTH
#define M1_RESET_HEIGHT  10

#define TRACK_OFF_X SH_OFF_X
#define TRACK_OFF_Y SH_OFF_Y + SH_HEIGHT + 1
#define TRACK_WIDTH IDLE_OFF_X + IDLE_WIDTH - 11
#define TRACK_HEIGHT 15

// sensor window config
#define SWITCH_OFF_X  TRACK_OFF_X + TRACK_WIDTH + 1
#define SWITCH_OFF_Y  TRACK_OFF_Y
#define SWITCH_WIDTH  28
#define SWITCH_HEIGHT 29

// task manager
// #define TASK_OFF_X  1
// #define TASK_OFF_Y  TRACK_OFF_Y + TRACK_HEIGHT + 2
// #define TASK_WIDTH  SH_WIDTH
// #define TASK_HEIGHT 11
#define TASK_OFF_X  LOG_OFF_X + LOG_WIDTH + 1
#define TASK_OFF_Y  LOG_OFF_Y
#define TASK_WIDTH  24
#define TASK_HEIGHT LOG_HEIGHT

// trains
#define TRAIN_OFF_X  SH_OFF_X
#define TRAIN_OFF_Y  TRACK_OFF_Y + TRACK_HEIGHT -10
#define TRAIN_WIDTH  TRACK_WIDTH
#define TRAIN_HEIGHT 10

#define DRIVER1_OFF_X  SH_OFF_X
#define DRIVER1_OFF_Y  SWITCH_OFF_Y + SWITCH_HEIGHT + 1
#define DRIVER1_WIDTH  SH_WIDTH/2
#define DRIVER1_HEIGHT 10

#define DRIVER2_OFF_X  DRIVER1_OFF_X + DRIVER1_WIDTH
#define DRIVER2_OFF_Y  SWITCH_OFF_Y + SWITCH_HEIGHT + 1
#define DRIVER2_WIDTH  SH_WIDTH/2
#define DRIVER2_HEIGHT 10

#define REP_OFF_X  SH_OFF_X
#define REP_OFF_Y  TRAIN_OFF_Y + TRAIN_HEIGHT + 1
#define REP_WIDTH  TRACK_WIDTH
#define REP_HEIGHT NUM_TRAINS + 3

#define RESERV_OFF_X  SH_OFF_X
#define RESERV_OFF_Y  LOG_OFF_Y + LOG_HEIGHT + 1
#define RESERV_WIDTH  TASK_OFF_X + TASK_WIDTH - 2
#define RESERV_HEIGHT 3

#endif
