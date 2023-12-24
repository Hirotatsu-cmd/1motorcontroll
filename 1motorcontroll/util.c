/*****************************************************************************
* Util.c - �v���W�F�N�g�̃��[�e�B���e�B�֐��Q
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <rt.h>

#include "1motorcontroll.h"

/*****************************************************************************
* FUNCTION:		Catalog
*
* PARAMETERS:	1. �ǂ̃v���W�F�N�g�n���h�����g�p���邩���v���W�F�N�g�n���h���Ŏw��
*				2. �J�^���O�����I�u�W�F�N�g�n���h�����w��
*				3. �J�^���O���閼��(�p���p14�����ȓ�)���w��
*
* RETURNS:		BOOLEAN �������Ӗ����܂�
*
* DESCRIPTION:	�^����ꂽ���̂����łɑ��݂��Ă���A�v���Z�X�����[�g�v���Z�X�̏ꍇ
*				�ŁA����ɂ��łɑ��݂��閼�̂��A�s���ȃI�u�W�F�N�g�������Ă���ꍇ
*				�ȏ�𖞂����Ƃ��A���݂��Ă��閼�͈̂�x�폜����܂��B
\*****************************************************************************/
BOOLEAN				Catalog(
	RTHANDLE			hProcess,
	RTHANDLE			hObject,
	LPSTR				lpszName)
{
	RTHANDLE		hOld;

	if (CatalogRtHandle(hProcess, hObject, lpszName))
		return TRUE;

	// �悭����G���[�����o���܂�
	if (((hOld = LookupRtHandle(hProcess, lpszName, NO_WAIT)) != BAD_RTHANDLE) &&
		(GetRtHandleType(hOld) == INVALID_TYPE))
	{
		// �Â��G���g�����폜���čēx���݂܂�
		if (UncatalogRtHandle(hProcess, lpszName))
			return (CatalogRtHandle(hProcess, hObject, lpszName));
	}
	return FALSE;
}

/*****************************************************************************
* FUNCTION:   Cleanup (local function)
*
* DESCRIPTION:
*  �X���b�h�ɑ΂������I�ɏI������悤�ɒʒm����;
*  �ʒm����X���b�h�����݂���ꍇ�A�폜����
*  �������ꂽ���̑��̃I�u�W�F�N�g���폜����B
\*****************************************************************************/
void				Cleanup(void)
{
	int				i;

	// �N���[���A�b�v���ł��邱�Ƃ������܂�
	gInit.state		= CLEANUP_BUSY;
	gInit.bShutdown = TRUE;

#ifdef _DEBUG
  fprintf(stderr, "1motorcontroll started cleaning up\n");
#endif

	// �X���b�h�̏I�����ő� 100 x 1�b�ҋ@���܂�
	// TODO �ҋ@���Ԃ̒����ɂ����āA�{���l 100��ύX���Ă�������
	for (i = 0; i < 100; i++)
	{
		// ���̈�A�̃X�e�[�g�����g�ɂ����āA�S������AND���e�X�g���܂�
		if (NULL_RTHANDLE == gInit.htPoll1)
		if (NULL_RTHANDLE == gInit.htPoll2)
		if (NULL_RTHANDLE == gInit.htServer1)
		if (NULL_RTHANDLE == gInit.htServer2)

			break;
		RtSleep(100);
	}

	// �|�[�����O�X���b�h���I�����܂�
	if (NULL_RTHANDLE != gInit.htPoll1)
		if (!DeleteRtThread(gInit.htPoll1))
			Fail("Cannot delete poll thread Poll1");
	if (NULL_RTHANDLE != gInit.htPoll2)
		if (!DeleteRtThread(gInit.htPoll2))
			Fail("Cannot delete poll thread Poll2");

	// ���荞�݃n���h�����I�����܂�
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

	// ���[�g�v���Z�X����J�^���O���̂��폜���܂�
	if (gInit.bCataloged)
		if (!UncatalogRtHandle(hRootProcess, "1motorcontro"))
			Fail("Cannot remove my own name");

#ifdef _DEBUG
	fprintf(stderr, "1motorcontroll finished cleaning up\n");
#endif

	// �I��
	exit(0);
}

/*****************************************************************************
* FUNCTION:     	Fail
*
* PARAMETERS:   	printf�Ɠ��l�̃p�����[�^�����҂��܂�
*
* DESCRIPTION:
*  �f�o�b�O���[�h�ɂ����ă��b�Z�[�W���v�����g���܂��B�V�������C����ǉ���
*  ���X�g�G���[�R�[�h���v�����g���܂��B���̌���s���v���Z�X�͍폜����܂�:
*  �J�����g�X���b�h�����C���X���b�h�̏ꍇ�A���ړI�ɏI������܂��B
*  �J�����g�X���b�h�����C���X���b�h�ȊO�̏ꍇ�A�I���v���𑗂�
*  �����̓R�[�����X���b�h�ɖ߂�܂�
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

	// ��O���C�����C�������p�Ƀ��^�[�����邱�Ƃ��m�F���Ă�������
	GetRtExceptionHandlerInfo(THREAD_HANDLER, &eh);
	eh.ExceptionMode = 0;
	SetRtExceptionHandler(&eh);

	// �����������J�n�O�ł���ꍇ�A�����ɏI�����܂�
	if (BEFORE_INIT == gInit.state)
		exit(0);

	if (gInit.hMain == GetRtThreadHandles(THIS_THREAD))
	{
		// ���C���X���b�h�ł���ꍇ:
		// �������������ł���ꍇ�ACleanup�����s���܂�
		if (INIT_BUSY == gInit.state)
			Cleanup();  // �ԋp���܂���

		// ���C���X���b�h:�������������s���Ă��Ȃ����߁A���^�[���Ƃ��܂�
		return;
	}

	// ���C���X���b�h�ȊO:
	// ���C���X���b�h�ɑ΂��N���[���A�b�v��v�����܂�
	// (�폜�v�����[���𑗂�)�G���[�͖������܂��B
	hDelMbx			= LookupRtHandle(NULL_RTHANDLE, "R?EXIT_MBOX", 5000);
	dwTerminate		= TERMINATE;
	SendRtData(hDelMbx, &dwTerminate, 4);
}

/*****************************************************************************
*
* FUNCTION:		UsecsToKticks
*
* PARAMETERS:	1. �ʕb���Ӗ����鐔�l
*
* RETURNS:		�჌�x���e�B�b�N�l
*
* DESCRIPTION:	�^����ꂽ������WAIT_FOREVER�̎��AWAIT_FOREVER��ԋp���܂��B
*				�����͒჌�x���`�b�N�l�Ő؂�グ���܂��B
\*****************************************************************************/
DWORD				UsecsToKticks(
	DWORD				dwUsecs)
{
	if (dwUsecs == WAIT_FOREVER)
		return WAIT_FOREVER;

	return (dwUsecs + dwKtickInUsecs - 1) / dwKtickInUsecs;
}
