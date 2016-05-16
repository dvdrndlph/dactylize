// PointSeries.cpp: implementation of the CPointSeries class.
//
//////////////////////////////////////////////////////////////////////

#include "PointSeries.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPointSeries::CPointSeries()
{
	numPoints=0;
}

CPointSeries::~CPointSeries()
{

}

void CPointSeries::AddPointSeries(CPointSeries *series)
{
	int i;
	for (i=0; i < series->numPoints; i++)
	{
		points[numPoints] = series->points[i];
		numPoints++;
	}
}

void CPointSeries::AddPoint(CvPoint point)
{
	points[numPoints] = point;
	numPoints++;
}

void CPointSeries::AddLine(CvPoint startPoint, CvPoint endPoint)
{
	int x = startPoint.x;
	int y = startPoint.y;
	int x2 = endPoint.x;
	int y2 = endPoint.y;

	int dx = abs(x - x2);
    int dy = abs(y - y2);
    double s = double(0.99/(dx>dy?dx:dy));
    double t = 0.0;
	CvPoint tempPoint;
    while(t < 1.0)
    {
        dx = int((1.0 - t)*x + t*x2);
        dy = int((1.0 - t)*y + t*y2);
        //putPix(dx, dy, col);
		if ((dx >= 0) && (dx < IMAGE_WIDTH) && (dy >= 0) && (dy < IMAGE_HEIGHT))
		{
			tempPoint.x = dx;
			tempPoint.y = dy;
			AddPoint(tempPoint);
		}
		else
			t = 1.0; //END LOOP
        t += s; 
    }
}

