  1 #include <linux/init.h>   // 0x00으로 실행 시 버튼을 누르면 LED가 켜진다.
  2 #include <linux/module.h>
  3 #include <linux/kernel.h>
  4
  5 #include <linux/fs.h>
  6 #include <linux/errno.h>
  7 #include <linux/types.h>
  8 #include <linux/fcntl.h>
  9 #include <linux/gpio.h>
 10
 11 #define   LEDKEY_DEV_NAME            "ledkey_dev"
 12 #define   LEDKEY_DEV_MAJOR            230
 13
 14 #define OFF 0
 15 #define ON 1
 16 #define GPIOLEDCNT 8
 17 #define GPIOKEYCNT 8
 18 static int gpioLed[GPIOLEDCNT] = {6,7,8,9,10,11,12,13};
 19 static int gpioKey[GPIOKEYCNT] = {16,17,18,19,20,21,22,23};
 20
 21 static int gpioLedInit(void);
 22 static void gpioLedSet(long);
 23 static void gpioLedFree(void);
 24 static int gpioKeyInit(void);
 25 static int gpioKeyGet(void);
 26 static void gpioKeyFree(void);
 27
 28 static int gpioLedInit(void)
 29 {
 30     int i;
 31     int ret=0;
 32     char gpioName[10];
 33     for(i=0;i<GPIOLEDCNT;i++)
 34     {
 35         sprintf(gpioName,"led%d",i);
 36         ret = gpio_request(gpioLed[i],gpioName);
 37         if(ret < 0) {
 38             printk("Failed gpio_request() gpio%d error \n",i);
 39             return ret;
 40         }
 41
 42         ret = gpio_direction_output(gpioLed[i],OFF);
 43         if(ret < 0) {
 44             printk("Failed gpio_direction_output() gpio%d error \n",i);
 45             return ret;
 46         }
 47     }
 48     return ret;
 49 }
 50
 51 static void gpioLedSet(long val)
 52 {
 53     int i;
 54     for(i=0;i<GPIOLEDCNT;i++)
 55     {
 56         gpio_set_value(gpioLed[i],(val>>i) & 0x1);
 57     }
 58 }
 59 static void gpioLedFree(void)
 60 {
 61     int i;
 62     for(i=0;i<GPIOLEDCNT;i++)
 63     {
 64         gpio_free(gpioLed[i]);
 65     }
 66 }
 67 static int gpioKeyInit(void)
 68 {
 69     int i;
 70     int ret=0;
 71     char gpioName[10];
 72     for(i=0;i<GPIOKEYCNT;i++)
 73     {
 74         sprintf(gpioName,"key%d",gpioKey[i]);
 75         ret = gpio_request(gpioKey[i], gpioName);
 76         if(ret < 0) {
 77             printk("Failed Request gpio%d error\n", 6);
 78             return ret;
 79         }
 80     }
 81     for(i=0;i<GPIOKEYCNT;i++)
 82     {
 83         ret = gpio_direction_input(gpioKey[i]);
 84         if(ret < 0) {
 85             printk("Failed direction_output gpio%d error\n", 6);
 86             return ret;
 87         }
 88     }
 89     return ret;
 90 }
 91 static int  gpioKeyGet(void)
 92 {
 93     int i;
 94     int ret;
 95     int keyData=0;
 96     for(i=0;i<GPIOKEYCNT;i++)
 97     {
 98 //      ret=gpio_get_value(gpioKey[i]) << i;
 99 //      keyData |= ret;
100         ret=gpio_get_value(gpioKey[i]);
101         keyData = keyData | ( ret << i );
102     }
103     return keyData;
104 }
105 static void gpioKeyFree(void)
106 {
107     int i;
108     for(i=0;i<GPIOKEYCNT;i++)
109     {
110         gpio_free(gpioKey[i]);
111     }
112 }
113
114 int ledkey_open (struct inode *inode, struct file *filp)
115 {
116     int num0 = MAJOR(inode->i_rdev);
117     int num1 = MINOR(inode->i_rdev);
118     printk( "ledkey open -> major : %d\n", num0 );
119     printk( "ledkey open -> minor : %d\n", num1 );
120     try_module_get(THIS_MODULE);
121     return 0;
122 }
123
124 loff_t ledkey_llseek (struct file *filp, loff_t off, int whence )
125 {
126     printk( "ledkey llseek -> off : %08X, whenec : %08X\n", (unsigned int)off, whence );
127     return 0x23;
128 }
129
130 ssize_t ledkey_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
131 {
132     char kbuf;
133     printk( "ledkey read -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
134     kbuf = (char)gpioKeyGet();
135     put_user(kbuf, buf);
136     //result = copy_to_user(buf, &kbuf, count);
137     return count;
138 }
139
140 ssize_t ledkey_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
141 {
142 //  int i;
143 //  int result;
144     char kbuff;
145 /*  char kbuff[10];
146     for(i=0;i<count;i++)
147         get_user(kbuff[i],buf++);
148 */
149 /*  char kbuff[10];
150     copy_from_user(kbuff,buf,count);
151 */
152     printk( "ledkey write -> buf : %08X, count : %08X \n", (unsigned int)buf, count );
153     get_user(kbuff,buf);
154 //  result = copy_from_user(&kbuff,buf,count);
155     gpioLedSet(kbuff);
156     return count;  // buf 사이즈라서 1이 리턴
157 }
158
159 //int ledkey_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
160 static long ledkey_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
161 {
162
163     printk( "ledkey ioctl -> cmd : %08X, arg : %08X \n", cmd, (unsigned int)arg );
164     return 0x53;
165 }
166
167 int ledkey_release (struct inode *inode, struct file *filp)
168 {
169     printk( "ledkey release \n" );
170     module_put(THIS_MODULE);
171     return 0;
172 }
173
174 struct file_operations ledkey_fops =
175 {
176 //    .owner    = THIS_MODULE,
177     .open     = ledkey_open,
178     .read     = ledkey_read,
179     .write    = ledkey_write,
180     .unlocked_ioctl = ledkey_ioctl,
181     .llseek   = ledkey_llseek,
182     .release  = ledkey_release,
183 };
184
185 int ledkey_init(void)
186 {
187     int result;
188
189     printk( "ledkey ledkey_init \n" );
190
191     result=gpioLedInit();
192     if(result < 0)
193         return result;
194     result=gpioKeyInit();
195     if(result < 0)
196         return result;
197
198     result = register_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME, &ledkey_fops);
199     if (result < 0) return result;
200
201     return 0;
202 }
203
204 void ledkey_exit(void)
205 {
206     printk( "ledkey ledkey_exit \n" );
207     unregister_chrdev( LEDKEY_DEV_MAJOR, LEDKEY_DEV_NAME );
208     gpioLedFree();
209     gpioKeyFree();
210 }
211
212 module_init(ledkey_init);
213 module_exit(ledkey_exit);
214
215 MODULE_LICENSE("Dual BSD/GPL");
