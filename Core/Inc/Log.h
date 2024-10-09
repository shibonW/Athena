/*
 * Log.h
 *
 *  Created on: Oct 9, 2024
 *      Author: 15845
 */


#ifndef INC_LOG_H_
#define INC_LOG_H_
#include <stdint.h>  // 为了使用标准整数类型
#include <string.h>
#define FLASH_SECTOR_SIZE 256
#define BUFFER_SIZE 16  // 定义缓冲区的大小
#define FLASH_START_ADDR ((uint32_t)0x00000000)

// 定义包含XYZ坐标和时间戳的结构体
typedef struct {
    float x;
    float y;
    float z;
    uint8_t timestamp;
} SensorData;

// 缓冲区
extern SensorData buffer[];


void AddDataToBuffer(SensorData data); // 添加新的传感器数据到缓冲区
void WriteDataToFlash(void);
void AddSensorData(SensorData data);


#endif /* INC_LOG_H_ */
