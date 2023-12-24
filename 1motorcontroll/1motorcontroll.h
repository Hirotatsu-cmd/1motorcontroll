// 1motorcontroll �v���W�F�N�g�̃C���N���[�h�t�@�C��

// ���ׂẴX���b�h�ŗ��p�\�Ȋ֐�
BOOLEAN				Catalog(RTHANDLE hProcess, RTHANDLE hObject, LPSTR lpszName);
void				Cleanup(void);
void				Fail(LPSTR lpszMessage, ...);
DWORD				UsecsToKticks(DWORD dwUsecs);

// �O���[�o���^�C�v�錾
typedef enum 		{
	BEFORE_INIT,
	INIT_BUSY,
	INIT_DONE,
	CLEANUP_BUSY 
}					INIT_STATE;

typedef struct		{
	RTHANDLE			hMain;		// ���C���X���b�h��RTHANDLE
	INIT_STATE			state;		// ���C���X���b�h�̏��
	BOOLEAN				bCataloged;	// TRUE: �v���Z�X�������[�g�ɃJ�^���O�����ꍇTRUE
	BOOLEAN				bShutdown;	// ���ׂẴX���b�h���I����TRUE
	// ���荞�݃n���h���錾
	BOOLEAN				bInterrupt1;	// �n���h�����A�N�e�B�u�̏ꍇ�ATRUE
	// ���[���{�b�N�X�A�Z�}�t�H�X���b�h�錾
	RTHANDLE			htServer1;	// �T�[�o�X���b�h��RTHANDLE
	RTHANDLE			htServer2;	// �T�[�o�X���b�h��RTHANDLE
	// �|�[�����O�X���b�h�錾
	RTHANDLE			htPoll1;	// �|�[�����O�X���b�h��RTHANDLE
	RTHANDLE			htPoll2;	// �|�[�����O�X���b�h��RTHANDLE
}					INIT_STRUCT;

// �O���[�o���ϐ�
extern RTHANDLE		hRootProcess;	// ���[�g�v���Z�X��RTHANDLE
extern DWORD		dwKtickInUsecs;	// �჌�x���e�B�b�N��us�l
extern INIT_STRUCT	gInit;			// �O���[�o���I�u�W�F�N�g��錾����\����

// ���荞�݃n���h���֐�
void				Interrupt1Init(void);
BOOLEAN				Interrupt1Kill(void);

// ���[���{�b�N�X�A�Z�}�t�H�X���b�h�֐�
void				ServerServer1(void* param);
void				ServerServer2(void* param);

// �|�[�����O�X���b�h�֐�
void				Poll1(void* param);
void				Poll2(void* param);
