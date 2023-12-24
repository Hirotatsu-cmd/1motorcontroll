/*****************************************************************************
* Poll2.c - �|�[�����O�̂��߂̃��W���[�� Poll2
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>
#include <pcibus.h>
#include <stdlib.h>


#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:		Poll2
* DESCRIPTION:
*   �|�[�����O�X���b�h Poll2
\*****************************************************************************/
void				Poll2(
	void*				param)
{
#ifdef _DEBUG
	fprintf(stderr, "Poll2 started\n");
#endif

	// �{�X���b�h���������Ă��邱�Ƃ�o�^
	gInit.htPoll2	= GetRtThreadHandles(THIS_THREAD);

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, gInit.htPoll2, "TPoll2");

	//3346a�����ݒ�

	PCIDEV			dev;
	//����iobase�g��
	DWORD dwBaseAddr;

	dev.wVendorId = 4423;
	dev.wDeviceId = 3346;
	dev.wDeviceIndex = 0; // Instance;
	if (!PciFindDevice(&dev))
		Fail("PCI device %04x/%04x(%u) not found\n", dev.wVendorId, dev.wDeviceId, dev.wDeviceIndex);

	if (dev.bUnusable)
		Fail("PCI device %04x/%04x(%u) not assigned to this node", dev.wVendorId, dev.wDeviceId, dev.wDeviceIndex);

	// �d����Ԃ�D0���(��Ə��)�Ɉڍs�����A�������y��I/O�ւ̃A�N�Z�X���\�ɂ��܂�
	PciEnableDevice(&dev);

	// �K�v�ɉ����āA�o�X �}�X�^�@�\��ݒ肵�܂�
	PciSetMaster(&dev);

	// �x�[�X�A�h���X�̎擾
	dwBaseAddr = (WORD)dev.dwBaseAddr[0] & 0xfffc;

	/* dwBaseaddr�Ŏ擾�ł��Ȃ������炱�����g��
	// BAR1����I/O�̃x�[�X�A�h���X����������T���v���R�[�h�ł�
	//if (dev.IoSpace[1].flags & IOSPACE_IO)
	//	iobase = (WORD)dev.IoSpace[1].start;
	// inbyte�֐���outbyte�֐���p���āAI/O���W�X�^�ɃA�N�Z�X���邽�߂ɁA"iobase"���g�p���邱�Ƃ��ł��܂��B
	*/

	outbyte(dwBaseAddr + 0x18, 0x01);		//�A�i���O�o��
	outbyte(dwBaseAddr + 0x05, 0x03);		//DA�ϊ����[�h�A�S�`�����l���o��enable(6�����l����)
	outbyte(dwBaseAddr + 0x02, 0x00);		//DA�o�̓`�����l���ݒ�A�`�����l��1�A6����0x05�܂Œǉ�
	outbyte(dwBaseAddr + 0x06, 0x02);		//�o�̓����W�ݒ�A�}5V


	//�R�s�y�AServer1�����]�p���󂯎��
	//�F�X�������Ă�̂͊ԈႢ�Ȃ�
	WORD			wActual;
	BYTE			byMessage[128];
	RTHANDLE		hPoll2;

	WORD			wActual2;
	BYTE			byMessagef[128];
	RTHANDLE		hDmbx3;

#ifdef _DEBUG
	fprintf(stderr, "Thread for Poll2 started\n");
#endif
	// TODO: �t���O���K�؂łȂ���ΕύX���Ă�������
	hPoll2 = CreateRtMailbox(DATA_MAILBOX | FIFO_QUEUING);
	if (BAD_RTHANDLE == hPoll2)
	{
		Fail("Cannot create data mailbox Poll2");
		return;
	}
	if (!Catalog(NULL_RTHANDLE, hPoll2, "Poll2"))
	{
		Fail("Cannot catalog data mailbox Poll2");
		return;
	}


	while (!gInit.bShutdown)
	{
		RtSleep(10);

#ifdef _DEBUG
		fprintf(stderr, "Poll2 waking up\n");
#endif


#ifdef _DEBUG
		fprintf(stderr, "Thread for Poll2 waiting\n");
#endif

			// TODO: �^�C���A�E�g�l���K�؂łȂ���ΕύX���Ă�������
		wActual = ReceiveRtData(hPoll2, byMessage, WAIT_FOREVER);
		if (0 == wActual)
		{
			Fail("Receive from data mailbox Poll2 failed");
			break;
		}

		// TODO: byMessage(�K�v�ɉ�����dwActual)�𗘗p���A�������J�n�ł��܂�

		//byMessage��ϊ�

		outbyte(dwBaseAddr + 0x01, 0x00 + byMessage);			//��ʏo�͓d���f�[�^�̐ݒ�,255�x�܂ł͉��ʃr�b�g�����ŕ\���ł��適��R�@�M���l����Η]�T�Œ�����̂ŕ�����K�v����
		outbyte(dwBaseAddr + 0x00, 0x00 + byMessage);		 //���ʏo�͓d���f�[�^�̐ݒ�

		//Server2���犮���ʒm���󂯎��



#ifdef _DEBUG
			fprintf(stderr, "Thread for ServMbx started\n");
#endif
			// TODO: �t���O���K�؂łȂ���ΕύX���Ă�������
		hDmbx3 = CreateRtMailbox(DATA_MAILBOX | FIFO_QUEUING);
		if (BAD_RTHANDLE == hDmbx3)
		{
			Fail("Cannot create data mailbox Poll2");
			return;
		}
		if (!Catalog(NULL_RTHANDLE, hDmbx3, "Poll2"))
		{
			Fail("Cannot catalog data mailbox Poll2");
			return;
		}


			
#ifdef _DEBUG
			fprintf(stderr, "Thread for ServMbx waiting\n");
#endif

		// TODO: �^�C���A�E�g�l���K�؂łȂ���ΕύX���Ă�������
		wActual2 = ReceiveRtData(hDmbx3, byMessagef, WAIT_FOREVER);
		if (0 == wActual2)
		{
			Fail("Receive from data mailbox ServMbx failed");
			break;
		}
			// TODO: byMessage(�K�v�ɉ�����dwActual)�𗘗p���A�������J�n�ł��܂�
			printf("%s\n", byMessagef);
			//dmbx����Ȃ���
	}

	// �{�X���b�h�̏I����ʒm
	gInit.htPoll2	= NULL_RTHANDLE;

}
