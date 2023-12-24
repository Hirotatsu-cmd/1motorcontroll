/*****************************************************************************
* Poll2.c - ポーリングのためのモジュール Poll2
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>
#include <pcibus.h>
#include <stdlib.h>


#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:		Poll2
* DESCRIPTION:
*   ポーリングスレッド Poll2
\*****************************************************************************/
void				Poll2(
	void*				param)
{
#ifdef _DEBUG
	fprintf(stderr, "Poll2 started\n");
#endif

	// 本スレッドが生存していることを登録
	gInit.htPoll2	= GetRtThreadHandles(THIS_THREAD);

	// スレッドカタログ、ただし失敗は無視
	Catalog(NULL_RTHANDLE, gInit.htPoll2, "TPoll2");

	//3346a初期設定

	PCIDEV			dev;
	//多分iobase使う
	DWORD dwBaseAddr;

	dev.wVendorId = 4423;
	dev.wDeviceId = 3346;
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

	/* dwBaseaddrで取得できなかったらこっち使う
	// BAR1内でI/Oのベースアドレスを検索するサンプルコードです
	//if (dev.IoSpace[1].flags & IOSPACE_IO)
	//	iobase = (WORD)dev.IoSpace[1].start;
	// inbyte関数やoutbyte関数を用いて、I/Oレジスタにアクセスするために、"iobase"を使用することができます。
	*/

	outbyte(dwBaseAddr + 0x18, 0x01);		//アナログ出力
	outbyte(dwBaseAddr + 0x05, 0x03);		//DA変換モード、全チャンネル出力enable(6軸を考えて)
	outbyte(dwBaseAddr + 0x02, 0x00);		//DA出力チャンネル設定、チャンネル1、6軸は0x05まで追加
	outbyte(dwBaseAddr + 0x06, 0x02);		//出力レンジ設定、±5V


	//コピペ、Server1から回転角を受け取る
	//色々問題抱えてるのは間違いない
	WORD			wActual;
	BYTE			byMessage[128];
	RTHANDLE		hPoll2;

	WORD			wActual2;
	BYTE			byMessagef[128];
	RTHANDLE		hDmbx3;

#ifdef _DEBUG
	fprintf(stderr, "Thread for Poll2 started\n");
#endif
	// TODO: フラグが適切でなければ変更してください
	hPoll2 = CreateRtMailbox(DATA_MAILBOX | FIFO_QUEUING);
	if (BAD_RTHANDLE == hPoll2)
	{
		Fail("Cannot create data mailbox Poll2");
		return;
	}
	if (!Catalog(NULL_RTHANDLE, hPoll2, "Poll2"))
	{
		Fail("Cannot catalog data mailbox Poll2");
		return;
	}


	while (!gInit.bShutdown)
	{
		RtSleep(10);

#ifdef _DEBUG
		fprintf(stderr, "Poll2 waking up\n");
#endif


#ifdef _DEBUG
		fprintf(stderr, "Thread for Poll2 waiting\n");
#endif

			// TODO: タイムアウト値が適切でなければ変更してください
		wActual = ReceiveRtData(hPoll2, byMessage, WAIT_FOREVER);
		if (0 == wActual)
		{
			Fail("Receive from data mailbox Poll2 failed");
			break;
		}

		// TODO: byMessage(必要に応じてdwActual)を利用し、処理を開始できます

		//byMessageを変換

		outbyte(dwBaseAddr + 0x01, 0x00 + byMessage);			//上位出力電圧データの設定,255度までは下位ビットだけで表現できる←大嘘　ギヤ考えれば余裕で超えるので分ける必要あり
		outbyte(dwBaseAddr + 0x00, 0x00 + byMessage);		 //下位出力電圧データの設定

		//Server2から完了通知を受け取る



#ifdef _DEBUG
			fprintf(stderr, "Thread for ServMbx started\n");
#endif
			// TODO: フラグが適切でなければ変更してください
		hDmbx3 = CreateRtMailbox(DATA_MAILBOX | FIFO_QUEUING);
		if (BAD_RTHANDLE == hDmbx3)
		{
			Fail("Cannot create data mailbox Poll2");
			return;
		}
		if (!Catalog(NULL_RTHANDLE, hDmbx3, "Poll2"))
		{
			Fail("Cannot catalog data mailbox Poll2");
			return;
		}


			
#ifdef _DEBUG
			fprintf(stderr, "Thread for ServMbx waiting\n");
#endif

		// TODO: タイムアウト値が適切でなければ変更してください
		wActual2 = ReceiveRtData(hDmbx3, byMessagef, WAIT_FOREVER);
		if (0 == wActual2)
		{
			Fail("Receive from data mailbox ServMbx failed");
			break;
		}
			// TODO: byMessage(必要に応じてdwActual)を利用し、処理を開始できます
			printf("%s\n", byMessagef);
			//dmbxじゃなくね
	}

	// 本スレッドの終了を通知
	gInit.htPoll2	= NULL_RTHANDLE;

}
