#include <user/train/waiting_room.h>
#include <terminal_manager.h>
#include <lib/train/event_window.h>
#include <track_data.h> //TODO: REMOVE THIS

CIRCULAR_BUFFER_DEF(eg_cb, EventGroup, EVENT_GROUP_BUFFER_SIZE);
CIRCULAR_BUFFER_DEF(ve_key_cb, int, VE_KEY_BUFFER_SIZE);

void SendSensorDelta(tid_t wr_tid, tid_t cs_tid, tid_t my_tid, char *new_sensors, char *old_sensors) {
  int i, j, r;
  WRRequest event;
  char new_dec;
  char old_dec;

  event.type = WR_RE;
  event.data.re.type = RE_SE;
  event.data.re.timestamp = Time(cs_tid, my_tid);

  // looping over decoders A->E
  for (i = 0; i < DECODER_SIZE*2; ++i) {
    new_dec = new_sensors[i];
    old_dec = old_sensors[i];

    if (new_dec == old_dec) continue;

    // looping over bits of decoder byte
    for (j = 0; j < 8; ++j) {
      if (GET_SENSOR(new_dec, j) != GET_SENSOR(old_dec, j)) {
        event.data.re.event.se_event.id = i*8 + (7-j);
        event.data.re.event.se_event.state = GET_SENSOR(new_dec, j);
        Send(wr_tid, &event, sizeof(event), &r, sizeof(r));
      }
    }
  }
}

static void SensorSubscriber() {
  int r;
  SNSubscribe sn_sub;
  Sensors event, prev;
  tid_t sn_pub, wr_tid, cs_tid, my_tid;

  sn_pub = WhoIs(SENSOR_PUBLISHER_ID);
  assert(sn_pub > 0);

  cs_tid = WhoIs(CLOCKSERVER_ID);
  assert(cs_tid > 0);

  wr_tid = MyParentTid();
  assert(wr_tid > 0);

  my_tid = MyTid();
  assert(my_tid > 0);  

  sn_sub.snr = SN_SUBSCRIBE;

  //TODO: INITIALIZE SENSORS
  init_sensors(&event);
  init_sensors(&prev);

  //Initial Subscribe
  Send(sn_pub, &sn_sub, sizeof(sn_sub), &r, sizeof(r));

  while (true) {
    //Send(sn_pub, &sn_sub, sizeof(sn_sub), &event, sizeof(event));
    Receive(&sn_pub, &event, sizeof(event));
    Reply(sn_pub, &r, sizeof(r));
    SendSensorDelta(wr_tid, cs_tid, my_tid, event.sensors, prev.sensors);
    memcpy(&prev, &event, sizeof(event));
  }

  Exit();
}


static void SwitchSubscriber() {
  int r;
  SWSubscribe sw_sub;
  WRRequest event;
  tid_t sw_pub, wr_tid;

  sw_pub = WhoIs(SWITCH_PUBLISHER_ID);
  assert(sw_pub > 0);

  wr_tid = MyParentTid();
  assert(wr_tid > 0);

  sw_sub.swr = SW_SUBSCRIBE;
  event.type = WR_RE;

  //Initial Subscribe
  Send(sw_pub, &sw_sub, sizeof(sw_sub), &r, sizeof(r));

  while (true) {
    //Send(sw_pub, &sw_sub, sizeof(sw_sub), &event.data.re, sizeof(RawEvent));
    Receive(&sw_pub, &event.data.re, sizeof(RawEvent));
    Reply(sw_pub, &r, sizeof(r));
    Send(wr_tid, &event, sizeof(event), &r, sizeof(r));
  }

  Exit();
}

static void TrainSubscriber() {
  int r;
  TSubscribe tsub;
  WRRequest event;
  tid_t tr_pub, wr_tid;

  tr_pub = WhoIs(TRAIN_PUBLISHER_ID);
  assert(tr_pub > 0);

  wr_tid = MyParentTid();
  assert(wr_tid > 0);

  tsub.tc = T_SUBSCRIBE;
  event.type = WR_RE;

  //Intial Subscribe
  Send(tr_pub, &tsub, sizeof(tsub), &r, sizeof(r));

  while (true) {
    //Send(tr_pub, &tsub, sizeof(tsub), &event.data.re, sizeof(RawEvent));
    Receive(&tr_pub, &event.data.re, sizeof(RawEvent));
    Reply(tr_pub, &r, sizeof(r));
    Send(wr_tid, &event, sizeof(event), &r, sizeof(r));
  }

  Exit();
}

