/*
 * main.c
 */

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>

#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/commons.h>


#define ENABLE_MAIN_TRACES 1
#if ENABLE_MAIN_TRACES
	#define MAIN_TRACE_INFO			TRACE_INFO
	#define MAIN_TRACE_DEBUG		TRACE_DEBUG
	#define MAIN_TRACE_WARNING		TRACE_WARNING
	#define MAIN_TRACE_ERROR		TRACE_ERROR
	#define MAIN_TRACE_FATAL		TRACE_FATAL
#else
	#define MAIN_TRACE_INFO(...)	{ }
	#define MAIN_TRACE_DEBUG(...)	{ }
	#define MAIN_TRACE_WARNING(...)	{ }
	#define MAIN_TRACE_ERROR		TRACE_ERROR
	#define MAIN_TRACE_FATAL		TRACE_FATAL
#endif


void taskMain() {
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	while(1) {
		LED_wave(1);
		LED_waveReverse(1);
		LED_wave(1);
		LED_waveReverse(1);

		vTaskDelay(500 / portTICK_RATE_MS);
	}
}

int main() {
	unsigned int i = 0;
	xTaskHandle taskMainHandle;

	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 2000000, BOARD_MCK);
	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	printf("\n\r -- ISIS-OBC First Project Program Booted --\n\r");
#ifdef __OPTIMIZE__
	printf("\n\r -- Compiled on  %s %s in release mode --\n\r", __DATE__, __TIME__);
#else
	printf("\n\r -- Compiled on  %s %s in debug mode --\n\r", __DATE__, __TIME__);
#endif

	LED_start();

	// The actual watchdog is already started, this only initializes the watchdog-kick interface.
	WDT_start();

	LED_wave(1);
	LED_waveReverse(1);
	LED_wave(1);
	LED_waveReverse(1);

	printf("\t main: Starting main task.. \n\r");
	xTaskGenericCreate(taskMain, (const signed char*)"taskMain", 1024, NULL, configMAX_PRIORITIES-2, &taskMainHandle, NULL, NULL);

	printf("\t main: Starting scheduler.. \n\r");
	vTaskStartScheduler();

	// This part should never be reached.
	printf("\t main: Waiting in an infinite loop. \n\r");
	while(1) {
		LED_wave(1);
		printf("MAIN: STILL ALIVE %d\n\r", i);
		i++;
	}

	return 0;
}
