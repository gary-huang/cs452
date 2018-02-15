#include<readerservice.h>

void ReaderServiceUART2Register(RSProtocol *rsp){
	int reply;
	int in_tid = WhoIs(READERSERVICE_UART2_ID);

	Send(in_tid, rsp, sizeof(RSProtocol), &reply, sizeof(int));
	assert(reply == 0);
}

void ReadChar(char *c){
	int reply;
	tid_t reader;
	RSResponse rsr;

	Receive(&reader, &rsr, sizeof(rsr));

	//COPY FIRST!!
	*c = *rsr.data;

	Reply(reader, &reply, sizeof(reply));
}

void ReadCommand(char *command, int *size){
	int reply = 0;
	tid_t reader;
	RSResponse rsr;

	Receive(&reader, &rsr, sizeof(rsr));

	//COPY FIRST!
	int i;
	for(i = 0; i < rsr.size; i++){
		command[i] = rsr.data[i];
	}
	*size = rsr.size;

	Reply(reader, &reply, sizeof(reply));
}

void NotifyChar(tid_t *list, int size, char data){
	int reply;
	int i;

	RSResponse rsr;
	rsr.data = &data;
	rsr.size = 1;
	for(i = 0; i < size; i++){
		Send(list[i], &rsr, sizeof(rsr), &reply, sizeof(reply));
	}
}

void NotifyCommand(tid_t *list, int size, char *data, int datasize){
	int reply;
	int i;

	RSResponse rsr;
	rsr.data = data;
	rsr.size = datasize;

	for(i = 0; i < size; i++){
		Send(list[i], &rsr, sizeof(rsr), &reply, sizeof(reply));
	}
}

void ReaderServiceUART2Notifier(){
	int reply;
	tid_t reader = MyParentTid();
	tid_t ios_tid = WhoIs(IOSERVER_UART2_RX_ID);
	RSProtocol rsp;

	rsp.rr_req = RR_WRITE;

	while(true){
		rsp.data = GetC(ios_tid);
		Send(reader, &rsp, sizeof(rsp), &reply, sizeof(reply));
		assert(reply == 0);
	}
}

void ReaderServiceUART2(){
	int reply = 0;

	int wCharSize = 0;
	int wCommandSize = 0;
	tid_t wait_char[MAX_TASK];
	tid_t wait_command[MAX_TASK];

	int r = RegisterAs(READERSERVICE_UART2_ID);
	assert(r == 0);

	Create(31, &ReaderServiceUART2Notifier);
	
	char command[COMMAND_SIZE];
	int csize = 0;

	while(true){
		tid_t req_tid;
		RSProtocol rsp;

		//Receive a request
		Receive(&req_tid, &rsp, sizeof(rsp));

		//Handle Request
		switch(rsp.rr_req){
			case RR_CHAR:
				//TODO: We may miss input if user tries to type before listener is registered
				wait_char[wCharSize] = req_tid;
				wCharSize++;
				Reply(req_tid, &reply, sizeof(reply));
				break;
			case RR_COMMAND:
				wait_command[wCommandSize] = req_tid;
				wCommandSize++;
				Reply(req_tid, &reply, sizeof(reply));
				break;
			case RR_WRITE:
				NotifyChar(wait_char, wCharSize, rsp.data);
				command[csize] = rsp.data;
				csize++;
				if(rsp.data == BACKSPACE){
					if(csize >= 2){
						csize -= 2;
					}else{
						csize = 0;
					}
				}
				else if(rsp.data == CARRIAGE_RETURN){
					//Unblock waiting on command (-ignore /n input)
					if(csize - 1 > 0){
						NotifyCommand(wait_command, wCommandSize, command, csize - 1);	
					}
					csize = 0;
				}
				Reply(req_tid, &reply, sizeof(reply));
				break;
			default:
				assert(0 && "Bad request");
		}
	}	
}