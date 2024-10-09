/*
 * Log.c
 *
 *  Created on: Oct 9, 2024
 *      Author: 15845
 */

#include "Log.h"
#include "w25q64_ll.h"
#include <stdint.h>
static int count = 0;
uint32_t flash_start_addr = FLASH_START_ADDR;
SensorData buffer[BUFFER_SIZE];
void AddDataToBuffer(SensorData data) {
    if (count < BUFFER_SIZE) {  //如果缓冲区没有存满就放入缓冲区
    	buffer[count] = data;
        count++;
    }

    // 检查缓冲区是否已满
    if (count == BUFFER_SIZE) {
        WriteDataToFlash();
        count = 0;  // 清空缓冲区
        memset(buffer, 0, sizeof(buffer));
        buffer[count] = data;
        count++;
    }
}

void WriteDataToFlash(void) {
	BSP_W25Qx_Erase_Block(flash_start_addr);//擦除扇区
	uint32_t  writeSize = BUFFER_SIZE * sizeof(SensorData);//设置写入大小
	BSP_W25Qx_Write((uint8_t*)buffer, flash_start_addr, writeSize);//写入buffer
	flash_start_addr = writeSize + flash_start_addr;  // 计算下一次写入时的开始地址
}

void AddSensorData(SensorData data) {
    AddDataToBuffer(data);
}



