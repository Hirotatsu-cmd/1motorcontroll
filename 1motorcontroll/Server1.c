/*****************************************************************************
* Server1.c - �X���b�h���W���[�� Server1
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>

#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:   ServerServer1
*
* �f�[�^���[���{�b�N�X�̃X���b�h Server1.
\*****************************************************************************/
void ServerServer1(void* param)
{
	WORD			wActual;
	BYTE			byMessage[128];
	RTHANDLE		hServer1;
	RTHANDLE		hProcess2 = NULL_RTHANDLE;
	RTHANDLE		hDmbx2;


#ifdef _DEBUG
	fprintf(stderr, "Thread for Server1 started\n");
#endif
	// TODO: �t���O���K�؂łȂ���ΕύX���Ă�������
	hServer1	= CreateRtMailbox(DATA_MAILBOX | FIFO_QUEUING);
	if (BAD_RTHANDLE == hServer1)
	{
		Fail("Cannot create data mailbox Server1");
		return;
	}
	if (!Catalog(NULL_RTHANDLE, hServer1, "Server1"))
	{
		Fail("Cannot catalog data mailbox Server1");
		return;
	}

	// �{�X���b�h���������Ă��邱�Ƃ�o�^
	gInit.htServer1	= GetRtThreadHandles(THIS_THREAD);

	// �X���b�h�J�^���O�A���������s�͖���
	Catalog(NULL_RTHANDLE, gInit.htServer1, "TServer1");

	hDmbx2 = LookupRtHandle(hProcess2, "Poll2", WAIT_FOREVER);
	if (BAD_RTHANDLE == hDmbx2)
	{
		Fail("Cannot find data mailbox");
		return;
	}


	while (!gInit.bShutdown)
	{
#ifdef _DEBUG
		fprintf(stderr, "Thread for Server1 waiting\n");
#endif

		// TODO: �^�C���A�E�g�l���K�؂łȂ���ΕύX���Ă�������
		wActual = ReceiveRtData(hServer1, byMessage, WAIT_FOREVER);
		if (0 == wActual)
		{
			Fail("Receive from data mailbox Server1 failed");
			break;
		}

		RtSleep(10);
		// TODO: fill a message and its size
		//bymessage�œ���ꂽ���𑗂�
		if (!SendRtData(hDmbx2, byMessage, 5))
		{
			Fail("Cannot send to data mailbox");
				break;
		}
		

	}

	// �{�X���b�h�̏I����ʒm
	gInit.htServer1	= NULL_RTHANDLE;
}
