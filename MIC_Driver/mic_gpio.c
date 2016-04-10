
#include "mic_gpio.h"


#define MIC_MAJOR	0
#define DEVICE_NAME		"mic-ifly"
#define MIC_I2C_NAME	"mic_xf"
#define MIC_NAME_SIZE	20
#define MIC_I2C_ADDRESS_LOW	0x47

static int globalmem_major = MIC_MAJOR;
struct i2c_client *mic_i2c_client = NULL;
EXPORT_SYMBOL(mic_i2c_client);

struct class *globalmem_cdev_class;
struct cdev	cdev;

static unsigned char mem_tmp[5];


#define GPIO_MIC_RESET	PIN_44_GPIO44
extern int gpio_request(unsigned gpio, const char *label);
extern int gpio_direction_output(unsigned, int);

static int mic_hardware_reset(void)
{
//	GPIO_MultiFun_Set(GPIO_MIC_RESET, PINMUX_LEVEL_GPIO_END_FLAG);
	gpio_request(GPIO_MIC_RESET, "MIC_RST");
	gpio_direction_output(GPIO_MIC_RESET, 0);	/* MIC Low Enable */
	gpio_set_value(GPIO_MIC_RESET, 0);
	mdelay(200);
	gpio_set_value(GPIO_MIC_RESET, 1);
	printk(KERN_INFO "mic-gpio init ok ...\r\n");

	return 0;
}

static int mic_software_init(void)
{
	int version;
	Command_t cmd ={0};
	int ret_val = 0;

	memset(&cmd,0,sizeof(cmd));

	mdelay(500);
	ret_val = VCWaitReady(&cmd, 0);
	if(ret_val != 1)
	{
		printk("MIC Module Not Wait Ready!\n");
		return ret_val;
	}
	VCReset(&cmd);

	version = VCGetVersion(&cmd);
//	printk("version=%d\n", version);
	if(version == 0)
	{
		printk("Get MIC Hardware Version Error!\n");
		return 0;
	}
	else if(version == 2)
	{
//		VCEnableNR(&cmd, 0); // �رս���

//		VCEnableNR(&cmd, 1); // �򿪽���
	}
	else if(version == 3 || version == 4) 
	{
		printk("Get MIC Hardware Version is %d!\n", version);	

		/* ���õ�ǰ�Ĺ���ģʽ */
		ret_val = VCChangeWorkMode(&cmd, WORK_MODE_TOPLIGHT);
		if(ret_val != 0)
		{
			printk("Set MIC WORK_MODE_TOPLIGHT Error! %d\n", ret_val);
		}
		ret_val = VCGetWorkMode(&cmd);	// ��鵱ǰ�Ĺ���ģʽ�Ƿ�Ϊ����ģʽ
		if (ret_val != WORK_MODE_TOPLIGHT)
		{
			printk("Get MIC WORK_MODE_TOPLIGHT Error!\n");
		}

		/* Ĭ�����й��ܹر�ֱ��¼��*/
		ret_val = VCChangeFunc(&cmd, FUNC_MODE_PASSBY);	
		if (ret_val != 0)
		{
			printk("Set MIC FUNC_MODE_PASSBY Error! %d\n", ret_val);
		}
		/*  ��鵱ǰ�Ĺ���*/
		ret_val = VCGetFunc(&cmd);
		if (ret_val != FUNC_MODE_PASSBY)
		{
			printk("Get MIC FUNC_MODE_PASSBY Error!\n");
		}
		
	}

	return 1;
}

static int bytes_to_int(unsigned char buf[], int start)
{
	int n = 0;
	n = ((int) buf[start]) << 24 | ((int) buf[start + 1]) << 16
	        | ((int) buf[start + 2]) << 8 | ((int) buf[start + 3]);
	return n;
}

static void int_to_bytes(int n, unsigned char buf[], int start)
{
	buf[start] = n >> 24;
	buf[start + 1] = n >> 16;
	buf[start + 2] = n >> 8;
	buf[start + 3] = n;
}

static int mic_cdev_open(struct inode *inode,struct file *filp)
{
//	printk("mic_open=============\n");
	
	return 0;
}

