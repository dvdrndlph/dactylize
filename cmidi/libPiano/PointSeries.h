// PointSeries.h: interface for the CPointSeries class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POINTSERIES_H__1737B7D1_6343_467F_A17C_8CD9B44B3B8E__INCLUDED_)
#define AFX_POINTSERIES_H__1737B7D1_6343_467F_A17C_8CD9B44B3B8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../PVS/pvi-utility.h"
#include "stdio.h"

#define IMAGE_WIDTH 160
#define IMAGE_HEIGHT 120

class CPointSeries  
{
public:
	CPointSeries();
	virtual ~CPointSeries();

	void AddPointSeries(CPointSeries *series);
	void AddPoint(CvPoint point);
	void AddLine(CvPoint startPoint, CvPoint endPoint);

	CvPoint points[50];
	int numPoints;

	bool mark;
};

#endif // !defined(AFX_POINTSERIES_H__1737B7D1_6343_467F_A17C_8CD9B44B3B8E__INCLUDED_)
