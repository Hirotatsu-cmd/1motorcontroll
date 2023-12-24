/*****************************************************************************
* Server2.c - �X���b�h���W���[�� Server2
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>

#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:   ServerServer2
*
* �f�[�^���[���{�b�N�X�̃X���b�h Server2.
\*****************************************************************************/
void ServerServer2(void* param)
{
	WORD			wActual;
	BYTE			byMessage[128];
	RTHANDLE		hServer2;

	RTHANDLE		hProcess4 = NULL_RTHANDLE;
	RTHANDLE		hDmbx4;

#ifdef _DEBUG
	fprintf(stderr, "Thread for Server2 started\n");
#endif
	// TODO: �t���O���K�؂łȂ���ΕύX���Ă�������
	hServer2	= CreateRtMailbox(DATA_MAILBOX | FIFO_QUEUING);
	if (BAD_RTHANDLE == hServer2)
	{
		Fail("Cannot create data mailbox Server2");
		return;
	}
	if (!Catalog(NULL_RTHANDLE, hServer2, "Server2"))
	{
		Fail("Cannot catalog data mailbox Server2");
		return;
	}

	// �{�X���b�h���������Ă��邱�Ƃ�o�^
	gInit.htServer2	= GetRtThreadHandles(THIS_THREAD);

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, gInit.htServer2, "TServer2");

	while (!gInit.bShutdown)
	{
#ifdef _DEBUG
		fprintf(stderr, "Thread for Server2 waiting\n");
#endif

		// TODO: �^�C���A�E�g�l���K�؂łȂ���ΕύX���Ă�������
		wActual = ReceiveRtData(hServer2, byMessage, WAIT_FOREVER);
		if (0 == wActual)
		{
			Fail("Receive from data mailbox Server2 failed");
			break;
		}

		// TODO: byMessage(�K�v�ɉ�����dwActual)�𗘗p���A�������J�n�ł��܂�
		

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
		hDmbx4 = LookupRtHandle(hProcess4, "Poll2", WAIT_FOREVER);
		if (BAD_RTHANDLE == hDmbx4)
		{
			Fail("Cannot find data mailbox");
			return;
		}


		
		// TODO: �J��Ԃ��s�������������ɔz�u���܂�
		// ����RtSleep�͗�ɂ����܂���
		RtSleep(10);
		// TODO: fill a message and its size
		//�f�[�^�T�C�Y��5����2��
		//bymessage�œ���ꂽ���𑗂�
		if (!SendRtData(hDmbx4, byMessage, 5))
		{
			Fail("Cannot send to data mailbox");
			break;
		}
		
	}

	// �{�X���b�h�̏I����ʒm
	gInit.htServer2	= NULL_RTHANDLE;
}