#define MIC_CMD_CTL_FUNC_MASK				0x80
#define IOCTL_CMD_I2C_FUNC_MODE_PASSBY		(MIC_CMD_CTL_FUNC_MASK|FUNC_MODE_PASSBY)
#define IOCTL_CMD_I2C_FUNC_MODE_NOISECLEAN	(MIC_CMD_CTL_FUNC_MASK|FUNC_MODE_NOISECLEAN)
#define IOCTL_CMD_I2C_FUNC_MODE_PHONE		(MIC_CMD_CTL_FUNC_MASK|FUNC_MODE_PHONE)
#define IOCTL_CMD_I2C_FUNC_MODE_WAKEUP		(MIC_CMD_CTL_FUNC_MASK|FUNC_MODE_WAKEUP)

#define GET_MIC_VERSION				0x80
#define GET_MIC_WAITREADY			(GET_MIC_VERSION + 1)
#define GET_MIC_CURFUNCTION			(GET_MIC_WAITREADY + 1)
#define SET_MIC_FUNCTION			(GET_MIC_CURFUNCTION + 1)
#define SET_MIC_WOEKMODE			(SET_MIC_WOEKMODE + 1)
#define SET_MIC_DACVOLUME			(SET_MIC_FUNCTION + 1)
#define SET_MIC_RESET				(SET_MIC_DACVOLUME + 1)


static ssize_t mic_cdev_read(struct file *filp,char __user *buf,size_t count, loff_t *ppos)
{
	int ret;
	unsigned int cmd_type;
	Command_t cmd ={0};

	cmd_type = (mem_tmp[0]<<8)|(mem_tmp[1]);
	switch(cmd_type)
	{
		case GET_MIC_VERSION:
			ret = VCGetVersion(&cmd);
			printk("version=%d\n", ret);
			break;
		case GET_MIC_WAITREADY:
			ret = VCWaitReady(&cmd, 0);
			printk("MIC Module %s Wait Ready!\n", ret?"":"Not" );
			break;
		case GET_MIC_CURFUNCTION:
			ret = VCGetFunc(&cmd);
			if (ret != (command&0x0F))
			{
				// ������
				printk("The Func now is %d!\n", ret);
			}
			break;
		default:
			break;
	}

	mem_tmp[2] = ret;
//	int_to_bytes(ret, mem_tmp, 1);
	if (copy_to_user(buf, (void*) mem_tmp, 5))
	{
		return -EFAULT;
	}

	return count;
}

static long mic_cdev_ioctl(struct file * filep, unsigned int command, unsigned long arg)
{
	unsigned long ret = -1;
	Command_t cmd ={0};

	/* �����л�*/
	switch(command)
	{
		case IOCTL_CMD_I2C_FUNC_MODE_PASSBY:
			/* �ر����й��ܣ����ṩ¼������ */
			ret = VCChangeFunc(&cmd, FUNC_MODE_PASSBY);
			break;
		case IOCTL_CMD_I2C_FUNC_MODE_NOISECLEAN:
			/* �л������빦��*/
			ret = VCChangeFunc(&cmd, FUNC_MODE_NOISECLEAN);
			break;
		case IOCTL_CMD_I2C_FUNC_MODE_PHONE:
			/* �����绰������������*/
			ret = VCChangeFunc(&cmd, FUNC_MODE_PHONE);
			break;
		case IOCTL_CMD_I2C_FUNC_MODE_WAKEUP:
			/* �л������ѻ�����������*/
			ret = VCConfigWakeupID(&cmd, 0);			// ���ñ��ػ��Ѵʡ��������������
			ret = VCConfigWakeupGpio(&cmd, 1, 480);		// ���ñ��ػ��ѵĻ����ź�Ϊ�ߵ�ƽ��Ч���źų�480ms
			ret = VCChangeFunc(&cmd, FUNC_MODE_WAKEUP);
			break;
		default:
			/* Ĭ���л������빦��*/
			ret = VCChangeFunc(&cmd, FUNC_MODE_NOISECLEAN);
			break;
	}

	if(ret != 0)
	{
		printk("Set Func Error!\n");
	}
	/*  ��鵱ǰ�Ĺ���*/
	ret = VCGetFunc(&cmd);
	if (ret != (command&0x0F))
	{
		// ������
		printk("Get Func is not the same of Set!\n");
	}
	
	if( copy_to_user((int *)arg, &ret, 1) )
	{
		return -EFAULT;
	}
	return 0;
}