static void VirtualEventSubscriber(){
  int r;
  VESubscribe vsub;
  WRRequest event;
  tid_t ve_pub, wr_tid;

  ve_pub = WhoIs(VIRTUAL_PUBLISHER_ID);
  assert(ve_pub > 0);

  wr_tid = MyParentTid();
  assert(wr_tid > 0);

  vsub.type = VER_SUBSCRIBE;
  event.type = WR_VE;
  
  //Initial Subscribe
  Send(ve_pub, &vsub, sizeof(vsub), &r, sizeof(r));

  while(true){
    //Send(ve_pub, &vsub, sizeof(vsub), &event.data.ve, sizeof(VirtualEvent));
    Receive(&ve_pub, &event.data.ve, sizeof(event.data.ve));
    Reply(ve_pub, &r, sizeof(r));
    Send(wr_tid, &event, sizeof(event), &r, sizeof(r));
  }

  Exit();
}

// ------------- WAITING ROOM SERVER ---------------- //

static void reset_waiting_room(VirtualEvent *ve){
  ve->type = VE_NONE;
}

void TimeoutWR_VE(void *args){
  WRRequest wrr;
  int r;

  tid_t cs_tid = WhoIs(CLOCKSERVER_ID);
  tid_t my_tid = MyTid();
  tid_t wr_tid = MyParentTid();
  tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  assert(cs_tid > 0 && my_tid > 0 && wr_tid > 0 && tm_tid > 0);

  wrr.type = WR_TO;
  wrr.data.ve = *(VirtualEvent *) args;

  // TMLogStrf(tm_tid, "Timestart for location %s\n", wrr.data.ve.event.train_at.node->name);
  Delay(cs_tid, my_tid, wrr.data.ve.timeout);
  // TMLogStrf(tm_tid, "Timeout for location %s\n", wrr.data.ve.event.train_at.node->name);

  Send(wr_tid, &wrr, sizeof(wrr), &r, sizeof(r));
  Exit();
}

static void handle_ve_tr_at(VirtualEvent ve, VirtualEvent *waiting, ve_key_cb *sensorToVE){
  int key;

  //key = ve.key;
  key = ve.event.train_at.train_num * MAX_OUTSTANDING_EVENT + ve.key;
  assert(key >= 0 && key < MAX_OUTSTANDING_EVENT * MAX_LIVE_TRAINS);

  tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  assert(tm_tid > 0);

  if(waiting[key].type == VE_REG){
    //Spawn a timeout handler  
    CreateArgs(27, &TimeoutWR_VE, (void *)&ve, sizeof(VirtualEvent)); //TODO: UPDATE PRIORITY    
  }else{
    // TMLogStrf(tm_tid, "Already handled location %s\n", ve.event.train_at.node->name);    
  }
}

static void add_train_unique_registration(ve_key_cb *cb, VirtualEvent *waiting, VirtualEvent ve, eg_cb *dataBuf, int*liveMap){
  int base, i, reged, r, train_num, key;
  EventGroup eg;

  train_num = ve.event.train_at.train_num;
  key = train_num * KEY_MAX + ve.key;
  base = train_num * KEY_MAX;

  for(i = 0; i < cb->size; ++i){
    r = ve_key_cb_pop(cb, &reged);
    assert(r != CB_E_EMPTY);
    if(!((reged - base) >= 0 && (reged - base) < KEY_MAX)){
      r = ve_key_cb_push(cb, reged);
      assert(r != CB_E_FULL);
    }
    else{
      //Removed the older version (manually time it out)
      tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
      TMLogStrf(tm_tid, "REMOVED PREVIOUS REG\n");
      eg.type = VRE_VE;
      eg.ve = waiting[reged];
      eg.ve.event.train_at.train_num = liveMap[eg.ve.event.train_at.train_num];
      r = eg_cb_push(dataBuf, eg);
      assert(r != CB_E_FULL);
      assert(reged >= 0 && reged < MAX_LIVE_TRAINS * MAX_OUTSTANDING_EVENT);
      reset_waiting_room(&waiting[reged]);
    }
  }

  r = ve_key_cb_push(cb, key); //Push the new key
  assert(r != CB_E_FULL);
}

