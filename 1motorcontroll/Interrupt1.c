/*****************************************************************************
* FILE NAME:  Interrupt1.c - ���荞�݃n���h������ interrupt Pci(4423, 3346)
* 
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>
#include <pcibus.h>
#include <string.h>

#include "1motorcontroll.h"

char				indata;	// �n���h���Ŏg���A6���Ȃ�[5]�̔z���ݒ肷��

// �n���h���ƃX���b�h�Ԃŋ��L����郂�W���[�����x���̃f�[�^�ϐ�
static WORD			wLevel;
static PCIDEV		dev;
static WORD			iobase;
static DWORD*		regs;
static MSI_PARAM_EX	msi;

/*****************************************************************************
* FUNCTION:			Interrupt1
*
* DESCRIPTION:
*	���荞�݂̍ł��N���e�B�J���ȏ����ł�
*	�{�����͓K�؂ȒP���ȃI�u�W�F�N�g�ւ̃V�O�i������A
*	��蕡�G��I/O�|�[�g�̏����܂ł��܂��܂ł�
* NOTE:         
*	�n���h���ł͌���ꂽINtime�@�\�̂ݗ��p�ł��܂�
\*****************************************************************************/
__INTERRUPT void	Interrupt1(
	WORD				wCSRA,
	WORD				wLevel,
	PVOID				pv)
{
	// TODO: ���[�J���ϐ��͂����ɐ錾���܂�

	__SHARED_INTERRUPT_PROLOG();

	// �n���h�����ł͗�O����͕s�v�ł�: ������\�����邱�Ƃ��ł��܂���

	// TODO: ���荞�݃n���h���R���e�L�X�g�ɂ����鏈��
	//�ϊ��������N�_�ɂ���
	indata = inbyte(iobase + 0);

	//�N���A��������Ȃ��Ƃ܂�������������Ȃ��c

	SignalRtInterruptThread(wLevel);

	__SHARED_INTERRUPT_RETURN();
}

/*****************************************************************************
* FUNCTION:			Interrupt1Thread
*
* DESCRIPTION:
*	�ʏ�X���b�h�̌`���Ŋ��荞�݂������܂�
\*****************************************************************************/
void				Interrupt1Thread(
	void*				context)
{
	// MSI_LEVEL == wLevel�̂Ƃ��̂݁A&msi�n�����K�v�ƂȂ�܂�
	wLevel			= SetRtInterruptHandlerEx(wLevel, 16, (LPPROC)Interrupt1, &msi);
	if (BAD_LEVEL == wLevel)
	{
		Fail("Cannot set interrupt handler for Pci(4423, 3346)");
		return;
	}

	// ���荞�݂����݂��Ă��邱�Ƃ�o�^
	gInit.bInterrupt1	= TRUE;

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, GetRtThreadHandles(THIS_THREAD), "TInterrupt1");

	//���[���{�b�N�X���M�p
	RTHANDLE		hProcess = NULL_RTHANDLE;
	RTHANDLE		hDmbx;
	hDmbx = LookupRtHandle(hProcess, "Server2", WAIT_FOREVER);
	gInit.bInterrupt1 = GetRtThreadHandles(THIS_THREAD);



	if (BAD_RTHANDLE == hDmbx)
	{
		Fail("Cannot find data mailbox");
		return;

	while (!gInit.bShutdown)
	{
		if (!WaitForRtInterrupt(wLevel, WAIT_FOREVER))
		{
			Fail("Cannot wait for interrupt signal for Pci(4423, 3346)");
			break;
		}

		// TODO: RT�v���Z�X�R���e�L�X�g�ɂ����銄�荞�݂��������܂��B
		RtSleep(10);
		// TODO: fill a message and its size
		if (!SendRtData(hDmbx, "fin!", 5))
		{
			Fail("Cannot send to data mailbox");
			break;
		}

	}
	/*
	RTHANDLE		hProcess = NULL_RTHANDLE;
	RTHANDLE		hDmbx;

	// TODO: �v���Z�X�n���h���ƃ��[���{�b�N�X�̖��̂𒲐߂��܂�
	// TODO: �f�[�^�^���[���{�b�N�X�����̃v���Z�X����������ꍇ�A���̐��s�͍폜���Ă�������
	hProcess = LookupRtHandle(hRootProcess, "DMBX_OWNER", WAIT_FOREVER);
	if (BAD_RTHANDLE == hProcess)
	{
		Fail("Cannot find data mailbox process");
		return;
	}

	// TODO: �f�[�^�^���[���{�b�N�X�����̃v���Z�X����������ꍇ�AhProcess��NULL_RTHANDLE�ɒu�������܂�
	hDmbx = LookupRtHandle(hProcess, "DMBX_NAME", WAIT_FOREVER);
	if (BAD_RTHANDLE == hDmbx)
	{
		Fail("Cannot find data mailbox");
		return;
	}

	// �{�X���b�h���������Ă��邱�Ƃ�o�^
	gInit.htClient1 = GetRtThreadHandles(THIS_THREAD);

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, gInit.htClient1, "TClient1");

	while (!gInit.bShutdown)
	{
		// TODO: �J��Ԃ��s�������������ɔz�u���܂�
		// ����RtSleep�͗�ɂ����܂���
		RtSleep(1000);
		// TODO: fill a message and its size
		if (!SendRtData(hDmbx, "test", 5))
		{
			Fail("Cannot send to data mailbox");
			break;
		}
	}
	*/

	Interrupt1Kill();
}