static ssize_t mic_cdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int cmd_type, mode_type, ret;
	Command_t cmd ={0};

	/* ��ȡʵ�ʳ��� */
	memset(mem_tmp, 0, count);
	if (copy_from_user(mem_tmp, buf, count))
	{
		return -EFAULT;
	}
	else
	{
		cmd_type = mem_tmp[0];
		switch(cmd_type)
		{
			case SET_MIC_WOEKMODE:
//				mode_type = (cmd_work_mode_id)bytes_to_int(mem_tmp, 1);
				mode_type = (cmd_work_mode_id)mem_tmp[1];
				ret = VCChangeWorkMode(&cmd, mode_type);
				break;
			case SET_MIC_FUNCTION:
//				ret = VCChangeFunc(&cmd, (cmd_func_id)bytes_to_int(mem_tmp, 1));
				mode_type = (cmd_func_id)mem_tmp[1];
				ret = VCChangeFunc(&cmd, mode_type);
				if(mode_type == FUNC_MODE_WAKEUP)
				{
					VCConfigWakeupID(&cmd, 0);			// ���ñ��ػ��Ѵʡ��������������
					VCConfigWakeupGpio(&cmd, 1, 480);	// ���ñ��ػ��ѵĻ����ź�Ϊ�ߵ�ƽ��Ч���źų�480ms
				}
				break;
			case SET_MIC_DACVOLUME:
				mode_type = (cmd_dac_gain_id)mem_tmp[1];
				ret = VCSetDACVolume(&cmd, mode_type);
				break;
			case SET_MIC_RESET:
				mic_hardware_reset();
				ret = mic_software_init();
				printk("MIC RESET %s.\n", ret?"success":"fail");
				break;
			default:
				break;
		}
		printk("Set MIC %d %s.\n", mode_type, (ret == 0)?"success":"fail");
	}

	return count;
}

static int mic_cdev_close(struct inode *inode,struct file *filp)
{
	return 0;
}

static const struct file_operations cdev_fops =
{
	.owner			= THIS_MODULE,
	.open			= mic_cdev_open,
	.read			= mic_cdev_read,
	.unlocked_ioctl	= mic_cdev_ioctl,
	.write			= mic_cdev_write,
	.release		= mic_cdev_close,
};

static int mic_cdev_init(void)
{
	int ret = 0, index = 0;
	char name[100];
	dev_t devno = MKDEV(globalmem_major, index);

	if(globalmem_major)
	{
		register_chrdev_region(devno, 1, DEVICE_NAME);
	}
	else
	{
		ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
		globalmem_major = MAJOR(devno);
	}

	cdev_init(&cdev, &cdev_fops);
	ret = cdev_add(&cdev, devno, 1);
	if(ret)
		printk(KERN_NOTICE "Error %d adding mic cdev %d", ret, index);

	sprintf(name, "%s-class-%d", DEVICE_NAME, index);
	globalmem_cdev_class = class_create(THIS_MODULE, name);
	if(IS_ERR(globalmem_cdev_class))
	{
		printk("Err: failed in creating class.\n");
		return -1; 
	}

	sprintf(name, "%s-%d", DEVICE_NAME, index);
	device_create(globalmem_cdev_class, NULL, devno, NULL, name);
	return 0;
}


static ssize_t mic_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int ret_val;
	Command_t cmd ={0};

#if 1
	ret_val = VCGetWorkMode(&cmd);
	printk("VCGetWorkMode=%d\n", ret_val);
	ret_val = VCGetFunc(&cmd);
	printk("VCGetFunc=%d\n", ret_val);
#endif
	return 0;
}

