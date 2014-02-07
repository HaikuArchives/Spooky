//////////////////////////////////////////////////////////////////////////
// Spooky, a blanker module, by Gertjan van Ratingen
// Loosely based on the 'Bouncing ball' sources by Hiroshi Lockheimer
//////////////////////////////////////////////////////////////////////////

#include "Spooky.h"
#include "SpookyEyes.h"


CSpookyThread	*gSpook = NULL;
int16 			prev_s;

void
module_initialize(
	void	*inSettings,
	long 	inSettingsSize)
{
#pragma unused (inSettings, inSettingsSize)
}


void
module_cleanup(
	void	**outSettings,
	long	*outSettingsSize)
{
	*outSettings = NULL;
	*outSettingsSize = 0;
}


void
module_start_saving(
	BView	*inView)
{	
	inView->Window()->Show();

	gSpook = new CSpookyThread();
	gSpook->StartSaving(inView);	
}


void
module_stop_saving()
{
	gSpook->StopSaving();
	delete (gSpook);
	gSpook = NULL;
}


void
module_start_config(
	BView *inView)
{
	if (!inView->Window()->Lock())
		return;

	BStringView *view = NULL;
	
	view = new BStringView(BRect(15.0, 10.0, 100.0, 30.0),
								 B_EMPTY_STRING,
								 "Spooky");
	inView->AddChild(view);
	view->SetViewColor(inView->ViewColor());
	view->SetFont(be_bold_font);

	view = new BStringView(BRect(15.0, 30.0, 200.0, 50.0),
								 B_EMPTY_STRING,
								 "by Gertjan van Ratingen");
	inView->AddChild(view);
	view->SetViewColor(inView->ViewColor());
	view = new BStringView(BRect(15.0, 45.0, 200.0, 65.0),
								 B_EMPTY_STRING,
								 "<gertjan@a1200.iaehv.nl>");
	inView->AddChild(view);
	view->SetViewColor(inView->ViewColor());

	inView->Window()->Unlock();
}


void
module_stop_config()
{
}

bool dac_func(void *arg, char *buf, uint32 count, void *header)
{
#pragma unused (arg, header)
	int32 s,d,db;
	int16 *short_out;

	if (gSpook->num_samples == 0) return true;

	short_out = (int16 *)buf;
	count /= 16; // from 44100 Hz,16bit,stereo to 11025 Hz, 8bit, mono
	if (count > gSpook->num_samples) count = gSpook->num_samples;
	gSpook->num_samples -= count;

	while(count-- > 0)
	{
		s = (*(gSpook->sample_ptr++)) << 6;
		d = (s + 3*prev_s)/4;
		db = *short_out + d; *short_out++ = clip(db);
		db = *short_out + d; *short_out++ = clip(db);
		d = (s + prev_s)/2;
		db = *short_out + d; *short_out++ = clip(db);
		db = *short_out + d; *short_out++ = clip(db);
		d = (3*s + prev_s)/4;
		db = *short_out + d; *short_out++ = clip(db);
		db = *short_out + d; *short_out++ = clip(db);
		d = s;
		db = *short_out + d; *short_out++ = clip(db);
		db = *short_out + d; *short_out++ = clip(db);
		prev_s = s;
	}

	return true;
}


CSpookyThread::CSpookyThread()
	: CSaveThread()
{
	uint8 color, *data, *newdata;
	int16 width, height, width2;
	BScreen bscreen;
	rgb_color rgb;

	for(int i=0;i<NUM_BITMAPS;i++)
	{
		switch(i)
		{
			case 0:
					data = eyes_6;
					rgb.red = rgb.green = rgb.blue = 0x70;
					break;
			case 1:
					data = eyes_7;
					rgb.red = rgb.green = rgb.blue = 0x90;
					break;
			case 2:
					data = eyes_8;
					rgb.red = rgb.green = rgb.blue = 0xa0;
					break;
			case 3:
					data = eyes_9;
					rgb.red = rgb.green = rgb.blue = 0xc0;
					break;
			case 4:
					data = eyes_10;
					rgb.red = rgb.green = rgb.blue = 0xe0;
					break;
			case 5:
					data = eyes_11;
					rgb.red = rgb.green = rgb.blue = 0xf0;
					break;
			case 6:
					data = eyes_12;
					rgb.red = rgb.blue = 0x40;
					rgb.green = 0xf0;
					break;
		}
	
		color = bscreen.IndexForColor(rgb);

		height = i+6;
		width = 2*height;
		width2 = (width + 3) & 0xfffc; // round to even short

		// create bitmap with data
		bitmaps[i] = new BBitmap(BRect(0,0,width-1,height-1), B_COLOR_8_BIT);
		newdata = new uint8[bitmaps[i]->BitsLength()];
		uint8 *dst = newdata;
		for(int y=0;y<height;y++)
		{
			dst = &newdata[width2*y];
			for(int x=0; x<width; x++)
			{
				if (*data++ == 1)
					*dst++ = color;
				else
					*dst++ = 0;
			}
		}
		bitmaps[i]->SetBits(newdata, width2*height, 0, B_COLOR_8_BIT);
		delete[] newdata;
	}

	num_samples = 0;
}

