// ============================================================
//  CSaveThread.h	©1996 Hiroshi Lockheimer
// ============================================================

#pragma once


class CSaveThread {
public:
						CSaveThread();
	virtual				~CSaveThread();
	
	virtual void		StartSaving(BView *view);
	virtual void		StopSaving();
	
protected:
	void				StartSaveThread(long priority = B_LOW_PRIORITY);
	void				StopSaveThread();
	
	virtual double		Save();
	
	static long			SaveThreadMain(void *data);
	
protected:
	sem_id				mSaveSem;
	thread_id			mSaveThread;
	BView*				mView;
};


