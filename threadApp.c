#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

/*
 * 程序清单：这是一个 hwtimer 设备使用例程
 * 例程导出了 hwtimer_sample 命令到控制终端
 * 命令调用格式：hwtimer_sample
 * 程序功能：硬件定时器超时回调函数周期性的打印当前tick值，2次tick值之差换算为时间等同于定时时间值。
*/


#define HWTIMER_DEV_NAME          "timer3"

static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
  rt_kprintf("this is hwtimer timeout callback function!\n");
  rt_kprintf("tick is: %d!\n", rt_tick_get());
}

static int hwtimer_sample(int argc, char *argv[])
{
  rt_err_t ret = RT_EOK;
  rt_hwtimerval_t timeout_s;      // 超时时间结构体
  rt_device_t hw_dev = RT_NULL;   // 定时器设备句柄
  rt_hwtimer_mode_t mode;         // 定时器模式

  hw_dev = rt_device_find(HWTIMER_DEV_NAME);
  if (hw_dev == RT_NULL)
  {
    rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
    return RT_ERROR;
  }

  // 以读写方式打开设备
  ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
  if (ret != RT_EOK)
  {
    rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
    return ret;
  }

  // 设置超时回调函数
  rt_device_set_rx_indicate(hw_dev, timeout_cb);

  // 周期性定时器
  mode = HWTIMER_MODE_PERIOD;
  ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
  if (ret != RT_EOK)
  {
    rt_kprintf("set mode failed! ret is %d\n", ret);
    return ret;
  }

  // 设置定时器超时值为5s并启动定时器
  timeout_s.sec = 5;
  timeout_s.usec = 0;

  if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
  {
    rt_kprintf("set timeout value failed\n");
    return RT_ERROR;
  }

  rt_thread_mdelay(3500);

  rt_device_read(hw_dev, 0, &timeout_s, sizeof(timeout_s));
  rt_kprintf("Read: Sec = %d, Usec = %d\n", timeout_s.sec, timeout_s.usec);

  return ret;
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    __HAL_RCC_TIM3_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM3_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }
}

MSH_CMD_EXPORT(hwtimer_sample, hwtimer sample);

