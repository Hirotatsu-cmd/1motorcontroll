/*****************************************************************************
* Poll1.c - ポーリングのためのモジュール Poll1
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>
#include <pcibus.h>
#include <stdlib.h>

#include "1motorcontroll.h"


/*****************************************************************************
* FUNCTION:		Poll1
* DESCRIPTION:
*   ポーリングスレッド Poll1
\*****************************************************************************/
void				Poll1(
	void*				param,
	WORD				wPciVendorId,
	WORD				wPciDeviceId)
{
#ifdef _DEBUG
	fprintf(stderr, "Poll1 started\n");
#endif

	// 本スレッドが生存していることを登録
	gInit.htPoll1	= GetRtThreadHandles(THIS_THREAD);

	// スレッドカタログ、ただし失敗は無視
	Catalog(NULL_RTHANDLE, gInit.htPoll1, "TPoll1");

	/*************************
	ここから追加要素
	*************************/

	PCIDEV			dev;
	//エラー回避のため入れたけど正しいかわからん
	DWORD dwBaseAddr;
	
	dev.wVendorId = 4423;
	dev.wDeviceId = 6205;
	dev.wDeviceIndex = 0; // Instance;
	if (!PciFindDevice(&dev))
		Fail("PCI device %04x/%04x(%u) not found\n", dev.wVendorId, dev.wDeviceId, dev.wDeviceIndex);

	if (dev.bUnusable)
		Fail("PCI device %04x/%04x(%u) not assigned to this node", dev.wVendorId, dev.wDeviceId, dev.wDeviceIndex);

	// 電源状態をD0状態(作業状態)に移行させ、メモリ及びI/Oへのアクセスを可能にします
	PciEnableDevice(&dev);

	// 必要に応じて、バス マスタ機能を設定します
	PciSetMaster(&dev);

	// ベースアドレスの取得
	dwBaseAddr = (WORD)dev.dwBaseAddr[0] & 0xfffc;
	
	/* 取得できなかったらこっち使う
	// BAR1内でI/Oのベースアドレスを検索するサンプルコードです
	//if (dev.IoSpace[1].flags & IOSPACE_IO)
	//	iobase = (WORD)dev.IoSpace[1].start;
	// inbyte関数やoutbyte関数を用いて、I/Oレジスタにアクセスするために、"iobase"を使用することができます。
	*/

	/***************************
	二軸からはベースアドレスを二つ以上取得して作業を進める
	****************************/


	//全体的に設定が怪しいので後で検討
	long paluse;		//パルス数の結論
	long degree;		//角度

	outbyte(dwBaseAddr + 0x04, 0x06);       // モードレジスタ設定、位相差パルス4逓倍非同期クリア、ガイドp7参照
	outbyte(dwBaseAddr + 0x06, 0x00);       // ソフトウェアラッチ

	//メールボックス設定
	//メールボックス使ってserver1に送る
	//以下clientのコピペ
	RTHANDLE		hProcess = NULL_RTHANDLE;
	RTHANDLE		hDmbx;

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
	hDmbx = LookupRtHandle(hProcess, "Server1", WAIT_FOREVER);
	if (BAD_RTHANDLE == hDmbx)
	{
		Fail("Cannot find data mailbox");
		return;
	}

	// 本スレッドが生存していることを登録
	gInit.htPoll1 = GetRtThreadHandles(THIS_THREAD);

	// スレッドカタログ、ただし失敗は無視
	Catalog(NULL_RTHANDLE, gInit.htPoll1, "TPoll1");
	
	//カウントロジック、無限ループで半恒久的なパルス導出、実行時間も出したいがここでいいのか？
	//8bitカウントだから、256パルスまでしかそのままだとカウントできない対策のコードを書く→勘違い、一つのbaseにつき24bitいける
	//

	while (!gInit.bShutdown)
	{
		outbyte(dwBaseAddr + 0x10, 0x00);       // モータ1カウンタ書き込み下位
		outbyte(dwBaseAddr + 0x11, 0x00);       // モータ1カウンタ書き込み中位
		outbyte(dwBaseAddr + 0x12, 0x00);       // モータ1カウンタ書き込み上位
		outbyte(dwBaseAddr + 0x00, 0x00);       // モータ1カウンタ読み出し下位
		outbyte(dwBaseAddr + 0x01, 0x00);       // モータ1カウンタ読み出し中位
		outbyte(dwBaseAddr + 0x02, 0x00);       // モータ1カウンタ読み出し上位
		outbyte(dwBaseAddr + 0x05, 0x15);       // ステータスレジスタ読み込み、外部信号すべてLOW

		paluse = outbyte(dwBaseAddr + 0x05, 0x15);

		degree = paluse * 0x2328 / 0x4147;		//0x4147は横移動のパルス総数、モーターの回転範囲を0～90°のギア比100倍、つまり9000°

		printf("現在のモーター1の角度は%dです\n", degree);

		RtSleep(10);

#ifdef _DEBUG
		fprintf(stderr, "Poll1 waking up\n");
#endif

		// TODO:  100 ミリ秒ごとに繰り返す処理を配置します
		//clientをコピペしてメールボックス送信
		//サイズ調整してない
		if (!SendRtData(hDmbx, degree, 5))
		{
			Fail("Cannot send to data mailbox");
			break;
		}

	}

	// 本スレッドの終了を通知
	gInit.htPoll1	= NULL_RTHANDLE;
}
