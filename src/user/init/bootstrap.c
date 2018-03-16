#include <user/init/bootstrap.h>

// void SendGo(){
//   tid_t writer = WhoIs(IOSERVER_UART1_TX_ID);
//   char command[1];
//   command[0] = 96;
//   WriteCommandUART1(writer, command, 1);
//   Exit();
// }

void RailwayInit(){
    tid_t tx_tid = WhoIs(IOSERVER_UART1_TX_ID);
    assert(tx_tid >= 0);
    tid_t rx_tid = WhoIs(IOSERVER_UART1_RX_ID);
    assert(rx_tid >= 0);

    //Send Reset Commands to Terminal
    //BLPutC(tx_tid, 'X');
    BLPutC(tx_tid, 192);

    //Flush the IO
    FlushIO(tx_tid);
    FlushIO(rx_tid);

    //Send Go
    PutC(tx_tid, 96);

    Create(29, &TrainProvider);
    Create(29, &SwitchProvider);
    Create(29, &SensorProvider);
    
    Exit();
}

void Bootstrap() {
  int mytid;
  mytid = MyTid();
  Create(31, &NameServer);
  Create(31, &ClockServer);
  Create(30, &IOServerUART1);
  Create(30, &IOServerUART2);
  Create(30, &TerminalManager);

  Create(30, &TrackDataInit);

  //Create(30, &SendGo);
  Create(30, &RailwayInit);

  // interfaces
  Create(10, &TrainTrackInterface);
  Create(5, &TimerInterface);
  Create(5, &NProcsInterface);
  Create(5, &MemUsageInterface);
  Create(0, &Logger);
  //DelayCS(mytid, 200);


  // DelayCS(mytid, 200);
  // Create(20, &StoppingServerTest);
  // Create(20, &StopAtServer);
  // StopAtProtocol sap;
  // sap.train = 24;
  // sap.gear = 14;
  // sap.stop_from = 54; //D7
  // sap.stop_at = 17; //B3
  // CreateArgs(20, &StopAt, (void *)&sap);

  Exit();
}
