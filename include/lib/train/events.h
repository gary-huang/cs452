#ifndef EVENTS_H
#define EVENTS_H


/* ====== Raw Events (generated by real-life) ========= */

#include <user/train/train.h>
#include <lib/train/switch.h>
#include <lib/train/sensor.h>

typedef enum RawEventType {
  RE_NONE = 0,
  RE_SE   = 1,
  RE_SW   = 2,
  RE_TR_CMD = 3,
} RawEventType;

typedef Sensor RawSensorEvent;
typedef SWProtocol RawSwitchEvent;
typedef TrainProtocol RawTrainCmdEvent;

union RawEvents {
  RawTrainCmdEvent  tr_cmd_event;
  RawSensorEvent    se_event;
  RawSwitchEvent    sw_event;
};

typedef struct RawEvent {
  RawEventType type;
  int timestamp;
  union RawEvents event;
} RawEvent;


/* ====== Virtual Events (generated by model) ========= */


typedef enum VirtualEventType {
  VE_NONE = 0,
  VE_REG = 1,
  VE_TR_AT = 2,
} VirtualEventType;

typedef struct VirtualTrainAtEvent {
  int train_num;
  track_node *node;
} VirtualTrainAtEvent;

union VirtualEvents {
  VirtualTrainAtEvent train_at;
};

typedef struct VirtualEvent {
  VirtualEventType type;
  int timestamp;
  int timeout;    //TODO: HACK
  int key;
  RawEventType depend;
  union VirtualEvents event;
} VirtualEvent;


typedef enum EventGroupType {
  VRE_VE_RE,
  VRE_VE,
  VRE_RE,
  RE,
} EventGroupType;

typedef struct EventGroup {
  EventGroupType type;
  RawEvent       re;
  VirtualEvent   ve;
} EventGroup;

#endif