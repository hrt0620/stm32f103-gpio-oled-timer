#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#include "main.h"

#define OLED_ADDRESS	0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

//定义IIC总线连接的GPIO端口, 用户只需要修改下面代码即可任意改变SCL和SDA的引脚
//修改引脚时修改  四个地方    
#define OLED_SCL_GPIO_PORT	GPIOB			/* GPIO端口 */
#define OLED_SCL_PIN		GPIO_PIN_6			/* 连接到SCL时钟线的GPIO */

#define OLED_SDA_GPIO_PORT	GPIOB			/* GPIO端口 */
#define OLED_SDA_PIN		GPIO_PIN_7			/* 连接到SDA数据线的GPIO */

// HAL库GPIO操作宏定义
#define OLED_SCL_HIGH()		HAL_GPIO_WritePin(OLED_SCL_GPIO_PORT, OLED_SCL_PIN, GPIO_PIN_SET)
#define OLED_SCL_LOW()		HAL_GPIO_WritePin(OLED_SCL_GPIO_PORT, OLED_SCL_PIN, GPIO_PIN_RESET)
#define OLED_SDA_HIGH()		HAL_GPIO_WritePin(OLED_SDA_GPIO_PORT, OLED_SDA_PIN, GPIO_PIN_SET)
#define OLED_SDA_LOW()		HAL_GPIO_WritePin(OLED_SDA_GPIO_PORT, OLED_SDA_PIN, GPIO_PIN_RESET)
#define OLED_SDA_READ()		HAL_GPIO_ReadPin(OLED_SDA_GPIO_PORT, OLED_SDA_PIN)

// 延时函数声明
void DelayMs(uint32_t ms);

// 函数声明
void IIC_GPIO_Config(void);
void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_WaitAck(void);
void Write_IIC_Byte(uint8_t _ucByte);
void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowStr_S(unsigned char x, unsigned char y, unsigned char ch[], char POS,unsigned char len,unsigned char TextSize);
void OLED_ShowCC_S(unsigned char x, unsigned char y, unsigned char *s , char POS,unsigned char len);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_ShowCC(unsigned char x, unsigned char y, unsigned char *s);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void OLED_Set_Pos(uint8_t x, uint8_t y);

#endif


