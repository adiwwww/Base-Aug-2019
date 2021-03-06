#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Timing/Time.h>
#include <hal/errors.h>
#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>
#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/Housekepping/Dump.h"

//idle global variables
int				g_idle_period = 300 ;				// idle time default = 5 min
Boolean 		g_idle_flag = FALSE;
time_unix 		g_idle_start_time = 0;

//setting trxvu idle off
int SetIdleOff()
{
	//TODO: remove print after testing complete
	printf("inside SetIdleOff()\n");
	int err=IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_idle_state_off);
	if(err!=0)
	{
		//TODO: handle failure in setting idle sttate off
		printf("failed in setting trxvu idle off - %d\n", err);
	}
	return err;
}

// Checking if in idle state and if need to return back to regular state
void HandleIdleTime()
{
	//if in idle state
	if(g_idle_flag==TRUE)
	{
		//if idle period has passed
		if (CheckExecutionTime(g_idle_start_time, g_idle_period)==TRUE)
		{
			SetIdleOff();
			g_idle_flag=FALSE;
		}
		else
		{
			//TODO: remove print after testing complete
			printf("idle end period not reached\n");
		}
	}
}


// Initialize TRXVU component
int InitTrxvu()
{
	//TODO: remove print after testing complete
	printf("inside InitTrxvu()\n");

	//set I2C addresses
	ISIStrxvuI2CAddress i2cAdress;
	i2cAdress.addressVu_rc=I2C_TRXVU_RC_ADDR;
	i2cAdress.addressVu_tc=I2C_TRXVU_TC_ADDR;

	//set max frame lengths
	ISIStrxvuFrameLengths framelengths;
	framelengths.maxAX25frameLengthRX=SIZE_RXFRAME;
	framelengths.maxAX25frameLengthTX=SIZE_TXFRAME;

	//set bitrate
	ISIStrxvuBitrate default_bitrates;
	default_bitrates=trxvu_bitrate_9600;

	//Initialize TRXVU driver
	int err = IsisTrxvu_initialize(&i2cAdress,&framelengths,&default_bitrates,1);
	if(err!=0)
	{
		printf("Error in the initialization: %d\n", err);
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		printf("IsisTrxvu_initialize succeeded\n");
	}


	//TODO: check if required as bitrate already provided in init
	vTaskDelay(100);
	err=IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX ,trxvu_bitrate_9600);
	if(err!=0)
	{
		printf("Error in the IsisTrxvu_tcSetAx25Bitrate: %d\n", err);
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		printf("IsisTrxvu_tcSetAx25Bitrate succeeded\n");
	}
	vTaskDelay(100);

	//Set Antenas addresses for both sides
	ISISantsI2Caddress address;
	address.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	address.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//initialize Antenas system
	err=IsisAntS_initialize(&address,1);
	if(err!=0)
	{
		printf("Error in the initialization of the Antennas: %d\n", err);
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		printf("initialization of the Antennas succeeded\n");
	}

	//Initialize TRXVU transmit lock
	InitTxModule();

	//Initialize beacon parameters
	InitBeaconParams();

	return err;
}


//TRX VU main logic
 CommandHandlerErr TRX_Logic()
{
	#ifdef TESTING
	 	 printf("Inside TRX_Logic()\n");
	#endif
	sat_packet_t cmd={0};
	int onCmdCount;
	unsigned char* data = NULL;
	unsigned int length=0;
	CommandHandlerErr  res;

	//check if we have online command (frames in buffer)
	onCmdCount = GetNumberOfFramesInBuffer();

	if(onCmdCount>0) {
		//get the online command
		res = GetOnlineCommand(&cmd);
		if(res!=0)
			printf("Error in getting the online command: %d\n", res);
		else {
			#ifdef TESTING
				printf("Getting the online command success\n");
			#endif

			//Reset WD communication with earth by saving current time as last communication time in FRAM
			ResetGroundCommWDT();

			//Send Acknowledge to earth
			SendAckPacket(ACK_RECEIVE_COMM, &cmd, data, length);

			//run command
			res = ActUponCommand(&cmd);
		}
	}

	//check idle timer
	HandleIdleTime();

	//handle beacon
	BeaconLogic();

	return res;
}

// Command to set idle state to on
int CMD_SetIdleOn()
{
	//TODO: remove print after testing complete
	printf("inside CMD_SetIdleOn()\n");
	int err=IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_idle_state_on);
	if(err!=0)
	{
		printf("Failed in setting trxvy idle on - %d\n", err);
	}
	else
	{
		//get time of start idle period & set idle state flag to true
		Time_getUnixEpoch((unsigned int*)&g_idle_start_time);
		g_idle_flag=TRUE;
	}
	return err;
}



