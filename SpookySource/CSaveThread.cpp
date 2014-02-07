// ============================================================
//  CSaveThread.cpp	©1996 Hiroshi Lockheimer
// ============================================================

#include "CSaveThread.h"


CSaveThread::CSaveThread()
{
	mSaveSem = -1;
	mSaveThread = -1;
	mView = NULL;
}


CSaveThread::~CSaveThread()
{
}


void
CSaveThread::StartSaving(
	BView	*view)
{
	mView = view;
	
	StartSaveThread();
}


void
CSaveThread::StopSaving()
{
	StopSaveThread();
}


void
CSaveThread::StartSaveThread(
	long	priority)
{
	mSaveSem = create_sem(1, "mSaveSem");
	mSaveThread = spawn_thread(SaveThreadMain, "mSaveThread", 
							   priority, this);
	resume_thread(mSaveThread);
}


void
CSaveThread::StopSaveThread()
{
	acquire_sem(mSaveSem);
	delete_sem(mSaveSem);
	
	long value = 0;
	wait_for_thread(mSaveThread, &value);
	
	mSaveSem = -1;
	mSaveThread = -1;
}


double
CSaveThread::Save()
{
	return (0.0);
}

	
long
CSaveThread::SaveThreadMain(
	void	*data)
{
	CSaveThread	*object = (CSaveThread *)data;
	sem_id		sem = object->mSaveSem;
	
	while (acquire_sem(sem) == B_NO_ERROR) {
		double sleep = object->Save();
		release_sem(sem);
		
		while (sleep > 0.0) {
			if (acquire_sem(sem) != B_NO_ERROR)
				sleep = 0.0;
			else {
				double interval = 50000.0;
				interval = (interval > sleep) ? sleep : interval;
				snooze(interval);
				sleep -= interval;
				release_sem(sem);
			}
		}
	}
				
	return (B_NO_ERROR);
}
