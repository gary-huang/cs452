#ifndef UPDATES_H
#define UPDATES_H

#include <lib/train/train.h>
#include <lib/train/estimator.h>

#define MAX_NUM_UPDATES 10

typedef enum TrackEventType {
  TE_TR_MOVE,
  TE_TR_DATA,
  TE_TR_POSITION,
  TE_SW_CHANGE,
  TE_SE_CHANGE,
  TE_TR_SPEED,
  TE_TR_STATUS,
  MAX_TRACK_EVENT  // NOTE: keep as last element in enum
} TrackEventType;

typedef enum TrackDataType{
  TD_ALL,
  TD_TR,
  TD_TR_A,
  TD_TR_L,
  TD_SW,
  TD_SE
} TrackDataType;


typedef struct TrackEventTrainGearChange {
  int num;
  int newgear;
} TETRGear;

typedef struct TrackEventSwitchChange {
  int num;
  int newdir;
} TESWChange;

typedef struct TrackEventSensorChange {
  int num;
  int state;
} TESEChange;

typedef struct TrackEventTrainSpeed {
  int num;
  int old;
  int new;
} TETRSpeed;

typedef struct TrackEventTrainPosition {
  int num;
  track_node *node;
} TETRPosition;

typedef struct TrackEventTrainStatus {
  int num;
  TrainStatus old;
  TrainStatus new;
} TETRStatus;

union TrackEvents {
  TETRPosition tr_pos;
  TETRGear   tr_gear;
  TESWChange sw_change;
  TESEChange se_change;
  TETRSpeed  tr_speed;
  TETRStatus tr_status;
};

typedef struct TrackEvent {
  TrackEventType type;
  int ts;
  union TrackEvents event;
} TrackEvent;

typedef struct TrackUpdate {
  TrackEvent events[MAX_NUM_UPDATES];
  int num;
} TrackUpdate;

//============================================

typedef enum TDType {
  TD_TR_TRAIN,
  TD_SW_SWITCH,
  MAX_TRACK_DATA  // NOTE: keep as last element in enum
} TDType;

union TrackDatas{
  train tr_train;
  swi   *sw_switch;
};

typedef struct TrackData{
  TDType type;
  union TrackDatas data;
} TrackData;

#endif
