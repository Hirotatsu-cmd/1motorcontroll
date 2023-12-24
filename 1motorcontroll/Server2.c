/*****************************************************************************
* Server2.c - スレッドモジュール Server2
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>

#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:   ServerServer2
*
* データメールボックスのスレッド Server2.
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
	// TODO: フラグが適切でなければ変更してください
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

	// 本スレッドが生存していることを登録
	gInit.htServer2	= GetRtThreadHandles(THIS_THREAD);

	// スレッドカタログ、ただし失敗は無視
	Catalog(NULL_RTHANDLE, gInit.htServer2, "TServer2");

	while (!gInit.bShutdown)
	{
#ifdef _DEBUG
		fprintf(stderr, "Thread for Server2 waiting\n");
#endif

		// TODO: タイムアウト値が適切でなければ変更してください
		wActual = ReceiveRtData(hServer2, byMessage, WAIT_FOREVER);
		if (0 == wActual)
		{
			Fail("Receive from data mailbox Server2 failed");
			break;
		}

		// TODO: byMessage(必要に応じてdwActual)を利用し、処理を開始できます
		

		// TODO: プロセスハンドルとメールボックスの名称を調節します
		// TODO: データ型メールボックスをこのプロセスが生成する場合、次の数行は削除してください
		/*hProcess = LookupRtHandle(hRootProcess, "DMBX_OWNER", WAIT_FOREVER);
		if (BAD_RTHANDLE == hProcess)
		{
			Fail("Cannot find data mailbox process");
			return;
		}
		*/
		// TODO: データ型メールボックスをこのプロセスが生成する場合、hProcessをNULL_RTHANDLEに置き換えます
		hDmbx4 = LookupRtHandle(hProcess4, "Poll2", WAIT_FOREVER);
		if (BAD_RTHANDLE == hDmbx4)
		{
			Fail("Cannot find data mailbox");
			return;
		}


		
		// TODO: 繰り返し行う処理をここに配置します
		// 次のRtSleepは例にすぎません
		RtSleep(10);
		// TODO: fill a message and its size
		//データサイズを5から2へ
		//bymessageで得られた情報を送る
		if (!SendRtData(hDmbx4, byMessage, 5))
		{
			Fail("Cannot send to data mailbox");
			break;
		}
		
	}

	// 本スレッドの終了を通知
	gInit.htServer2	= NULL_RTHANDLE;
}