static void handle_ve_reg(VirtualEvent ve, VirtualEvent *waiting, ve_key_cb *sensorToVE, eg_cb *dataBuf, int* liveMap /*TODO: REMOVE*/){
  int r;
  int sensor = ve.event.train_at.node->num;
  int key = ve.event.train_at.train_num * MAX_OUTSTANDING_EVENT + ve.key;

  tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  assert(tm_tid > 0);

  assert(sensor >= 0 && sensor < SENSOR_SIZE);
  assert(key >= 0 && key < MAX_LIVE_TRAINS * MAX_OUTSTANDING_EVENT);

  //r = ve_key_cb_push(&sensorToVE[sensor], key);
  //assert(r != CB_E_FULL);
  add_train_unique_registration(&sensorToVE[sensor], waiting, ve, dataBuf, liveMap);
  if(waiting[key].type != VE_NONE){
    assert(0 && "overwritten");
  }
  waiting[key] = ve;
  TMLogStrf(tm_tid, "REG %d on %s\n", ve.key, ve.event.train_at.node->name);
}

void HandleWR_VE(WRRequest *event, VirtualEvent *waiting, ve_key_cb *sensorToVE, eg_cb *dataBuf, int* liveMap/*TODO: REMOVE*/){
  switch(event->data.ve.type){
    case VE_TR_AT:
      handle_ve_tr_at(event->data.ve, waiting, sensorToVE);
      break;
    case VE_REG:
      handle_ve_reg(event->data.ve, waiting, sensorToVE, dataBuf, liveMap);
      break;
    default:
      assert(0 && "Bad Virtual Event");
  }
}

static int pop_earliest_ts(ve_key_cb *sensor, VirtualEvent *waiting, int *key){
  int i,r, max_ts, max_key;
  max_ts = -1;
  max_key = -1;

  if(sensor->size == 0) return -1;

  //Find the next virtual event
  for(i = 0; i < sensor->size; i++){
    r = ve_key_cb_pop(sensor, key);
    //Error checking
    if(r) return r;
    if(*key < 0 || *key >= MAX_LIVE_TRAINS * MAX_OUTSTANDING_EVENT) return -2;
    if(waiting[*key].type == VE_NONE) return -3;

    //Update the closest virtual event
    if(waiting[*key].timestamp < max_ts || max_ts == -1){
      max_key = *key;
    }

    //Push back
    r = ve_key_cb_push(sensor, *key);
    if(r) return r;
  }

  //Pop the expected virtual event
  for(i = 0; i < sensor->size; i++){
    r = ve_key_cb_pop(sensor, key);
    if(r) return r;

    if(*key == max_key){
      break;
    }

    r = ve_key_cb_push(sensor, *key);
    if(r) return r;    
  }

  //Set key as max
  *key = max_key;
  return 0;
}

static void handle_re_se(RawEvent re, VirtualEvent *waiting, ve_key_cb *sensorToVE, eg_cb *dataBuf, int *liveMap /*TODO:REMOVE*/){
  EventGroup eg;
  int sensor, key, r, i, max_ts;

  tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  assert(tm_tid > 0);

  sensor = re.event.se_event.id;
  assert(sensor >= 0);

  if(sensorToVE[sensor].size > 0){
    //Possible conflict, but we'll assume they all ran over it
    // while(ve_key_cb_pop(&sensorToVE[sensor], &key) != CB_E_EMPTY){
    //   if(waiting[key].type == VE_NONE){
    //     assert(0 && "Didn't clear sensors");
    //   }
    //   eg.type = waiting[key].type == VE_REG ? VRE_RE : VRE_VE_RE; 
    //   eg.re = re;
    //   eg.ve = waiting[key];
    //   eg.ve.event.train_at.train_num = liveMap[eg.ve.event.train_at.train_num];
    //   r = eg_cb_push(dataBuf, eg);
    //   assert(r != CB_E_FULL);
    //   assert(key >= 0 && key < MAX_LIVE_TRAINS * MAX_OUTSTANDING_EVENT);
    //   reset_waiting_room(&waiting[key]);
    //   TMLogStrf(tm_tid, "HIT %d on %s\n", key, TRACK[re.event.se_event.id].name);
    // }
    //No more conflict, assume earliest timestamp
    // r = pop_earliest_ts(&sensorToVE[sensor], waiting, &key);
    r = ve_key_cb_pop(&sensorToVE[sensor], &key);
    assert(r == 0);
    eg.type = waiting[key].type == VE_REG ? VRE_RE : VRE_VE_RE; 
    eg.re = re;
    eg.ve = waiting[key];
    eg.ve.event.train_at.train_num = liveMap[eg.ve.event.train_at.train_num];
    r = eg_cb_push(dataBuf, eg);
    assert(r != CB_E_FULL);
    reset_waiting_room(&waiting[key]);
    TMLogStrf(tm_tid, "HIT %d on %s\n", key, TRACK[re.event.se_event.id].name);
  }else{
    //Just an RE
    eg.type = RE;
    eg.re = re;
    r = eg_cb_push(dataBuf, eg);
    assert(r != CB_E_FULL);
    // TMLogStrf(tm_tid, "RE on %d\n", re.event.se_event.id);
  }
}

