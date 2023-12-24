/*****************************************************************************
* FILE NAME:    1motorcontroll.c - メインプログラムモジュール
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <rt.h>

#include "1motorcontroll.h"

// グローバル変数
RTHANDLE			hRootProcess;
DWORD				dwKtickInUsecs;
INIT_STRUCT			gInit;

/*****************************************************************************
* FUNCTION:     main
*
* DESCRIPTION:
*  これはメインプログラムモジュールです
*  共有メモリや、すべてのスレッド、すべてのオブジェクトを生成します
*  このmain関数が終了するまでの間プロセスとして存在し、終了時にそれらのすべては削除されます
\*****************************************************************************/
int					main(int argc, char* argv[])
{
	SYSINFO			sysinfo;
	EVENTINFO		eiEventInfo;
#ifdef _DEBUG
	fprintf(stderr, "1motorcontroll started\n");
#endif

	// ルートプロセスを取得する（失敗しません）
	hRootProcess	= GetRtThreadHandles(ROOT_PROCESS);

	// モジュール情報構造体をクリアします
	memset(&gInit, 0, sizeof(gInit));
	gInit.state		= BEFORE_INIT;

	// 低レベルティック値（マイクロ秒）を取得します
	if (!CopyRtSystemInfo(&sysinfo))
		Fail("Cannot copy system info");
	dwKtickInUsecs	= 10000 / sysinfo.KernelTickRatio;
	if (dwKtickInUsecs == 0)
		Fail("Invalid low level tick length");

	// プロセス最大プライオリティを調整します(失敗は無視)
	// TODO adjust 2番目のパラメータを数値的に小さくすることでより高いプライオリティスレッド生成を許容します
	// 注意: 割り込みスレッド生成初期化時には本値を0として設定します
	SetRtProcessMaxPriority(NULL_RTHANDLE, 150);

	// メインスレッド(この関数)のハンドルを取得します
	gInit.hMain		= GetRtThreadHandles(THIS_THREAD);
	gInit.state		= INIT_BUSY;

	// スレッドカタログを試みますが、失敗は無視します
	Catalog(NULL_RTHANDLE, gInit.hMain, "TMain");

	// ルートプロセスに本プロセスをカタログします
	if (!Catalog(hRootProcess, GetRtThreadHandles(THIS_PROCESS), "1motorcontro"))
		Fail("Cannot catalog process name");
	gInit.bCataloged = TRUE;

	// create mailbox, semaphore and message queue threads
	if (BAD_RTHANDLE == CreateRtThread(160, (LPPROC)ServerServer1, 4194304, 0))
		Fail("Cannot create thread for Server1");
	if (BAD_RTHANDLE == CreateRtThread(160, (LPPROC)ServerServer2, 4194304, 0))
		Fail("Cannot create thread for Server2");

	// 割り込みハンドラを生成します
	Interrupt1Init();

	// ポーリングスレッドを生成します
	if (BAD_RTHANDLE == CreateRtThread(170, (LPPROC)Poll1, 4194304, 0))
		Fail("Cannot create poll thread Poll1");
	if (BAD_RTHANDLE == CreateRtThread(170, (LPPROC)Poll2, 4194304, 0))
		Fail("Cannot create poll thread Poll2");

	// 初期化が完了したことを示します
	gInit.state		= INIT_DONE;
#ifdef _DEBUG
	fprintf(stderr, "1motorcontroll finished initialization\n");
#endif

	// イベントを待機します
	while (RtNotifyEvent(RT_SYSTEM_NOTIFICATIONS | RT_EXIT_NOTIFICATIONS,
		WAIT_FOREVER, &eiEventInfo))
	{
		switch(eiEventInfo.dwNotifyType)
		{
		case TERMINATE:
			// TODO: 本プロセスは終了します
			// 環境をクリーンアップします
			Cleanup();  // ここから戻ることはありません

		case NT_HOST_UP:
			// TODO: RTクライアントにおいてNTホストの復帰対応時処理
			break;

		case NT_BLUESCREEN:
			// TODO: Windowsのブルースクリーン検出対応処理
			break;

		case KERNEL_STOPPING:
			// TODO: INtimeカーネルサービス停止時対応時処理
			break;

		case NT_HOST_HIBERNATE:
			// TODO: Windowsホストの休止状態移行時対応処理
			break;

		case NT_HOST_STANDBY:
			// TODO: Windowsホストのスタンバイ状態移行時対応処理
			break;
			break;

		case NT_HOST_SHUTDOWN_PENDING:
			// TODO: Windowsホストのシャットダウン処理時対応処理
			break;
		}
	}
	Fail("Notify failed");
	return 0;
}
