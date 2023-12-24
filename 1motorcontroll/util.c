/*****************************************************************************
* Util.c - プロジェクトのユーティリティ関数群
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <rt.h>

#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:		Catalog
*
* PARAMETERS:	1. どのプロジェクトハンドルを使用するかをプロジェクトハンドルで指定
*				2. カタログされるオブジェクトハンドルを指定
*				3. カタログする名称(英半角14文字以内)を指定
*
* RETURNS:		BOOLEAN 成功を意味します
*
* DESCRIPTION:	与えられた名称がすでに存在しており、プロセスがルートプロセスの場合
*				で、さらにすでに存在する名称が、不正なオブジェクトを示している場合
*				以上を満たすとき、存在している名称は一度削除されます。
\*****************************************************************************/
BOOLEAN				Catalog(
	RTHANDLE			hProcess,
	RTHANDLE			hObject,
	LPSTR				lpszName)
{
	RTHANDLE		hOld;

	if (CatalogRtHandle(hProcess, hObject, lpszName))
		return TRUE;

	// よくあるエラーを検出します
	if (((hOld = LookupRtHandle(hProcess, lpszName, NO_WAIT)) != BAD_RTHANDLE) &&
		(GetRtHandleType(hOld) == INVALID_TYPE))
	{
		// 古いエントリを削除して再度試みます
		if (UncatalogRtHandle(hProcess, lpszName))
			return (CatalogRtHandle(hProcess, hObject, lpszName));
	}
	return FALSE;
}

/*****************************************************************************
* FUNCTION:   Cleanup (local function)
*
* DESCRIPTION:
*  スレッドに対し自発的に終了するように通知する;
*  通知後もスレッドが存在する場合、削除する
*  生成されたその他のオブジェクトを削除する。
\*****************************************************************************/
void				Cleanup(void)
{
	int				i;

	// クリーンアップ中であることを示します
	gInit.state		= CLEANUP_BUSY;
	gInit.bShutdown = TRUE;

#ifdef _DEBUG
  fprintf(stderr, "1motorcontroll started cleaning up\n");
#endif

	// スレッドの終了を最大 100 x 1秒待機します
	// TODO 待機時間の調整において、本数値 100を変更してください
	for (i = 0; i < 100; i++)
	{
		// 次の一連のステートメントにおいて、全条件のANDをテストします
		if (NULL_RTHANDLE == gInit.htPoll1)
		if (NULL_RTHANDLE == gInit.htPoll2)
		if (NULL_RTHANDLE == gInit.htServer1)
		if (NULL_RTHANDLE == gInit.htServer2)

			break;
		RtSleep(100);
	}

	// ポーリングスレッドを終了します
	if (NULL_RTHANDLE != gInit.htPoll1)
		if (!DeleteRtThread(gInit.htPoll1))
			Fail("Cannot delete poll thread Poll1");
	if (NULL_RTHANDLE != gInit.htPoll2)
		if (!DeleteRtThread(gInit.htPoll2))
			Fail("Cannot delete poll thread Poll2");

	// 割り込みハンドラを終了します
	if (gInit.bInterrupt1)
		if (!Interrupt1Kill())
			Fail("Cannot terminate interrupt Pci(4423, 3346)");
 
	// kill mailbox, semaphore and message queue threads
	if (NULL_RTHANDLE != gInit.htServer1)
		if (!DeleteRtThread(gInit.htServer1))
			Fail("Cannot delete thread for Server1");
	if (NULL_RTHANDLE != gInit.htServer2)
		if (!DeleteRtThread(gInit.htServer2))
			Fail("Cannot delete thread for Server2");

	// ルートプロセスからカタログ名称を削除します
	if (gInit.bCataloged)
		if (!UncatalogRtHandle(hRootProcess, "1motorcontro"))
			Fail("Cannot remove my own name");

#ifdef _DEBUG
	fprintf(stderr, "1motorcontroll finished cleaning up\n");
#endif

	// 終了
	exit(0);
}

/*****************************************************************************
* FUNCTION:     	Fail
*
* PARAMETERS:   	printfと同様のパラメータを期待します
*
* DESCRIPTION:
*  デバッグモードにおいてメッセージをプリントします。新しいラインを追加し
*  ラストエラーコードをプリントします。その後実行中プロセスは削除されます:
*  カレントスレッドがメインスレッドの場合、直接的に終了されます。
*  カレントスレッドがメインスレッド以外の場合、終了要求を送り
*  処理はコール元スレッドに戻ります
\*****************************************************************************/
void Fail(LPSTR lpszMessage, ...)
{
	EXCEPTION		eh;
	RTHANDLE		hDelMbx;
	DWORD			dwTerminate;

#ifdef _DEBUG
	va_list			ap;

	va_start(ap, lpszMessage);
	vfprintf(stderr, lpszMessage, ap);
	va_end(ap);
	fprintf(stderr, "\nError nr=%x %s\n", GetLastRtError(), GetRtErrorText(GetLastRtError()));
#endif

	// 例外がインライン処理用にリターンすることを確認してください
	GetRtExceptionHandlerInfo(THREAD_HANDLER, &eh);
	eh.ExceptionMode = 0;
	SetRtExceptionHandler(&eh);

	// 初期化処理開始前である場合、即座に終了します
	if (BEFORE_INIT == gInit.state)
		exit(0);

	if (gInit.hMain == GetRtThreadHandles(THIS_THREAD))
	{
		// メインスレッドである場合:
		// 初期化処理中である場合、Cleanupを実行します
		if (INIT_BUSY == gInit.state)
			Cleanup();  // 返却しません

		// メインスレッド:初期化処理を行っていないため、リターンとします
		return;
	}

	// メインスレッド以外:
	// メインスレッドに対しクリーンアップを要求します
	// (削除要求メールを送る)エラーは無視します。
	hDelMbx			= LookupRtHandle(NULL_RTHANDLE, "R?EXIT_MBOX", 5000);
	dwTerminate		= TERMINATE;
	SendRtData(hDelMbx, &dwTerminate, 4);
}

/*****************************************************************************
*
* FUNCTION:		UsecsToKticks
*
* PARAMETERS:	1. μ秒を意味する数値
*
* RETURNS:		低レベルティック値
*
* DESCRIPTION:	与えられた引数がWAIT_FOREVERの時、WAIT_FOREVERを返却します。
*				引数は低レベルチック値で切り上げられます。
\*****************************************************************************/
DWORD				UsecsToKticks(
	DWORD				dwUsecs)
{
	if (dwUsecs == WAIT_FOREVER)
		return WAIT_FOREVER;

	return (dwUsecs + dwKtickInUsecs - 1) / dwKtickInUsecs;
}