void HandleWR_RE(WRRequest *event, VirtualEvent *waiting, ve_key_cb *sensorToVE, eg_cb *dataBuf, int *liveMap /*TODO: REMOVE*/){
  int r;
  EventGroup eg;

  tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  assert(tm_tid > 0);

  switch(event->data.re.type){
    case RE_SE:
      handle_re_se(event->data.re, waiting, sensorToVE, dataBuf, liveMap /*TODO: REMOVE*/);
      break;
    case RE_SW:
    case RE_TR_CMD:
    case RE_TR_INIT:
      eg.type = RE;
      eg.re = event->data.re;
      r = eg_cb_push(dataBuf, eg);
      assert(r != CB_E_FULL);
      // TMLogStrf(tm_tid, "RE NOT SENSOR\n");
      break;
    default:
      assert(0 && "Bad Raw Event");
  }
}

static void handle_to_tr_at(VirtualEvent ve, VirtualEvent *waiting, ve_key_cb *sensorToVE, eg_cb *dataBuf, int *liveMap /*TODO: REMOVE THIS*/){
  EventGroup eg;
  int sensor, key, size, i, r;

  tid_t tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  assert(tm_tid > 0);

  sensor = ve.event.train_at.node->num;
  assert(sensor >= 0);

  //Handle if waiting on sensors
  if(sensorToVE[sensor].size > 0){
    size = sensorToVE[sensor].size;
    //Remove Virtual Event Due to Timeout
    for(i = 0; i < size; i++){
      r = ve_key_cb_pop(&sensorToVE[sensor], &key);
      assert(r != CB_E_EMPTY);
      if(key == ve.event.train_at.train_num * MAX_OUTSTANDING_EVENT + ve.key){
        eg.type = VRE_VE;
        eg.ve = ve;
        eg.ve.event.train_at.train_num = liveMap[eg.ve.event.train_at.train_num];        
        r = eg_cb_push(dataBuf, eg);
        assert(r != CB_E_FULL);
        assert(key >= 0 && key < MAX_LIVE_TRAINS * MAX_OUTSTANDING_EVENT);
        reset_waiting_room(&waiting[key]);
        TMLogStrf(tm_tid, "TO %d on %s\n", key, ve.event.train_at.node->name);
        break;
      }
      else{
        r = ve_key_cb_push(&sensorToVE[sensor], key);
        assert(r != CB_E_FULL);
      }
    }
  }

  //Handle if was old
  key = ve.event.train_at.train_num * MAX_OUTSTANDING_EVENT + ve.key;
  if(waiting[key].type != VE_NONE){
    eg.type = VRE_VE;
    eg.ve = ve;
    eg.ve.event.train_at.train_num = liveMap[eg.ve.event.train_at.train_num];
    r = eg_cb_push(dataBuf, eg);
    assert(r != CB_E_FULL);
    reset_waiting_room(&waiting[key]);
    TMLogStrf(tm_tid, "VRE VE on %s\n", ve.event.train_at.node->name);
  }
}

void HandleWR_TO(WRRequest *event, VirtualEvent *waiting, ve_key_cb *sensorToVE, eg_cb *dataBuf, int*liveMap /*TODO: REMOVE THIS*/){

  switch(event->data.ve.type){
    case VE_TR_AT:
      handle_to_tr_at(event->data.ve, waiting, sensorToVE, dataBuf,liveMap /*TODO: REMOVE THIS*/);
      break;
    default:
      assert(0 && "Timeout from bad data");
  }
}

void init_waiting_room(VirtualEvent* waiting, ve_key_cb *map){
  int i;
  for(i = 0; i < MAX_LIVE_TRAINS * MAX_OUTSTANDING_EVENT; i++){
    waiting[i].type = VE_NONE;
  }
  for(i = 0; i < SENSOR_SIZE; i++){
    ve_key_cb_init(&map[i]);
  }
}

