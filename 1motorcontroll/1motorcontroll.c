/*****************************************************************************
* FILE NAME:    1motorcontroll.c - ���C���v���O�������W���[��
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <rt.h>

#include "1motorcontroll.h"

// �O���[�o���ϐ�
RTHANDLE			hRootProcess;
DWORD				dwKtickInUsecs;
INIT_STRUCT			gInit;

/*****************************************************************************
* FUNCTION:     main
*
* DESCRIPTION:
*  ����̓��C���v���O�������W���[���ł�
*  ���L��������A���ׂẴX���b�h�A���ׂẴI�u�W�F�N�g�𐶐����܂�
*  ����main�֐����I������܂ł̊ԃv���Z�X�Ƃ��đ��݂��A�I�����ɂ����̂��ׂĂ͍폜����܂�
\*****************************************************************************/
int					main(int argc, char* argv[])
{
	SYSINFO			sysinfo;
	EVENTINFO		eiEventInfo;
#ifdef _DEBUG
	fprintf(stderr, "1motorcontroll started\n");
#endif

	// ���[�g�v���Z�X���擾����i���s���܂���j
	hRootProcess	= GetRtThreadHandles(ROOT_PROCESS);

	// ���W���[�����\���̂��N���A���܂�
	memset(&gInit, 0, sizeof(gInit));
	gInit.state		= BEFORE_INIT;

	// �჌�x���e�B�b�N�l�i�}�C�N���b�j���擾���܂�
	if (!CopyRtSystemInfo(&sysinfo))
		Fail("Cannot copy system info");
	dwKtickInUsecs	= 10000 / sysinfo.KernelTickRatio;
	if (dwKtickInUsecs == 0)
		Fail("Invalid low level tick length");

	// �v���Z�X�ő�v���C�I���e�B�𒲐����܂�(���s�͖���)
	// TODO adjust 2�Ԗڂ̃p�����[�^�𐔒l�I�ɏ��������邱�Ƃł�荂���v���C�I���e�B�X���b�h���������e���܂�
	// ����: ���荞�݃X���b�h�������������ɂ͖{�l��0�Ƃ��Đݒ肵�܂�
	SetRtProcessMaxPriority(NULL_RTHANDLE, 150);

	// ���C���X���b�h(���̊֐�)�̃n���h�����擾���܂�
	gInit.hMain		= GetRtThreadHandles(THIS_THREAD);
	gInit.state		= INIT_BUSY;

	// �X���b�h�J�^���O�����݂܂����A���s�͖������܂�
	Catalog(NULL_RTHANDLE, gInit.hMain, "TMain");

	// ���[�g�v���Z�X�ɖ{�v���Z�X���J�^���O���܂�
	if (!Catalog(hRootProcess, GetRtThreadHandles(THIS_PROCESS), "1motorcontro"))
		Fail("Cannot catalog process name");
	gInit.bCataloged = TRUE;

	// create mailbox, semaphore and message queue threads
	if (BAD_RTHANDLE == CreateRtThread(160, (LPPROC)ServerServer1, 4194304, 0))
		Fail("Cannot create thread for Server1");
	if (BAD_RTHANDLE == CreateRtThread(160, (LPPROC)ServerServer2, 4194304, 0))
		Fail("Cannot create thread for Server2");

	// ���荞�݃n���h���𐶐����܂�
	Interrupt1Init();

	// �|�[�����O�X���b�h�𐶐����܂�
	if (BAD_RTHANDLE == CreateRtThread(170, (LPPROC)Poll1, 4194304, 0))
		Fail("Cannot create poll thread Poll1");
	if (BAD_RTHANDLE == CreateRtThread(170, (LPPROC)Poll2, 4194304, 0))
		Fail("Cannot create poll thread Poll2");

	// �������������������Ƃ������܂�
	gInit.state		= INIT_DONE;
#ifdef _DEBUG
	fprintf(stderr, "1motorcontroll finished initialization\n");
#endif

	// �C�x���g��ҋ@���܂�
	while (RtNotifyEvent(RT_SYSTEM_NOTIFICATIONS | RT_EXIT_NOTIFICATIONS,
		WAIT_FOREVER, &eiEventInfo))
	{
		switch(eiEventInfo.dwNotifyType)
		{
		case TERMINATE:
			// TODO: �{�v���Z�X�͏I�����܂�
			// �����N���[���A�b�v���܂�
			Cleanup();  // ��������߂邱�Ƃ͂���܂���

		case NT_HOST_UP:
			// TODO: RT�N���C�A���g�ɂ�����NT�z�X�g�̕��A�Ή�������
			break;

		case NT_BLUESCREEN:
			// TODO: Windows�̃u���[�X�N���[�����o�Ή�����
			break;

		case KERNEL_STOPPING:
			// TODO: INtime�J�[�l���T�[�r�X��~���Ή�������
			break;

		case NT_HOST_HIBERNATE:
			// TODO: Windows�z�X�g�̋x�~��Ԉڍs���Ή�����
			break;

		case NT_HOST_STANDBY:
			// TODO: Windows�z�X�g�̃X�^���o�C��Ԉڍs���Ή�����
			break;
			break;

		case NT_HOST_SHUTDOWN_PENDING:
			// TODO: Windows�z�X�g�̃V���b�g�_�E���������Ή�����
			break;
		}
	}
	Fail("Notify failed");
	return 0;
}