/*****************************************************************************
* FUNCTION:     Interrupt1Init
*
* DESCRIPTION:
*  ���荞�݃n���h���Ɗ��荞�݃X���b�h���g�p���܂�
\*****************************************************************************/
void				Interrupt1Init(void)
{
	//DWORD	nVectors;

	// �ݒ肳�ꂽ�p�����[�^��p���āA�f�o�C�X���������܂�
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

	// BAR0���Ń������̃x�[�X�A�h���X���������A�}�b�v����T���v���R�[�h�ł�
	if (dev.IoSpace[0].flags & IOSPACE_MEM)
		regs = (DWORD*)MapRtPhysicalMemory(dev.IoSpace[0].start, dev.IoSpace[0].size);
	// BAR0���Ń��������W�X�^�ɃA�N�Z�X���邽�߂ɁA"regs"���g�p�ł��܂�

	// BAR1����I/O�̃x�[�X�A�h���X����������T���v���R�[�h�ł�
	if (dev.IoSpace[1].flags & IOSPACE_IO)
		iobase = (WORD)dev.IoSpace[1].start;
	// inbyte�֐���outbyte�֐���p���āAI/O���W�X�^�ɃA�N�Z�X���邽�߂ɁA"iobase"���g�p���邱�Ƃ��ł��܂��B

	//3346���荞�ݏ����ݒ�
	inbyte(iobase + 0x0D, 0x40);		//���荞�ݗv���X�e�[�^�X�ADA�J�n���荞�݁@�v���O���}�̉\���͂�����
	inbyte(iobase + 0x0F, 0x40);		//���荞�ݗv���X�e�[�^�X�ADA�J�n���荞�݁@

	// MSI�ŃT�|�[�g����Ă���f�o�C�X�̏ꍇ�́A�K�v�ɉ����āA�����݃n���h����ݒ肵�Ă����������T�|�[�g����ĂȂ�
	/*if (0 != dev.MsiOffset)
	{
		nVectors		= PciGetMsiCount(&dev);
		wLevel			= MSI_LEVEL;
		msi.PciAddress	= MKPCIADDR((&dev));
		msi.Param		= NULL; // �����ł̓p�����[�^��K�v�Ƃ��܂���
		if (nVectors > 1)
		{
			msi.MsiIndex   		= 0;    // �ǂ�MSI���荞�݌������w��
			msi.MsiVectors 		= AllocateRtInterrupts(nVectors);
			msi.ReservedZero	= 0;  // �����̌݊����̂���0���Z�b�g���Ȃ���΂Ȃ�Ȃ�
		}
		else
		{
			msi.MsiIndex		= 0;
			msi.MsiVectors		= 0;
			msi.ReservedZero	= 0;
		}
	}
	else
	{
		wLevel			= PciGetInterruptLevel(&dev);
		if (BAD_LEVEL == wLevel)
		{
			Fail("Unable to get interrupt for device %04x/%04x(%u)", dev.wVendorId, dev.wDeviceId, dev.wDeviceIndex);
			return;
		}
		wLevel			|= SHARED_LEVEL;
	}*/

	// �X���b�h�́A�v���Z�X�ŋ��e����Ă���v���C�I���e�B���A�����v���C�I���e�B���擾���A�����ݏ������\�Ƃ��܂�
	SetRtProcessMaxPriority(NULL_RTHANDLE, 0);
	//���荞�݃X���b�h�N��
	CreateRtThread(0, (LPPROC)Interrupt1Thread, 4194304, 0);
}

/*****************************************************************************
* FUNCTION:			Interrupt1Kill
*
* DESCRIPTION:
*  ���荞�݃n���h�����I�����܂�
\*****************************************************************************/
BOOLEAN				Interrupt1Kill(void)
{
	// ���荞�݂��I�����Ă��邱�Ƃ�o�^
	gInit.bInterrupt1	= FALSE;
	return ResetRtInterruptHandler(wLevel);
}
