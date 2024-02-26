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
  }
  static int gpioKeyInit(void)
  {
      int i;
      int ret=0;
      char gpioName[10];
      for(i=0;i<GPIOKEYCNT;i++)
      {
          sprintf(gpioName,"key%d",gpioKey[i]);
          ret = gpio_request(gpioKey[i], gpioName);
          if(ret < 0) {
              printk("Failed Request gpio%d error\n", 6);
              return ret;
          }
      }
      for(i=0;i<GPIOKEYCNT;i++)
      {
          ret = gpio_direction_input(gpioKey[i]);
          if(ret < 0) {
              printk("Failed direction_output gpio%d error\n", 6);
              return ret;
          }
      }
      return ret;
  }
  static int  gpioKeyGet(void)
  {
      int i;
      int ret;
      int keyData=0;
      for(i=0;i<GPIOKEYCNT;i++)
      {
  //      ret=gpio_get_value(gpioKey[i]) << i;
  //      keyData |= ret;
         ret=gpio_get_value(gpioKey[i]);
         keyData = keyData | ( ret << i );
     }
     return keyData;
 }
 static void gpioKeyFree(void)
 {
     int i;
     for(i=0;i<GPIOKEYCNT;i++)
     {
         gpio_free(gpioKey[i]);
     }
 }

 int ledkey_open (struct inode *inode, struct file *filp)
 {
     int num0 = MAJOR(inode->i_rdev);
     int num1 = MINOR(inode->i_rdev);
     printk( "ledkey open -> major : %d\n", num0 );
     printk( "ledkey open -> minor : %d\n", num1 );
     try_module_get(THIS_MODULE);
     return 0;
 }

 loff_t ledkey_llseek (struct file *filp, loff_t off, int whence )
 {
     printk( "ledkey llseek -> off : %08X, whenec : %08X\n", (unsigned int)off, whence );
     return 0x23;
 }

 ssize_t ledkey_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
 {
     char kbuf;
     printk( "ledkey read -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
     kbuf = (char)gpioKeyGet();
     put_user(kbuf, buf);
     //result = copy_to_user(buf, &kbuf, count);
     return count;
 }

 ssize_t ledkey_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
 {
 //  int i;
 //  int result;
     char kbuff;
 /*  char kbuff[10];
     for(i=0;i<count;i++)
         get_user(kbuff[i],buf++);
 */
 /*  char kbuff[10];
     copy_from_user(kbuff,buf,count);
 */
     printk( "ledkey write -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
     get_user(kbuff,buf);
 //  result = copy_from_user(&kbuff,buf,count);
     gpioLedSet(kbuff);
     return count;  // buf 사이즈라서 1이 리턴
 }

 //int ledkey_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
 static long ledkey_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
 {

     printk( "ledkey ioctl -> cmd : %08X, arg : %08X \n", cmd, (unsigned int)arg );
     return 0x53;
 }

 int ledkey_release (struct inode *inode, struct file *filp)
 {
     printk( "ledkey release \n" );
     module_put(THIS_MODULE);
     return 0;
 }

 struct file_operations ledkey_fops =
 {
 //    .owner    = THIS_MODULE,
     .open     = ledkey_open,
     .read     = ledkey_read,
     .write    = ledkey_write,
     .unlocked_ioctl = ledkey_ioctl,
     .llseek   = ledkey_llseek,
     .release  = ledkey_release,
 };

 int ledkey_init(void)
 {
     int result;

     printk( "ledkey ledkey_init \n" );

     result=gpioLedInit();
     if(result < 0)
         return result;
     result=gpioKeyInit();
     if(result < 0)
         return result;

     result = register_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME, &ledkey_fops);
     if (result < 0) return result;

     return 0;
 }

 void ledkey_exit(void)
 {
     printk( "ledkey ledkey_exit \n" );
     unregister_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME );
     gpioLedFree();
     gpioKeyFree();
 }

 module_init(ledkey_init);
 module_exit(ledkey_exit);

 MODULE_LICENSE("Dual BSD/GPL");