//TODO: TEMP
void init_train_map(int *map){
  int i;
  for(i = 0; i < TRAIN_MAX; i++){
    map[i] = -1;
  }
}

void WaitingRoom(){
  int r;
  WRRequest event;
  EventGroup data;
  tid_t req_tid;
  tid_t courier = -1;

  VirtualEvent waiting[KEY_MAX*MAX_LIVE_TRAINS];
  ve_key_cb sensorToVE[SENSOR_SIZE];
  int live = 0; //TODO: TEMPORARY
  int trainMap[TRAIN_MAX]; //TODO: TEMPORARY - REMOVE THIS
  int liveMap[TRAIN_MAX]; //TODO: TEMPORARY - REMOVE THIS

  eg_cb dataBuf;
  eg_cb_init(&dataBuf);
  init_waiting_room(waiting, sensorToVE);
  init_train_map(trainMap); //TODO: TEMPORARY
  init_train_map(liveMap); //TODO: TEMPORARY

  r = RegisterAs(WAITING_ROOM_ID);
  assert(r == 0);

  Create(27, &TrainProvider);
  Create(27, &SwitchProvider);
  Create(27, &SensorProvider);
  Create(27, &VirtualProvider);

  Create(29, &TrainSubscriber);
  Create(29, &SwitchSubscriber);
  Create(29, &SensorSubscriber);
  Create(29, &VirtualEventSubscriber);

  while(true){

    if(dataBuf.size > 0 && courier > 0){
      r = eg_cb_pop(&dataBuf, &data);
      assert(r != CB_E_EMPTY);
      Reply(courier, &data, sizeof(data));
      courier = -1;
    }

    Receive(&req_tid, &event, sizeof(event));    

    switch(event.type){
      case WR_VE:
        Reply(req_tid, &r, sizeof(r));
        //TODO: TEMPORARY
        assert(event.data.ve.event.train_at.train_num >= 1 && event.data.ve.event.train_at.train_num <= TRAIN_MAX);
        assert(event.data.ve.key >= 0 && event.data.ve.key < KEY_MAX);
        if(trainMap[event.data.ve.event.train_at.train_num] == -1){
          trainMap[event.data.ve.event.train_at.train_num] = live;
          liveMap[live] = event.data.ve.event.train_at.train_num;
          event.data.ve.event.train_at.train_num = live;
          live++;
        }
        else{
          event.data.ve.event.train_at.train_num = trainMap[event.data.ve.event.train_at.train_num];
        }     
        HandleWR_VE(&event, waiting, sensorToVE, &dataBuf, liveMap);
        break;
      case WR_RE:
        Reply(req_tid, &r, sizeof(r));
        HandleWR_RE(&event, waiting, sensorToVE, &dataBuf, liveMap);
        break;
      case WR_TO:
        Reply(req_tid, &r, sizeof(r)); 
        HandleWR_TO(&event, waiting, sensorToVE, &dataBuf, liveMap);
	break;
      case WR_CE:
        courier = req_tid;
        break;
      default:
        assert(0 && "bad request");
    }
  }

  Exit();
}

//-----------Tests----------//
void test_waiting_room(){
  int r;
  tid_t cs_tid, my_tid, wr_tid, tm_tid, vp_tid;
  VERequest ver;
  VirtualEvent ve;

  my_tid = MyTid();
  wr_tid = MyParentTid();
  cs_tid = WhoIs(CLOCKSERVER_ID);
  tm_tid = WhoIs(TERMINAL_MANAGER_ID);
  vp_tid = WhoIs(VIRTUAL_PROVIDER_ID);

  assert(my_tid > 0 && wr_tid > 0 && cs_tid > 0 && tm_tid > 0 && vp_tid > 0);

  ve.type = VE_TR_AT;
  ve.timestamp = Time(cs_tid, my_tid) + 500;
  ve.timeout = 500;
  ve.depend = 38;
  ve.event.train_at.train_num = 24;
  ve.event.train_at.node = &TRACK[4];

  ver.type = VER_REGISTER;
  ver.ve = ve;

  while(true){
    ver.ve.timestamp = Time(cs_tid, my_tid) + 500;
    TMLogStrf(tm_tid, "Sending train %d at %s\n", ver.ve.event.train_at.train_num, ver.ve.event.train_at.node->name);
    Send(vp_tid, &ver, sizeof(ver), &r, sizeof(r));
    Delay(cs_tid, my_tid, 1200);  //Every 120 seconds
  }

  Exit();
}
