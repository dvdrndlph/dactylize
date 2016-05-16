// PlayingHand.h: interface for the CPlayingHand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYINGHAND_H__6A9A24C0_DD66_442C_969C_4F296FA7437A__INCLUDED_)
#define AFX_PLAYINGHAND_H__6A9A24C0_DD66_442C_969C_4F296FA7437A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef struct PlayingFinger
{
/*	CvPoint ptInImage;
	float nXOnPiano;
	float nYOnPiano;
	int nOnNote;
	bool bInPlay; //  <-- not captured by midi (i.e. guessed)
	bool bPressed; // <-- know from midi
	int nFingerName;
//	RANGE_N_CONSTRAINT
*/} PlayingFinger;


class CPlayingHand  
{
public:
	CPlayingHand();
	virtual ~CPlayingHand();

	PlayingFinger m_pfinger1;
	PlayingFinger m_pfinger2;
	PlayingFinger m_pfinger3;
	PlayingFinger m_pfinger4;
	PlayingFinger m_pfinger5;

//	contrainFingerFromFinger( nFingerKnown, nFingerNotKnown, int, int, int, int);

};

#endif // !defined(AFX_PLAYINGHAND_H__6A9A24C0_DD66_442C_969C_4F296FA7437A__INCLUDED_)
