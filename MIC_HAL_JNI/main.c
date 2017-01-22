#include "vc_ctrl_api.h"
#include "vc_i2c_command.h"

// ����ʾ��


int  VCI2CInit(const char * devname);

void  VCI2CTerminate(void);

int main(int argc, char *argv[])
{
	int32_t ret = -1;
	int rv;
	Command_t cmd ={0};

	if(argc < 2)
	{
		printf("Please enter as | ntr-ctrl /dev/name \n");
		return 0;
	}
	rv = VCI2CInit(argv[1]);
	printf("VCI2CInit rv = %d\n", rv);
	if(rv < 0)
		return -1;
	
//	VCI2CSleep(2500); // ģ������
	
	// �����л�
	// �ϵ��������л���¼�����ܡ�һ�㲻����ʶ��ͻ���ʱ����ʹ��ģ���¼�����ܡ�
	ret = VCChangeFunc(&cmd, FUNC_MODE_PASSBY);
	printf("VCChangeFunc(&cmd, FUNC_MODE_PASSBY) ret = %d\n", ret);
	// ����ʶ��ǰ�л������빦��
	ret = VCChangeFunc(&cmd, FUNC_MODE_NOISECLEAN);
	// ��������ʱ�л������ѻ�����������
	ret = VCChangeFunc(&cmd, FUNC_MODE_WAKEUP);
	printf("VCChangeFunc(&cmd, FUNC_MODE_WAKEUP) ret = %d\n", ret);
	// ��绰ʱ�л����绰�����������ܡ�ͨ���㲻��Ҫ�˹��ܡ�
	// ע����ĵ绰ϵͳ�������˻����������ܡ�����Ҫʹ�����ǵĵ绰����������
	// ��绰ʱ��ֻҪ��ģ���л���¼�����ܼ��ɡ�
	ret = VCChangeFunc(&cmd, FUNC_MODE_PHONE);
	
	VCI2CTerminate();

	return 0;
}
