#include <user/test/ioserver_test.h>

int stay_alive;
void IdleTask() {
  int i = 0;
  while(stay_alive) {
    i++;
    if (i > 1500000) {
      PRINTF("ping\r\n");
      i = 0;
    }
  }
  Exit();
}

void IOTask() {
  char c;
  tid_t rios_tid, tios_tid;
  rios_tid = WhoIs(IOSERVER_UART2_RX_ID);
  assert(rios_tid > 0);
  tios_tid = WhoIs(IOSERVER_UART2_TX_ID);
  assert(tios_tid > 0);

  while (true) {
    c = GetC(rios_tid);
    if (c == 'q') {
      assert(0 && "EXIT");
      stay_alive = 0;
      Exit();
    }
    else {
      PutC(tios_tid, c);
      PRINTF("%c\r\n", c);
    }
  }
}


void IOServerTest() {
  stay_alive = 1;
  Create(31, &NameServer);
  Create(31, &ClockServer);
  Create(30, &IOServerUART2); // NOTE: priority has to be < priority of IOServer
  Create(26, &IOTask);
  Create(25, &IdleTask);
  Create(23, &NameServerStop);
  Create(23, &ClockServerStop);
  COMPLETE_TEST();
}