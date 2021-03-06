#include "MainTest.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>

#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/commons.h>

#include <hcc/api_fat.h>

#include "InitSystem.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/HouseKepping/TelemetryCollector.h"
#include "TrxvuTestingDemo.h"
#include "SubSystemModules/Communication/Trxvu.h"

Boolean selectAndExecuteTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

#define RESTART_INDEX		0
#define TLM_TEST_INDEX		1
#define EPS_TEST_INDEX		2
#define TRXVU_TEST_INDEX	3
#define CMD_TEST_INDEX		4
#define MNGMNT_TEST_INDEX	5
#define FS_TEST_INDEX 		6


	unsigned int number_of_tests = 7;

	printf("\n\r Select the device to be tested to perform: \n\r");
	printf("\t 0) Restart\n\r");
	printf("\t 1) Telemetry Testing\n\r");
	printf("\t 2) EPS Testing\n\r");
	printf("\t 3) TRXVU Testing\n\r");
	printf("\t 4) Commands Handeling Testing\n\r");
	printf("\t 5) Managment Testing\n\r");
	printf("\t 6) File System Testing\n\r");


	while (UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch (selection) {
	case RESTART_INDEX:
		restart();
		vTaskDelay(10000);
		printf("what?? \n\n\nwhere am I???");
		break;

	case TLM_TEST_INDEX:
		offerMoreTests = MainTelemetryTestBench();
		break;

	case EPS_TEST_INDEX:
		offerMoreTests = MainEpsTestBench();
		break;

	case TRXVU_TEST_INDEX:
		offerMoreTests = MainTrxvuTestBench();
		break;

	case CMD_TEST_INDEX:
		offerMoreTests = MainCommandsTestBench();
		break;

	case MNGMNT_TEST_INDEX:
		offerMoreTests = MainMaintenanceTestBench();
		break;

	case FS_TEST_INDEX:
		offerMoreTests = MainFileSystemTestBench();
		break;

	default:
		printf("Undefined Test\n\r");
		offerMoreTests = TRUE;
		break;
	}

	return offerMoreTests;
}


void taskTesting()
{
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);
	TestFirstActivationProcedure();
	/*InitSubsystems();
	//voltage_t curr_voltage = 0;
	int i = 0;
	while (1)
	{
		//GetBatteryVoltage(&curr_voltage);
		printf("GivatShmuel:main testing loop: i= : %d\n",  i);
		//BeaconLogic();
		TRX_Logic();
		printf("GivatShmuel:main testing loop after TRX_Logic: i= : %d\n",  i);
		TelemetryCollectorLogic();
		printf("GivatShmuel:main testing loop after Telemetry Logic: i= : %d\n",  i);
		vTaskDelay(100);
		printf("GivatShmuel:main testing loop after delay: i= : %d\n",  i);
		i++;
	}
	TestDumpTelemetry(); */
}

void TestFirstActivationProcedure()
{
	int err = StartSPI();
	err = StartI2C();
	err = StartFRAM();
	firstActivationProcedure();
}
