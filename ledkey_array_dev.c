   #include <linux/init.h>   // 0x00으로 실행 시 버튼을 누르면 LED가 켜진다.
   #include <linux/module.h>
   #include <linux/kernel.h>

   #include <linux/fs.h>
   #include <linux/errno.h>
   #include <linux/types.h>
   #include <linux/fcntl.h>
   #include <linux/gpio.h>

  #define   LEDKEY_DEV_NAME            "ledkey_dev"
  #define   LEDKEY_DEV_MAJOR            230

  #define OFF 0
  #define ON 1
  #define GPIOLEDCNT 8
  #define GPIOKEYCNT 8
  static int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
  static int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};

  static int gpioLedInit(void);
  static void gpioLedSet(long);
  static void gpioLedFree(void);
  static int gpioKeyInit(void);
  static int gpioKeyGet(void);
  static void gpioKeyFree(void);

  static int gpioLedInit(void)
  {
      int i;
      int ret=0;
      char gpioName[10];
      for(i=0;i<GPIOLEDCNT;i++)
      {
          sprintf(gpioName,"led%d",i);
          ret = gpio_request(gpioLed[i],gpioName);
          if(ret < 0) {
              printk("Failed gpio_request() gpio%d error \n",i);
              return ret;
          }

          ret = gpio_direction_output(gpioLed[i],OFF);
          if(ret < 0) {
              printk("Failed gpio_direction_output() gpio%d error \n",i);
              return ret;
          }
      }
      return ret;
  }

  static void gpioLedSet(long val)
  {
      int i;
      for(i=0;i<GPIOLEDCNT;i++)
      {
          gpio_set_value(gpioLed[i],(val>>i) & 0x1);
      }
  }
  static void gpioLedFree(void)
  {
      int i;
      for(i=0;i<GPIOLEDCNT;i++)
      {
          gpio_free(gpioLed[i]);
      }
