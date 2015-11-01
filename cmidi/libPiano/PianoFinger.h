// PianoFinger.h: interface for the CPianoFinger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PIANOFINGER_H__B34827CA_3402_495E_B7B9_D2C59F8E8201__INCLUDED_)
#define AFX_PIANOFINGER_H__B34827CA_3402_495E_B7B9_D2C59F8E8201__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "PianoHand.h"

class CPianoFinger  
{
public:
	CPianoFinger();
	CPianoFinger(int xVal, int yVal);
	virtual ~CPianoFinger();

	int x, y; //m_x, m_nX
	int relativeX, relativeY; //m_nXrel
	float sectionX, sectionY;	//Change names m_dX_width //
	float certainty;

	int knuckleX, knuckleY;

	int width;

	int m_nID;

	bool mark;
//	CPianoHand *hand;
};

#endif // !defined(AFX_PIANOFINGER_H__B34827CA_3402_495E_B7B9_D2C59F8E8201__INCLUDED_)
