#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <at91/utility/exithandler.h>
#include <string.h>
#include "GlobalStandards.h"
#include "InitSystem.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Housekepping/DUMP.h"
#include <satellite-subsystems/IsisAntS.h>
#include "TLM_management.h"

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#define I2c_SPEED_Hz 100000
//TODO change it after testing to 30.
#define ANT_AWAITED_TIME_SECONDS 1
#define I2c_Timeout 10
#define I2c_TimeoutTest portMAX_DELAY
#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);

//��� �� ���������� �������
Boolean isFirstActivation()
{
	unsigned char FirstActivation=0;
	int res=0;

	//TODO: remove print after testing
	printf("Inside isFirstActivation()");

	res = FRAM_read(&FirstActivation,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
	if (res==-2)
	{
		printf(" specified address and size are out of range of the FRAM space");
	}
	if  (res==-1)
	{
		printf(" obtaining lock for FRAM access fails");
	}

	if (FirstActivation==1)
	{
		//TODO: remove print after testing
		printf("Inside isFirstActivation() return TRUE");
		return TRUE;
	}
	else
	{
		//TODO: remove print after testing
		printf("Inside isFirstActivation() return FAlSE");
		return FALSE;
	}
}

//����� ����� 30 �� �������. ����� ����� ��� ����� ���� �� ������
void firstActivationProcedure()
{
	int err = 0;
	//TODO: remove print after testing
			printf("Inside firstActivationProcedure()");
	const int TotalWaitTime = 1000 * 60 * ANT_AWAITED_TIME_SECONDS; //TODO: change 30 to be a define. TODO: check total awaited time value after this line. There is a bug!
	int AwaitedTime = 0;
	err = FRAM_read ((unsigned char *)&AwaitedTime ,MOST_UPDATED_SAT_TIME_ADDR , MOST_UPDATED_SAT_TIME_SIZE	 );
	if (!err)
	{
		while (TotalWaitTime>AwaitedTime)
		{
			//TODO: remove print after testing
						printf("waited ");
			vTaskDelay(1000*10);

			AwaitedTime += 1000*10;
			FRAM_write((unsigned char*)&AwaitedTime , MOST_UPDATED_SAT_TIME_ADDR , MOST_UPDATED_SAT_TIME_SIZE);
			TelemetryCollectorLogic();

		}
	}
}

	//����� ���� ����� ���� �������.
void WriteDefaultValuesToFRAM()
{
	//TODO: remove print after testing
				printf("Inside WriteDefaultValuesToFRAM()");
	int DefNoCom=DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*)&DefNoCom, NO_COMM_WDT_KICK_TIME_ADDR,sizeof(DefNoCom));
	int NumberVoltages=NUMBER_OF_THRESHOLD_VOLTAGES;
	FRAM_write((unsigned char*)&NumberVoltages,EPS_THRESH_VOLTAGES_ADDR,EPS_THRESH_VOLTAGES_SIZE);
	int alpha=DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*)&alpha,EPS_ALPHA_FILTER_VALUE_ADDR,EPS_ALPHA_FILTER_VALUE_SIZE);
	int eps= DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&eps,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(eps));
	int trxvu=DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&trxvu,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(trxvu));
	int ant=DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&ant, ANT_SAVE_TLM_PERIOD_ADDR,sizeof(ant));
	int solar=DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&solar,SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(solar));
	int wod=DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&wod,WOD_SAVE_TLM_PERIOD_ADDR,sizeof(wod));
	//TODO:FRAM_write(add beacon);
}

	//����� � FRAM
int StartFRAM()
{
	int result=0;
	//TODO: remove print after testing
					printf("Inside StartFRAM()");
	result=FRAM_start();
	if(-1==result)
	{
		printf("failed to creaT semaforeS");
	}
	else if(-2==result)
	{
		printf("failed to initializing SPI");
	}
	else
	{
		//TODO: remove print after testing
		printf("FRAM_start was successful");
	}

	return result;
}

	// i2c ����� �
int StartI2C()
{
	int result=0;

	//TODO: remove after finish testing
	printf("Inside StartI2C() - calling I2C_start driver");

	result=I2C_start(I2c_Timeout,I2c_Timeout);
	if(result==-3)
	{
		printf("the driver uses a timeout of 1");
	}
	else if(result==-2)
	{
		printf("TWI peripheral fails");
	}
	else if(result==-1)
	{
		printf("creating the task that consumes I2C transfer requests failed");
	}

	if(result==0)
	{
		printf(" success");
	}

	return result;
}

	//spI ����� �
int StartSPI()
{
	int result= 0;
	//TODO: remove print after testing
						printf("Inside StartSPI()");
	result = SPI_start(bus1_spi, slave1_spi);

	if(result==0)
	{
		printf("success");
	}
	else if(result==-1)
	{
		printf("error");
	}
	return result;
}

	//����� ����� �� ������
int StartTIME()
{
	int err = 0;
	//TODO: remove print after testing
							printf("Inside StartTIME()");
	Time expected_deploy_time = UNIX_DEPLOY_DATE_JAN_D1_Y2020;
	err = Time_start(&expected_deploy_time, 0);
	if (0 != err)
	{
		return err;
	}

	time_unix time_before_wakeup = 0;
	if (!isFirstActivation())
	{
		printf("reset clock ");
		FRAM_read((unsigned char*) &time_before_wakeup,
				MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
		Time_setUnixEpoch(time_before_wakeup);
	}
	return 0;
}

	// ����� ������ ���� 30 �� ���
int DeploySystem()
{
	//TODO: remove print after testing
	printf(" DeploySystem() here");

	if(isFirstActivation())
	{
		firstActivationProcedure();

		unsigned int deployTime;
		Time_getUnixEpoch(&deployTime);
		FRAM_write((unsigned char *)&deployTime, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);

		ISISantsI2Caddress addressab;
		addressab.addressSideA=ANTS_I2C_SIDE_A_ADDR;
		addressab.addressSideB=ANTS_I2C_SIDE_B_ADDR;
		int res=IsisAntS_initialize( &addressab, 1);//����� ��� ����� 1

		if(res==0)
		{
			res=IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA, 10);//TODO: define interval
		}

		if(res==0)
			res = IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB, 10);//TODO: define interval

		if(res==0)
		{
			Boolean firstactivation= FALSE;
			FRAM_write((unsigned char *)&firstactivation, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
		}
	}

	return 0;
}

	//����� ��� �������
int InitSubsystems()
{
	//TODO: remove print after testing
		printf("InitSubsystems()  here");

	//TODO: not sure we should stop if something fails
	int err;
	err = StartSPI();
	if (err!=0)
		return err;

	err = StartI2C();
	if (err!=0)
		return err;

	err = StartFRAM();
	if (err!=0)
		return err;

	//TODO: not always only before flight
	WriteDefaultValuesToFRAM();

	err = StartTIME();
	if (err!=0)
		return err;

	err=InitTrxvu();
	if (err!=0)
		return err;

	err=InitializeFS(TRUE);
	if (err!=0)
		return err;

	err=InitDump();
	if (err!=0)
		return err;

	err=EPS_Init();
	if (err!=0)

		return err;

	err = DeploySystem();
	if (err!=0)
		return err;

	return 0;
}
