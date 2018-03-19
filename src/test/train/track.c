#include <test/train/track.h>

static void track_init_test(track_node *tr) {
  Track track;
  TrackInit(&track, tr);
}


static void next_sensors_test(track_node *tr) {
  Track track;
  TrackInit(&track, tr);

  poss_node_list pnl;
  poss_node_list_init(&pnl);

  track_node *start;
  start = &tr[trhr(tr, "A1")];
  assert(start->type == NODE_SENSOR);
  start = start->edge[DIR_AHEAD].dest;
  assert(start != NULL);

  TrackGetNextPossibleSensors(&track, start, &pnl);
  assert(pnl.size == 1);

  track_node *tn;
  poss_node_list_pop(&pnl, &tn);
  assert(tn->num == 44);

  start = tn->edge[DIR_AHEAD].dest;
  TrackGetNextPossibleSensors(&track, start, &pnl);
  assert(pnl.size == 1);
  poss_node_list_pop(&pnl, &tn);
  assert(tn->num == 70);

  start = tn->edge[DIR_AHEAD].dest;
  TrackGetNextPossibleSensors(&track, start, &pnl);
  assert(pnl.size == 1);
  poss_node_list_pop(&pnl, &tn);
  assert(tn->num == 54);

  start = tn->edge[DIR_AHEAD].dest;
  TrackGetNextPossibleSensors(&track, start, &pnl);
  assert(pnl.size == 2);
  poss_node_list_pop(&pnl, &tn);
  assert(tn->num == 56);
  poss_node_list_pop(&pnl, &tn);
  assert(tn->num == 73);

  start = tr[1].edge[DIR_AHEAD].dest;
  TrackGetNextPossibleSensors(&track, start, &pnl);
  poss_node_list_pop(&pnl, &tn);
  assert(pnl.size == 0);
}

static void basic_test(track_node *g) {
  Track track;
  Train train;
  EventGroup event;
  TrackInit(&track, g);

  // emulate finding train 24
  train.num = 24;
  TrackAddLostTrain(&track, &train);
  assert(track.train[24].num == 24);
  assert(track.train[24].status == TR_LOST);

  // simulate a sensor hit at A1
  event.type = RE;
  event.re.timestamp = 1000;
  event.re.type = RE_SE;
  event.re.event.se_event.id = 0; // A1
  event.re.event.se_event.state = 1;
  TrackInterpretEventGroup(&track, &event);
  assert(track.lost_trains.size == 0);
  assert(track.active_trains.size == 1);
  assert(track.vevents.size == 1);
  VirtualEvent ve;
  ve_list_pop(&track.vevents, &ve);
  assert(ve.type == VE_TR_AT);
  // printf("%d\n", ve.timestamp);
  assert(ve.timeout == INT_MAX);
  assert(ve.timestamp == INT_MAX);
  assert(track.train[24].num == 24);
  assert(track.train[24].sen_ts == 1000);
  assert(track.train[24].speed == -1);
  assert(ve.event.train_at.train_num == 24);
  assert(ve.event.train_at.node->num == 44);

  // simulate the next sensor hit at C13 with the VER
  event.type = VRE_RE;
  event.re.timestamp = 1100;
  event.re.type = RE_SE;
  event.re.event.se_event.id = 44; // A1
  event.re.event.se_event.state = 1;
  event.ve = ve;
  TrackInterpretEventGroup(&track, &event);
  assert(track.lost_trains.size == 0);
  assert(track.active_trains.size == 1);
  assert(track.train[24].sen_ts == 1100);
  assert(track.train[24].sen_ts == 1100);
  assert(track.train[24].speed == 2310);
  assert(track.vevents.size == 1);
  ve_list_pop(&track.vevents, &ve);
  assert(ve.type == VE_TR_AT);
  assert(ve.timeout == 378);
  assert(ve.timestamp == 1100+378);
  assert(ve.event.train_at.train_num == 24);
  assert(ve.event.train_at.node->num == 70);
}

void track_tests() {
  track_node graph[TRACK_MAX];
  init_tracka(graph);
  track_init_test(graph);
  next_sensors_test(graph);
  basic_test(graph);
}
