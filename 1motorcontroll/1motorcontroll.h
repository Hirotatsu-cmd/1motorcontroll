// 1motorcontroll プロジェクトのインクルードファイル

// すべてのスレッドで利用可能な関数
BOOLEAN				Catalog(RTHANDLE hProcess, RTHANDLE hObject, LPSTR lpszName);
void				Cleanup(void);
void				Fail(LPSTR lpszMessage, ...);
DWORD				UsecsToKticks(DWORD dwUsecs);

// グローバルタイプ宣言
typedef enum 		{
	BEFORE_INIT,
	INIT_BUSY,
	INIT_DONE,
	CLEANUP_BUSY 
}					INIT_STATE;

typedef struct		{
	RTHANDLE			hMain;		// メインスレッドのRTHANDLE
	INIT_STATE			state;		// メインスレッドの状態
	BOOLEAN				bCataloged;	// TRUE: プロセス名がルートにカタログされる場合TRUE
	BOOLEAN				bShutdown;	// すべてのスレッドが終了時TRUE
	// 割り込みハンドラ宣言
	BOOLEAN				bInterrupt1;	// ハンドラがアクティブの場合、TRUE
	// メールボックス、セマフォスレッド宣言
	RTHANDLE			htServer1;	// サーバスレッドのRTHANDLE
	RTHANDLE			htServer2;	// サーバスレッドのRTHANDLE
	// ポーリングスレッド宣言
	RTHANDLE			htPoll1;	// ポーリングスレッドのRTHANDLE
	RTHANDLE			htPoll2;	// ポーリングスレッドのRTHANDLE
}					INIT_STRUCT;

// グローバル変数
extern RTHANDLE		hRootProcess;	// ルートプロセスのRTHANDLE
extern DWORD		dwKtickInUsecs;	// 低レベルティックのus値
extern INIT_STRUCT	gInit;			// グローバルオブジェクトを宣言する構造体

// 割り込みハンドラ関数
void				Interrupt1Init(void);
BOOLEAN				Interrupt1Kill(void);

// メールボックス、セマフォスレッド関数
void				ServerServer1(void* param);
void				ServerServer2(void* param);

// ポーリングスレッド関数
void				Poll1(void* param);
void				Poll2(void* param);
