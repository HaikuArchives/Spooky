// ============================================================
//  spooky.h
//  for a blanker module, by Gertjan van Ratingen
// ============================================================

#pragma once

#pragma export on

extern "C" {
extern void	module_initialize(void *inSettings, long inSettingsSize);
extern void	module_cleanup(void **outSettings, long *outSettingsSize);
extern void	module_start_saving(BView *inView);
extern void	module_stop_saving();
extern void module_start_config(BView *inView);
extern void module_stop_config();
}

#pragma export reset

#include "CSaveThread.h"

#include "laugh.h"
#include "loon.h"
#include "owl.h"
#include "wolf.h"
#include "aaaaah.h"
#define NUM_SOUNDS 5

#define MAX_EYES 20
#define NUM_BITMAPS 7
#define MIN_EYE_SIZE 6
#define MAX_EYE_SIZE 12

#define clip(x) (x)<-32768?-32768:(x)>32767?32767:(x)

class CSpookyThread : CSaveThread {
public:
					CSpookyThread();
					~CSpookyThread();
					
	virtual void	StartSaving(BView *view);
	virtual void	StopSaving();

	uint32			num_samples;
	int8*			sample_ptr;

protected:
	virtual double	Save();
	void			PlayScarySound(int sound);

protected:
	BList			SpookList;
	BSubscriber		*dacsubscriber;
	BDACStream		*dacstream;
	BBitmap*		bitmaps[NUM_BITMAPS];
	BWindow*		mWindow;		// view's window
	BView*			mView;			// saver's view
};


class CSpook {
public:
					CSpook(BView *inView, BBitmap **bm);
	void			Init();
	void			BlinkEyes();

protected:
	int				SpookSize;	// the size of this thingy
	rgb_color		SpookColor;		// the color of the beast
	BBitmap			*bitmap, **bitmaps;
	BRect			SpookRect;		// the rectangle defining the 'eye'
	int				count;
	int 			open,stay,close;
	BView*			mView;			// saver's view
	BWindow*		mWindow;		// mView's window
	BRect			viewRect;		// mView's rectangle
};


inline uint32	lrand();
float			rangerand(float low, float high);
bool			dac_func(void *arg, char *buf, uint32 count, void *header);
