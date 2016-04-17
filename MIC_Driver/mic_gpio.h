#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h> /*copy_from_user */
#include <asm/system.h>
#include <mach/pinmux_reg.h>
#include <mach/ac83xx_pinmux_table.h>
#include <mach/ac83xx_gpio_pinmux.h>


#include <mach/gpio.h> 

#include "mic_command.h"
#include "mic_ctrl_api.h"
//#include "mic_command.h"

#include <linux/i2c-dev.h>
extern struct i2c_client  *mic_i2c_client;

extern int mic_i2cdev_ioctl_rdrw(struct i2c_client *client,
		unsigned long arg);

