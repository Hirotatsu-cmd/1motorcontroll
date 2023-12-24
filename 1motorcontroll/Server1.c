/*****************************************************************************
* Server1.c - スレッドモジュール Server1
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>

#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:   ServerServer1
*
* データメールボックスのスレッド Server1.
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
	// TODO: フラグが適切でなければ変更してください
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

	// 本スレッドが生存していることを登録
	gInit.htServer1	= GetRtThreadHandles(THIS_THREAD);

	// スレッドカタログ、ただし失敗は無視
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

		// TODO: タイムアウト値が適切でなければ変更してください
		wActual = ReceiveRtData(hServer1, byMessage, WAIT_FOREVER);
		if (0 == wActual)
		{
			Fail("Receive from data mailbox Server1 failed");
			break;
		}

		RtSleep(10);
		// TODO: fill a message and its size
		//bymessageで得られた情報を送る
		if (!SendRtData(hDmbx2, byMessage, 5))
		{
			Fail("Cannot send to data mailbox");
				break;
		}
		

	}

	// 本スレッドの終了を通知
	gInit.htServer1	= NULL_RTHANDLE;
}