CSpookyThread::~CSpookyThread()
{
	for(int i=0;i<NUM_BITMAPS;i++)
		delete bitmaps[i];
}

void
CSpookyThread::StartSaving(
	BView	*view)
{
	mWindow = view->Window();
	mView = view;

	for(int i=0;i<MAX_EYES;i++)
	{
		SpookList.AddItem(new CSpook(view, bitmaps));
	}

	dacstream = new BDACStream();
	dacsubscriber= new BSubscriber("SpookySound");
	dacsubscriber->Subscribe(dacstream);
	dacsubscriber->EnterStream(NULL,FALSE, /* no neighbor, end of stream */
						this,
						dac_func,
						NULL,
						TRUE /* run as seperate thread */
						);

	CSaveThread::StartSaving(view);
}


void
CSpookyThread::StopSaving()
{
	CSaveThread::StopSaving();

	dacsubscriber->ExitStream(TRUE);
	dacsubscriber->Unsubscribe();
	delete dacsubscriber;
	delete dacstream;

	CSpook *spook = NULL;
	for ( long i = 0; 
		  (spook = (CSpook *)SpookList.ItemAt(i)) != NULL; 
		  i++)
		delete (spook);
}


double
CSpookyThread::Save()
{
	if (mWindow->Lock())
	{
		CSpook *spook = NULL;
		for(int32 i = 0; (spook = (CSpook *)SpookList.ItemAt(i)) != NULL; i++)
		{
			spook->BlinkEyes();
		}
		mView->Flush();
		mWindow->Unlock();
	}

	if (num_samples == 0)
		if ((lrand() & 0x1ff) == 0x1ff)
			PlayScarySound((int)rangerand(0,NUM_SOUNDS-1));

	return (20000.0);
}


void
CSpookyThread::PlayScarySound(int sound)
{
	switch(sound)
	{
		case 0:
			sample_ptr = (int8 *)laugh;
			num_samples = laugh_size;
			break;
		case 1:
			sample_ptr = (int8 *)loon;
			num_samples = loon_size;
			break;
		case 2:
			sample_ptr = (int8 *)owl;
			num_samples = owl_size;
			break;
		case 3:
			sample_ptr = (int8 *)wolf;
			num_samples = wolf_size;
			break;
		case 4:
			sample_ptr = (int8 *)aaaaah;
			num_samples = aaaaah_size;
			break;
	}
	prev_s = (*sample_ptr) << 8;
}



CSpook::CSpook(BView *inView, BBitmap **bm)
{
	bitmaps = bm;
	mView = inView;
	mWindow = mView->Window();
	if (mWindow->Lock())
	{
		viewRect = mView->Bounds();
		mWindow->Unlock();
	}
	count = 0;
}

void CSpook::Init()
{
	SpookSize = rangerand(MIN_EYE_SIZE,MAX_EYE_SIZE);

	bitmap = bitmaps[SpookSize-MIN_EYE_SIZE];

	// get a random position for the eyes
	SpookRect.left = rangerand(viewRect.left, viewRect.right - SpookSize*2);
	SpookRect.right = SpookRect.left + 2*SpookSize - 1;
	SpookRect.top = rangerand(viewRect.top, viewRect.bottom - SpookSize);
	SpookRect.bottom = SpookRect.top + SpookSize - 1;

	// put some values for open, stay and close counters
	open = rangerand(4,18);		// #counts to open eyes
	stay = rangerand(50,90);		// count until which eyes will stay on screen
	close = rangerand(3,12);		// #counts to close eyes
	count = 1;
}


void
CSpook::BlinkEyes()
{
	if (count == 0 && (lrand() & 0x1ff) == 0x1ff)
	{
		Init();
	}

	if (count > 0)
	{
		int y = 0;

		BRect rect = SpookRect;
		BRect bitmaprect = bitmap->Bounds();

		if (count <= open)
		{
			y = (SpookSize*(open - count)) / open;
			bitmaprect.top = y;
			rect.top += y;
			mView->DrawBitmap(bitmap, bitmaprect, rect);
		}

		if (count > stay)
		{
			y = (SpookSize*(count - stay)) / close;
			rect.bottom = rect.top + y - 1;
			mView->SetHighColor(mView->LowColor());
			mView->FillRect(rect);
		}

		count++;

		if (count >= stay + close)
		{
			// erase eyes from screen, reset count
			mView->SetHighColor(mView->LowColor());
			mView->FillRect(SpookRect);
			count = 0;
		}
	}
}




uint32 idum=0;

inline uint32 lrand()
{
	idum=1664525L*idum+1013904223L;
	
	if(idum&0x00800200)
		idum=1664525L*idum+1013904223L;
	return idum;
}

float
rangerand(float low, float high)
{
	float range;

	range = high - low + 1.0;
	range = range * (float)lrand() / (float)0x100000000;

	return ( low + range );
}

