/*****************************************************************************
* FILE NAME:  Interrupt1.c - 割り込みハンドラ処理 interrupt Pci(4423, 3346)
* 
\*****************************************************************************/

#include <stdio.h>
#include <rt.h>
#include <pcibus.h>
#include <string.h>

#include "1motorcontroll.h"

char				indata;	// ハンドラで使う、6軸なら[5]の配列を設定する

// ハンドラとスレッド間で共有されるモジュールレベルのデータ変数
static WORD			wLevel;
static PCIDEV		dev;
static WORD			iobase;
static DWORD*		regs;
static MSI_PARAM_EX	msi;

/*****************************************************************************
* FUNCTION:			Interrupt1
*
* DESCRIPTION:
*	割り込みの最もクリティカルな処理です
*	本処理は適切な単純なオブジェクトへのシグナルから、
*	より複雑なI/Oポートの処理までさまざまです
* NOTE:         
*	ハンドラでは限られたINtime機能のみ利用できます
\*****************************************************************************/
__INTERRUPT void	Interrupt1(
	WORD				wCSRA,
	WORD				wLevel,
	PVOID				pv)
{
	// TODO: ローカル変数はここに宣言します

	__SHARED_INTERRUPT_PROLOG();

	// ハンドラ内では例外判定は不要です: それらを表示することもできません

	// TODO: 割り込みハンドラコンテキストにおける処理
	//変換完了を起点にする
	indata = inbyte(iobase + 0);

	//クリア処理入れないとまずいが見当たらない…

	SignalRtInterruptThread(wLevel);

	__SHARED_INTERRUPT_RETURN();
}

/*****************************************************************************
* FUNCTION:			Interrupt1Thread
*
* DESCRIPTION:
*	通常スレッドの形式で割り込みを扱います
\*****************************************************************************/
void				Interrupt1Thread(
	void*				context)
{
	// MSI_LEVEL == wLevelのときのみ、&msi渡しが必要となります
	wLevel			= SetRtInterruptHandlerEx(wLevel, 16, (LPPROC)Interrupt1, &msi);
	if (BAD_LEVEL == wLevel)
	{
		Fail("Cannot set interrupt handler for Pci(4423, 3346)");
		return;
	}

	// 割り込みが存在していることを登録
	gInit.bInterrupt1	= TRUE;

	// スレッドカタログ、ただし失敗は無視
	Catalog(NULL_RTHANDLE, GetRtThreadHandles(THIS_THREAD), "TInterrupt1");

	//メールボックス送信用
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

		// TODO: RTプロセスコンテキストにおける割り込みを処理します。
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

	// TODO: プロセスハンドルとメールボックスの名称を調節します
	// TODO: データ型メールボックスをこのプロセスが生成する場合、次の数行は削除してください
	hProcess = LookupRtHandle(hRootProcess, "DMBX_OWNER", WAIT_FOREVER);
	if (BAD_RTHANDLE == hProcess)
	{
		Fail("Cannot find data mailbox process");
		return;
	}

	// TODO: データ型メールボックスをこのプロセスが生成する場合、hProcessをNULL_RTHANDLEに置き換えます
	hDmbx = LookupRtHandle(hProcess, "DMBX_NAME", WAIT_FOREVER);
	if (BAD_RTHANDLE == hDmbx)
	{
		Fail("Cannot find data mailbox");
		return;
	}

	// 本スレッドが生存していることを登録
	gInit.htClient1 = GetRtThreadHandles(THIS_THREAD);

	// スレッドカタログ、ただし失敗は無視
	Catalog(NULL_RTHANDLE, gInit.htClient1, "TClient1");

	while (!gInit.bShutdown)
	{
		// TODO: 繰り返し行う処理をここに配置します
		// 次のRtSleepは例にすぎません
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
*  割り込みハンドラと割り込みスレッドを使用します
\*****************************************************************************/
void				Interrupt1Init(void)
{
	//DWORD	nVectors;

	// 設定されたパラメータを用いて、デバイスを検索します
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

	// BAR0内でメモリのベースアドレスを検索し、マップするサンプルコードです
	if (dev.IoSpace[0].flags & IOSPACE_MEM)
		regs = (DWORD*)MapRtPhysicalMemory(dev.IoSpace[0].start, dev.IoSpace[0].size);
	// BAR0内でメモリレジスタにアクセスするために、"regs"を使用できます

	// BAR1内でI/Oのベースアドレスを検索するサンプルコードです
	if (dev.IoSpace[1].flags & IOSPACE_IO)
		iobase = (WORD)dev.IoSpace[1].start;
	// inbyte関数やoutbyte関数を用いて、I/Oレジスタにアクセスするために、"iobase"を使用することができます。

	//3346割り込み初期設定
	inbyte(iobase + 0x0D, 0x40);		//割り込み要因ステータス、DA開始割り込み　プログラマの可能性はあるよね
	inbyte(iobase + 0x0F, 0x40);		//割り込み要因ステータス、DA開始割り込み　

	// MSIでサポートされているデバイスの場合は、必要に応じて、割込みハンドラを設定してください←サポートされてない
	/*if (0 != dev.MsiOffset)
	{
		nVectors		= PciGetMsiCount(&dev);
		wLevel			= MSI_LEVEL;
		msi.PciAddress	= MKPCIADDR((&dev));
		msi.Param		= NULL; // ここではパラメータを必要としません
		if (nVectors > 1)
		{
			msi.MsiIndex   		= 0;    // どのMSI割り込み源かを指定
			msi.MsiVectors 		= AllocateRtInterrupts(nVectors);
			msi.ReservedZero	= 0;  // 将来の互換性のため0をセットしなければならない
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

	// スレッドは、プロセスで許容されているプライオリティより、高いプライオリティを取得し、割込み処理を可能とします
	SetRtProcessMaxPriority(NULL_RTHANDLE, 0);
	//割り込みスレッド起動
	CreateRtThread(0, (LPPROC)Interrupt1Thread, 4194304, 0);
}

/*****************************************************************************
* FUNCTION:			Interrupt1Kill
*
* DESCRIPTION:
*  割り込みハンドラを終了します
\*****************************************************************************/
BOOLEAN				Interrupt1Kill(void)
{
	// 割り込みが終了していることを登録
	gInit.bInterrupt1	= FALSE;
	return ResetRtInterruptHandler(wLevel);
}