static ssize_t mic_store(struct device *dev, struct device_attribute *attr, const char * buf, size_t count)
{
	int ret;
	Command_t cmd ={0};

	if(strncmp(buf, "mic_off", 7) == 0)
	{
		ret = gpio_direction_output(GPIO_MIC_RESET, 0);	/* MIC Disable */
		gpio_set_value(GPIO_MIC_RESET, 0);
		printk("SET RESET PIN low %s.\n", ret?"success":"fail");
	}
	else if(strncmp(buf, "mic_on", 6) == 0)
	{
		ret = gpio_direction_output(GPIO_MIC_RESET, 1);	/* MIC Enable */
		gpio_set_value(GPIO_MIC_RESET, 1);
		printk("SET RESET PIN hight %s.\n", ret?"success":"fail");
	}
	else if(strncmp(buf, "mic_wait", 8) == 0)
	{
		ret = VCWaitReady(&cmd, 0);
		printk("MIC Module %s Wait Ready!\n", ret?"":"Not" );
	}
	else if(strncmp(buf, "mic_reset", 9) == 0)
	{
		mic_hardware_reset();
		ret = mic_software_init();
		printk("MIC RESET %s.\n", ret?"success":"fail");
	}
	else if(strncmp(buf, "mic_passby", 10) == 0)
	{
		ret = VCChangeFunc(&cmd, FUNC_MODE_PASSBY);
		printk("Set MIC FUNC_MODE_PASSBY %s.\n", (ret == 0)?"success":"fail");
	}
	else if(strncmp(buf, "mic_nr", 6) == 0)
	{
		ret = VCChangeFunc(&cmd, FUNC_MODE_NOISECLEAN);
		printk("Set MIC FUNC_MODE_NOISECLEAN %s.\n", (ret == 0)?"success":"fail");
	}
	else if(strncmp(buf, "mic_phone", 9) == 0)
	{
		ret = VCChangeFunc(&cmd, FUNC_MODE_PHONE);
		printk("Set MIC FUNC_MODE_PHONE %s.\n", (ret == 0)?"success":"fail");
	}
	else if(strncmp(buf, "mic_wakeup", 10) == 0)
	{
		ret = VCConfigWakeupID(&cmd, 0);			// ���ñ��ػ��Ѵʡ��������������
		ret = VCConfigWakeupGpio(&cmd, 1, 480);		// ���ñ��ػ��ѵĻ����ź�Ϊ�ߵ�ƽ��Ч���źų�480ms
		ret = VCChangeFunc(&cmd, FUNC_MODE_WAKEUP);
		printk("Set MIC FUNC_MODE_WAKEUP %s.\n", (ret == 0)?"success":"fail");
	}
	
	return count;
}

static DEVICE_ATTR(mic_gpio, S_IRUGO|S_IWUSR, mic_show, mic_store);

static struct attribute *mic_attrs[] = {
	&dev_attr_mic_gpio.attr, 
	NULL 
};

static struct attribute_group mic_group = {
	.name = NULL,
	.attrs = mic_attrs,
};

static int __devinit mic_probe(struct i2c_client *client, const struct i2c_device_id *did)
{
	int retval;

	mic_i2c_client = client;

	mic_hardware_reset();
	mic_software_init();
	retval = sysfs_create_group(&client->dev.kobj, &mic_group);
	if (retval)
	{
		printk(KERN_ERR "Can't create sysfs attrs for vty-server@%X\n", retval);
		return retval;
	}

	dev_info(&client->dev, "initialized\n");

	return 0;
}

static int __devexit mic_remove(struct i2c_client *client)
{
	mic_i2c_client = NULL;
	
	sysfs_remove_group(&client->dev.kobj, &mic_group);

	return 0;
}

static const struct i2c_device_id mic_id[] =
{
	{ MIC_I2C_NAME, 0 },
	{  },
};

static struct i2c_driver mic_dirver = 
{
	.driver = 
	{
		.name	= MIC_I2C_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= mic_probe,
	.remove		= __devexit_p(mic_remove),
	.id_table	= mic_id,
};

static int __init mic_module_init(void)
{
	struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int ret = 0;
    
	mic_cdev_init();

	memset(&info, 0, sizeof(struct i2c_board_info));

	info.addr = MIC_I2C_ADDRESS_LOW;
//	info.platform_data=pdata;
    strlcpy(info.type, MIC_I2C_NAME, MIC_NAME_SIZE);

    adapter = i2c_get_adapter(1);
    if (!adapter)
    {
        printk(KERN_ERR "Can't get i2c adapter 1\n");
        return -ENODEV;
    }

    client = i2c_new_device(adapter, &info);
    i2c_put_adapter(adapter);
	if (!client)
    {
        printk(KERN_ERR "Can't add i2c device at 0x%x\n", (unsigned int)info.addr);
        return -ENOMEM;
    }

	return i2c_add_driver(&mic_dirver);
}

static void __exit mic_module_exit(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
	i2c_del_driver(&mic_dirver);
}

module_init(mic_module_init);
module_exit(mic_module_exit);
MODULE_DESCRIPTION("MIC control");
MODULE_AUTHOR("HCN");
