#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "stm32h7xx_ll_usart.h"
#include "low_power_drv.h"

//test
extern bool framInit_flag;
extern FM25ObjectType fm25;

static void usart2L4Task(void *argument);

osThreadId_t usart2L4TaskHandle;
extern SemaphoreHandle_t Uart2RxComplete;
extern uint8_t usart2_rxBuf[];

const osThreadAttr_t usart2L4Task_attributes = {
  .name = "usart2L4Task",
  .stack_size = 128 * 8,
  .priority = (osPriority_t) osPriorityNormal,
};


void user_usart2L4Task_init(){
	usart2L4TaskHandle = osThreadNew(usart2L4Task, NULL, &usart2L4Task_attributes);
	printf("usart2L4Task init ok!\n");
}

static const UserInit usart2L4_init = {
		.init = user_usart2L4Task_init,
};

//USER_INIT(usart2L4_init);


// H7中用于响应L4指令的后台任务
static void usart2L4Task(void *argument)
{
//	测试发送
	uint8_t data[16];
	for(int i =0; i < 15;++i){
		data[i] = i;// 16字节数据
	}
	// test
	WKUP_Init();

	printf("usart2L4Task Start\n");
	uint8_t read_data[256] = {0};
//	LL_mDelay(200);
//	Enter_Standby();
	for(;;)
	{
		if(xSemaphoreTake(Uart2RxComplete, pdMS_TO_TICKS(1000)) == pdPASS)
		{
			for(int i = 0; i < 7; i ++)
			{
				printf("%x ", usart2_rxBuf[i]);
			}
			printf("\n");
			uint16_t addr = (uint16_t)((usart2_rxBuf[0] << 8) | usart2_rxBuf[1]);
			uint16_t len =  (uint16_t)((usart2_rxBuf[2] << 8) | usart2_rxBuf[3]);
			printf("Recv msg from L4,StartAddr:%04x Length:%04x\n", addr,len);
			ReadBytesFromFM25xxx(&fm25,(uint32_t)addr,read_data,len);
			// 重置验证(可删除)
			for(int i = 0; i < 8; i ++)
			{
				printf("%d ", read_data[i]);
			}
			printf("\n");
			memset(read_data, 0, sizeof(read_data));
			// 执行计算 返回结果
			Uart2_SendStr(data,16);
			for(int i = 0; i < 10; i ++)
			{
				for(int j = 0; j < 10; j ++)
				{
					printf("\n");
				}
			}
			// test
			Enter_Standby();
		}
	}
}



//    for(;;)
//    {
//    	Uart2_SendStr(data,16);
//        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//        osDelay(2000);
//    }
