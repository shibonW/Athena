/*
 * FileSystem.c
 *
 *  Created on: Oct 23, 2024
 *      Author: 15845
 */


#include <ringfs.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>


enum sector_status {
    SECTOR_ERASED     = 0xFFFFFFFF, //擦除过后，扇区的默认状态
    SECTOR_FREE       = 0xFFFFFF00, //扇区已经擦除，且目前是空闲的
    SECTOR_IN_USE     = 0xFFFF0000, //扇区当前包含有效数据
    SECTOR_ERASING    = 0xFF000000, //扇区正处于将要被擦除的过程中
    SECTOR_FORMATTING = 0x00000000, //整个分区正在格式化
};

struct sector_header {      //扇区头部结构体，用于记录扇区的状态和版本号
    uint32_t status;    //扇区状态，可以从sector_status查询当前扇区的状态
};

//获取当前扇区的绝对地址
static int _sector_address(struct ringfs *fs, int sector_offset){
	/*fs->flash->sector_offset表示文件系统分区占用的扇区，sector_offset表示当前扫描到的扇区号
	    上面两个数相加能得到当前要扫描的扇区的绝对地址，乘每个扇区的大小就能得到当前扇区的物理地址
	    */
	return (fs->flash->sector_offset + sector_offset) * fs->flash->sector_size;
}

//获取当前扇区的状态
static int _sector_get_status(struct ringfs *fs, int sector, uint32_t *status)
{

    return fs->flash->read(fs->flash,_sector_address(fs, sector) + offsetof(struct sector_header, status),status, sizeof(*status));
}

//设置扇区状态
static int _sector_set_status(struct ringfs *fs, int sector, uint32_t status){

	return fs->flash->program(fs->flash,_sector_address(fs, sector) + offsetof(struct sector_header, status),&status, sizeof(status));
}

//擦除一个扇区并将其标记为空闲状态
static int _sector_free(struct ringfs *fs, int sector)
{
    int sector_addr = _sector_address(fs, sector);//获取当前扇区的绝对地址
    _sector_set_status(fs, sector, SECTOR_ERASING);     //设置当前扇区的状态为正在擦除
    fs->flash->sector_erase(fs->flash, sector_addr);        //擦除当前扇区
//    fs->flash->program(fs->flash,
//            sector_addr + offsetof(struct sector_header, version),
//            &fs->version, sizeof(fs->version));     //写入版本号
    _sector_set_status(fs, sector, SECTOR_FREE);        //设置当前扇区的状态为空闲
    return 0;
}

//设置槽的状态，防止意外掉电
enum slot_status {
    SLOT_ERASED   = 0xFFFFFFFF, //擦除后空闲，可以写入
    SLOT_RESERVED = 0xFFFFFF00, //正在写入，但未提交，暂时不可用
    SLOT_VALID    = 0xFFFF0000, //写入完成，包含有效数据
    SLOT_GARBAGE  = 0xFF000000, //数据已无效，槽可回收或重用
};

struct slot_header {        //槽头部结构体，用于记录槽的状态
    uint32_t status;
};

static int _slot_address(struct ringfs *fs, struct ringfs_loc *loc) {
	return _sector_address(fs, loc->sector) +sizeof(struct sector_header) + (sizeof(struct slot_header) + fs->object_size) * loc->slot;
}

//获取槽的状态
static int _slot_get_status(struct ringfs *fs, struct ringfs_loc *loc, uint32_t *status)
{
    /*
    将当前槽的状态读入status变量中
    */
    return fs->flash->read(fs->flash,_slot_address(fs, loc) + offsetof(struct slot_header, status),status, sizeof(*status));
}

static int _slot_set_status(struct ringfs *fs, struct ringfs_loc *loc, uint32_t status)
{
    return fs->flash->program(fs->flash, _slot_address(fs, loc) + offsetof(struct slot_header, status),&status, sizeof(status));
}

