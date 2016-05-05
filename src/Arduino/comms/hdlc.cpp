
/*
 * RECEPTION RUTINE
 * Following HDLC Receive Flow Chart from AXSEM AX5042 Programming Manual (page 32)
 */
#include "AX5042.h"
 
char *hdlc_rx(){
	char *response;
	char data_size;
    int received;
	int i = 2;
    unsigned int control;
    unsigned int data;
    control = read_register(FIFOCTRL);
    data = read_register(FIFODATA);
    while(control & (bit(1) | bit(0))!=0x01){ // Search for delimiter
        control = read_register(FIFOCTRL);
        data = read_register(FIFODATA);
    }
    received=0;
	while(received == 0){
        control = read_register(FIFOCTRL);
        data = read_register(FIFODATA);
		if(control & bit(0) == bit(0)){
            return;
            /*DISCARDED PACKAGE: Abort detected*/
		}
        else if(control & bit(1) == bit(1)){   
            if(data & bit(3) == bit(3)){ 
                if(data & ( bit(2) | bit(1) | bit(0) ) == 0x06){
                    received = 1;
                    data_size = i-2;
                    *response = 0;
                    *(response+1) = data_size;
                    /*CRC OK: end of packet*/
                }else{
                    
                    /*DISCARDED PACKAGE: Number of packet bits not divisible by 8*/
                }
            }else{
                received= 1;
                *response=1;
                /*DISCARDED PACKAGE: wrong CRC
                 *We will notify the HWDmods of this reception */
            }  
        }else{
            *(response + i) = data;
            i++;
            /*DATA FRAME: save it to data buffer*/
        }
    }
	return response;
}


/*
 * TRANSMITION RUTINE
 * Following HDLC transmit Flow Chart from AXSEM AX5042 Programming Manual (page 31)
 */

void send_preamble(){
    int i = 0;
    unsigned int control = 0x03;
    unsigned int data = 0xAA;
    while(i < 10){
       write_register(FIFOCTRL,control);
       write_register(FIFODATA,data);
       i++;
    }
}

void send_packet(unsigned char * data, int data_size){
    int i = 0;

    /*HDLC FLAG, PACKET DELIMITER*/
    write_register(FIFOCTRL,0x03);
    write_register(FIFODATA,0x7E);

    for(i = 0; i < data_size; i++){
        write_register(FIFOCTRL,0x00);
        write_register(FIFODATA,data[i]);
    }

   /*CRC*/
    write_register(FIFOCTRL,0x01);
    write_register(FIFODATA,0x00);
    write_register(FIFOCTRL,0x01);
    write_register(FIFODATA,0x00);

  /*HDLC FLAG, PACKET DELIMITER*/
    write_register(FIFOCTRL,0x03);
    write_register(FIFODATA,0x7E);

}

void send_ABORT(){
    //HDLC ABORT
    write_register(FIFOCTRL,0x03);
    write_register(FIFODATA,0xFF);
    write_register(FIFOCTRL,0x03);
    write_register(FIFODATA,0xFF);

}

void hdlc_tx(char *data, int data_size){
    send_preamble();
  
    send_packet(data,data_size);

    send_ABORT();
}

