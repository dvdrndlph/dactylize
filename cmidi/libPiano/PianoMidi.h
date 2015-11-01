// PianoMidi.h: interface for the CPianoMidi class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIANOMIDI_H__9CE803AC_110B_4EA3_BDD4_F9F56AE2A655__INCLUDED_)
#define AFX_PIANOMIDI_H__9CE803AC_110B_4EA3_BDD4_F9F56AE2A655__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "portmidi.h"
#include "porttime.h"

#include "stdio.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
#define TIME_PROC ((long (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

class CPianoMidi  
{
public:
	int getAllEvents();
	void clearAllEvents();
	bool create();
	bool destroy();
	CPianoMidi();
	virtual ~CPianoMidi();

    PmStream * midi;
    PmError status, length;
    PmEvent buffer[1];

	int noteArray[20];
	int numNotes;
};

#endif // !defined(AFX_PIANOMIDI_H__9CE803AC_110B_4EA3_BDD4_F9F56AE2A655__INCLUDED_)
