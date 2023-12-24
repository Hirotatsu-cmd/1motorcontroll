/*****************************************************************************
* Poll1.c - �|�[�����O�̂��߂̃��W���[�� Poll1
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>
#include <pcibus.h>
#include <stdlib.h>

#include "1motorcontroll.h"


/*****************************************************************************
* FUNCTION:		Poll1
* DESCRIPTION:
*   �|�[�����O�X���b�h Poll1
\*****************************************************************************/
void				Poll1(
	void*				param,
	WORD				wPciVendorId,
	WORD				wPciDeviceId)
{
#ifdef _DEBUG
	fprintf(stderr, "Poll1 started\n");
#endif

	// �{�X���b�h���������Ă��邱�Ƃ�o�^
	gInit.htPoll1	= GetRtThreadHandles(THIS_THREAD);

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, gInit.htPoll1, "TPoll1");

	/*************************
	��������ǉ��v�f
	*************************/

	PCIDEV			dev;
	//�G���[����̂��ߓ��ꂽ���ǐ��������킩���
	DWORD dwBaseAddr;
	
	dev.wVendorId = 4423;
	dev.wDeviceId = 6205;
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
	
	/* �擾�ł��Ȃ������炱�����g��
	// BAR1����I/O�̃x�[�X�A�h���X����������T���v���R�[�h�ł�
	//if (dev.IoSpace[1].flags & IOSPACE_IO)
	//	iobase = (WORD)dev.IoSpace[1].start;
	// inbyte�֐���outbyte�֐���p���āAI/O���W�X�^�ɃA�N�Z�X���邽�߂ɁA"iobase"���g�p���邱�Ƃ��ł��܂��B
	*/

	/***************************
	�񎲂���̓x�[�X�A�h���X���ȏ�擾���č�Ƃ�i�߂�
	****************************/


	//�S�̓I�ɐݒ肪�������̂Ō�Ō���
	long paluse;		//�p���X���̌��_
	long degree;		//�p�x

	outbyte(dwBaseAddr + 0x04, 0x06);       // ���[�h���W�X�^�ݒ�A�ʑ����p���X4���{�񓯊��N���A�A�K�C�hp7�Q��
	outbyte(dwBaseAddr + 0x06, 0x00);       // �\�t�g�E�F�A���b�`

	//���[���{�b�N�X�ݒ�
	//���[���{�b�N�X�g����server1�ɑ���
	//�ȉ�client�̃R�s�y
	RTHANDLE		hProcess = NULL_RTHANDLE;
	RTHANDLE		hDmbx;

	// TODO: �v���Z�X�n���h���ƃ��[���{�b�N�X�̖��̂𒲐߂��܂�
	// TODO: �f�[�^�^���[���{�b�N�X�����̃v���Z�X����������ꍇ�A���̐��s�͍폜���Ă�������
	/*hProcess = LookupRtHandle(hRootProcess, "DMBX_OWNER", WAIT_FOREVER);
	if (BAD_RTHANDLE == hProcess)
	{
		Fail("Cannot find data mailbox process");
		return;
	}
	*/
	// TODO: �f�[�^�^���[���{�b�N�X�����̃v���Z�X����������ꍇ�AhProcess��NULL_RTHANDLE�ɒu�������܂�
	hDmbx = LookupRtHandle(hProcess, "Server1", WAIT_FOREVER);
	if (BAD_RTHANDLE == hDmbx)
	{
		Fail("Cannot find data mailbox");
		return;
	}

	// �{�X���b�h���������Ă��邱�Ƃ�o�^
	gInit.htPoll1 = GetRtThreadHandles(THIS_THREAD);

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, gInit.htPoll1, "TPoll1");
	
	//�J�E���g���W�b�N�A�������[�v�Ŕ��P�v�I�ȃp���X���o�A���s���Ԃ��o�������������ł����̂��H
	//8bit�J�E���g������A256�p���X�܂ł������̂܂܂��ƃJ�E���g�ł��Ȃ��΍�̃R�[�h�����������Ⴂ�A���base�ɂ�24bit������
	//

	while (!gInit.bShutdown)
	{
		outbyte(dwBaseAddr + 0x10, 0x00);       // ���[�^1�J�E���^�������݉���
		outbyte(dwBaseAddr + 0x11, 0x00);       // ���[�^1�J�E���^�������ݒ���
		outbyte(dwBaseAddr + 0x12, 0x00);       // ���[�^1�J�E���^�������ݏ��
		outbyte(dwBaseAddr + 0x00, 0x00);       // ���[�^1�J�E���^�ǂݏo������
		outbyte(dwBaseAddr + 0x01, 0x00);       // ���[�^1�J�E���^�ǂݏo������
		outbyte(dwBaseAddr + 0x02, 0x00);       // ���[�^1�J�E���^�ǂݏo�����
		outbyte(dwBaseAddr + 0x05, 0x15);       // �X�e�[�^�X���W�X�^�ǂݍ��݁A�O���M�����ׂ�LOW

		paluse = outbyte(dwBaseAddr + 0x05, 0x15);

		degree = paluse * 0x2328 / 0x4147;		//0x4147�͉��ړ��̃p���X�����A���[�^�[�̉�]�͈͂�0�`90���̃M�A��100�{�A�܂�9000��

		printf("���݂̃��[�^�[1�̊p�x��%d�ł�\n", degree);

		RtSleep(10);

#ifdef _DEBUG
		fprintf(stderr, "Poll1 waking up\n");
#endif

		// TODO:  100 �~���b���ƂɌJ��Ԃ�������z�u���܂�
		//client���R�s�y���ă��[���{�b�N�X���M
		//�T�C�Y�������ĂȂ�
		if (!SendRtData(hDmbx, degree, 5))
		{
			Fail("Cannot send to data mailbox");
			break;
		}

	}

	// �{�X���b�h�̏I����ʒm
	gInit.htPoll1	= NULL_RTHANDLE;
}
