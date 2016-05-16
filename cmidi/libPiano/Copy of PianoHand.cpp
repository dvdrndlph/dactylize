// PianoHand.cpp: implementation of the CPianoHand class.
//
//////////////////////////////////////////////////////////////////////

#include "PianoHand.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPianoHand::CPianoHand()
{
}

CPianoHand::CPianoHand(int ID, int x1, int y1, int x2, int y2)
{
	m_boundingBox.x = x1;
	m_boundingBox.y = y1;
	m_boundingBox.width = abs(x2 - x1);
	m_boundingBox.height = abs(y2 - y1);

	m_prevBoundingBox = m_boundingBox;
	m_prevCenter = m_center;

	m_center.x = int(float(x1 + x2) / 2);
	m_center.y = int(float(y1 + y2) / 2);

	velocityX = 0;
	velocityY = 0;

	m_nHandID = ID;

	m_nNumFingers = 0;

	m_handsImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_traceImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_edgeImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
}

CPianoHand::~CPianoHand()
{

}

void CPianoHand::AddFinger(int x, int y)
{
	if (m_nNumFingers < 5)
	{
		m_fingers[m_nNumFingers] = *(new CPianoFinger(x, y));
		
		m_fingers[m_nNumFingers].relativeX = x - m_boundingBox.x;
		m_fingers[m_nNumFingers].relativeY = y - m_boundingBox.y;
		
		m_fingers[m_nNumFingers].sectionX = float(m_fingers[m_nNumFingers].relativeX) / float(m_boundingBox.width);
		m_fingers[m_nNumFingers].sectionY = float(m_fingers[m_nNumFingers].relativeY) / float(m_boundingBox.height);

		//Start With 50% certainty
		m_fingers[m_nNumFingers].certainty = 0.5;
	
		m_nNumFingers++;
	}
	
}

void CPianoHand::UpdateFinger(int index, int x, int y)
{
	m_fingers[index].x = x;
	m_fingers[index].y = y;
		
	m_fingers[index].relativeX = x - m_boundingBox.x;
	m_fingers[index].relativeY = y - m_boundingBox.y;
		
	m_fingers[index].sectionX = float(m_fingers[index].relativeX) / float(m_boundingBox.width);
	m_fingers[index].sectionY = float(m_fingers[index].relativeY) / float(m_boundingBox.height);
}

void CPianoHand::UpdateFinger(int index)
{
	m_fingers[index].x = m_boundingBox.x + (m_fingers[index].sectionX * m_boundingBox.width);
	m_fingers[index].y = m_boundingBox.y + (m_fingers[index].sectionY * m_boundingBox.height);
		
	m_fingers[index].relativeX = m_fingers[index].x - m_boundingBox.x;
	m_fingers[index].relativeY = m_fingers[index].y - m_boundingBox.y;
}

void CPianoHand::UpdateLikelyFingers()
{
	int likelyX[5], likelyY[5];
	int i, j;
	float dist, smallestDistance;
	int smallestLoc;

	//Get Likely coordinates, based on evenly divided points along the
	//width of the hand
	for (i=0; i < 5; i++)
	{
		likelyX[i] = int(i * float(m_boundingBox.width)/5) + (m_boundingBox.x + 2);
		likelyY[i] = 50;
	}
	 
	if (m_nNumFingers < 5)
	{
		for (i=0; i < m_nNumFingers; i++)
		{
			smallestDistance= 1000;
			for (j=0; j < 5; j++)
			{
				dist = sqrt((abs(m_fingers[i].x - likelyX[j]) * abs(m_fingers[i].x - likelyX[j])));
				
				if (dist < smallestDistance)
				{
					smallestDistance = dist;
					smallestLoc = j;
				}
			}

			//Get Rid of That Value
			likelyX[smallestLoc] = 10000;
			likelyY[smallestLoc] = 10000;
			m_fingers[i].certainty = CERTAIN;
			
		}

		for (i=m_nNumFingers; i < 5; i++)
		{
			for (j=0; j < 5; j++)
			{
				if (likelyX[j] != 10000)
				{
					AddFinger(likelyX[j], likelyY[j]);
					m_fingers[i].certainty = UNCERTAIN;
					
					likelyX[j] = 10000;
					likelyY[j] = 10000;
				}
			}
		}
	}


	//Sort From Left To Right
	CPianoFinger tempFinger;
	for (i=0; i < 5; i++)
	{
		for (j=i; j < 5; j++)
		{
			if (m_fingers[j].x < m_fingers[i].x)
			{
				tempFinger = m_fingers[i];
				m_fingers[i] = m_fingers[j];
				m_fingers[j] = tempFinger;
			}
		}
	} 
}

float CPianoHand::Probability(CPianoHand *currHand)
{
	float prob;
	float dist;

	float prevDist;

	//Base probability on Distance difference between previous centre, 
	//and current centre
	dist = sqrt((abs(currHand->m_center.x - m_center.x) * abs(currHand->m_center.x - m_center.x))
			+ (abs(currHand->m_center.y - m_center.y) * abs(currHand->m_center.y - m_center.y)));

	prevDist = sqrt((abs(currHand->m_center.x - m_prevCenter.x) * abs(currHand->m_center.x - m_prevCenter.x))
			+ (abs(currHand->m_center.y - m_prevCenter.y) * abs(currHand->m_center.y - m_prevCenter.y)));

	prob = 1 - ((1)*(dist/IMAGE_WIDTH))/* + (0.3)*(prevDist/IMAGE_WIDTH))*/;

	return prob;
}
/*
void CPianoHand::UpdateWithHand(CPianoHand *currHand)
{
	int xOffset, yOffset;

	xOffset = currHand->m_boundingBox.x - m_boundingBox.x;
	yOffset = currHand->m_boundingBox.y - m_boundingBox.y;

	int i;
	for (i=0; i < 5; i++)
	{
		if (m_fingers[i].certainty < currHand->m_fingers[i].certainty)
		{
			m_fingers[i] = currHand->m_fingers[i];
		}
		else if (m_fingers[i].certainty == currHand->m_fingers[i].certainty)
		{
			if (((abs((m_fingers[i].relativeX)- currHand->m_fingers[i].relativeX)) < 5) &&
				((abs((m_fingers[i].relativeY)- currHand->m_fingers[i].relativeY)) < 5))
			{
				m_fingers[i] = currHand->m_fingers[i];
				m_fingers[i].certainty;
			}
			else
			{
				m_fingers[i].x += xOffset;
				m_fingers[i].y += yOffset;
			}

		}
		else
		{
			m_fingers[i].x += xOffset;
			m_fingers[i].y += yOffset;
		}

	}

	m_boundingBox = currHand->m_boundingBox;
	m_center = currHand->m_center;
}*/

void CPianoHand::UpdateWithHand(CPianoHand *currHand)
{
	//Update Previous Box

	if (((m_boundingBox.x != currHand->m_boundingBox.x) && (m_boundingBox.y != currHand->m_boundingBox.y)
		&& (m_boundingBox.width != currHand->m_boundingBox.width) && (m_boundingBox.height != currHand->m_boundingBox.height))
		|| ((m_center.x != currHand->m_center.x) && (m_center.y != currHand->m_center.y)))
	{
		m_prevBoundingBox = m_boundingBox;
		m_prevCenter = m_center;
	}

	//Update Box
	velocityX = m_center.x - currHand->m_center.x;
	velocityY = m_center.y - currHand->m_center.y;

	m_boundingBox = currHand->m_boundingBox;
	m_center = currHand->m_center;
	
/*
	int i, j;
	bool complete;

	for (i=0; i < currHand->m_nNumFingers; i++)
		currHand->m_fingers[i].mark = false;

	for (i=0; i < m_nNumFingers; i++)
	{
		complete = false;
		for (j=0; j < currHand->m_nNumFingers; j++)
		{
			if ((fabs(m_fingers[i].sectionX - currHand->m_fingers[j].sectionX) < 0.05)  && 
				(fabs(m_fingers[i].sectionY - currHand->m_fingers[j].sectionY) < 0.3))
			{
				if (currHand->m_fingers[j].mark == false)
				{
					UpdateFinger(i, currHand->m_fingers[j].x, currHand->m_fingers[j].y);

					//Increment certainty, each time it is found
					//Enforce correct finger
					m_fingers[i].certainty += ((1 - m_fingers[i].certainty)/1.05);

					complete = true;
				}
				currHand->m_fingers[j].mark = true;
			}
			else if ((fabs(m_fingers[i].sectionX - currHand->m_fingers[j].sectionX) < 0.1)  && 
				(fabs(m_fingers[i].sectionY - currHand->m_fingers[j].sectionY) < 0.5))
			{
				currHand->m_fingers[j].mark = true;
				//m_fingers[i].certainty += ((1 - m_fingers[i].certainty)/1.33);
				//complete = true;
			}

			//IF matched, exit loop
			if (complete == true)
				j = currHand->m_nNumFingers;
		}

		//IF wasn't matched
		if (complete == false)
		{
			//Decrease certainty, each time it is not found
			m_fingers[i].certainty *= 0.90;

			if (m_fingers[i].certainty < 0.00001)
			{
				for (j=0; j < currHand->m_nNumFingers; j++)
				{
					if (currHand->m_fingers[j].mark == false)
						m_fingers[i] = currHand->m_fingers[j];
				}
			}
			else
				UpdateFinger(i);
		}
	}

	for (i=0; i < currHand->m_nNumFingers; i++)
	{
		if (currHand->m_fingers[i].mark == false)
		{
			AddFinger(currHand->m_fingers[i].x, currHand->m_fingers[i].y);
		}
	}
	*/
}

void CPianoHand::SearchForHand(CIplImage *image)
{
	int x, y;

	float highMatch = 0;
	int highValX, highValY, highSplitType;
	float currMatch;
	int splitType;

	int i, j;

	CIplImage backupImage;
	backupImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	backupImage.copy(image);

	for (i=0; i < 10; i++)
	{
						//Top and Bottom   //Left and Right
		for (j=0; j < ((((i*2)+1)*2) + (((i*2)-1)*2)); j++)
		{
			//Loop top row
			for (x=-i; x <= i; x++)
			{
				y = -i;
				currMatch = CheckForHand(image, x, y, &splitType);
				if (currMatch > highMatch)
				{
					highMatch = currMatch;
					highValX = x;
					highValY = y;
					highSplitType = splitType;
			
				}				
			}
			for (x=-i; x <= i; x++)
			{
				y = i;
				currMatch = CheckForHand(image, x, y, &splitType);
				if (currMatch > highMatch)
				{
					highMatch = currMatch;
					highValX = x;
					highValY = y;
					highSplitType = splitType;
				}				
			}
			for (y=-i; y <= i; y++)
			{
				x = -i;
				currMatch = CheckForHand(image, x, y, &splitType);
				if (currMatch > highMatch)
				{
					highMatch = currMatch;
					highValX = x;
					highValY = y;
					highSplitType = splitType;
				}				
			}
			for (y=-i; y <= i; y++)
			{
				x = i;
				currMatch = CheckForHand(image, x, y, &splitType);
				if (currMatch > highMatch)
				{
					highMatch = currMatch;
					highValX = x;
					highValY = y;
					highSplitType = splitType;
				}				
			}
		}
	}

	if (highMatch > 0)
	{
		int x1, y1, x2, y2;
		cvCopy(backupImage.getIplImage(), image->getIplImage(), NULL);
		computeBlob(&backupImage, &backupImage, m_center.x+highValX, m_center.y+highValY, 100, &x1, &y1, &x2, &y2);

		CPianoHand tempHand; 
		
		
		if (highSplitType == 0)	//Center Reference
			tempHand = *(new CPianoHand(0, x1, y1, x2, y2));
		else if (highSplitType == 1)//Top-left reference
			tempHand = *(new CPianoHand(0, x1, y1, x1+m_boundingBox.width, y1+m_boundingBox.height));
		else if (highSplitType == 2)	//bottom-right reference
			tempHand = *(new CPianoHand(0, x2-m_boundingBox.width, y2-m_boundingBox.height, x2, y2));
		else	//Center reference, without much width change
			tempHand = *(new CPianoHand(0, x1, y1, x1+m_boundingBox.width, y2));
		UpdateWithHand(&tempHand);


		//Create Image Hands Mask Image from Bounding Box
		for (x=0; x < IMAGE_WIDTH; x++)
		{
			for (y=0; y < IMAGE_HEIGHT; y++)
			{
				m_handsImage.getIplImage()->imageData[y*IMAGE_WIDTH+x]=0;
				m_traceImage.getIplImage()->imageData[y*IMAGE_WIDTH+x]=0;

				if (x >= tempHand.m_boundingBox.x && x < (tempHand.m_boundingBox.x+tempHand.m_boundingBox.width))
				{
					if (y >= tempHand.m_boundingBox.y && y < (tempHand.m_boundingBox.y+tempHand.m_boundingBox.height))
					{
							m_handsImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] = 
								(unsigned char)image->getIplImage()->imageData[y*IMAGE_WIDTH+x];
						
					}
				}

			}
		}

		CIplImage tempImage;
		tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

		cvDilate(m_handsImage.getIplImage(), m_edgeImage.getIplImage(), NULL, 1);
		cvErode(m_edgeImage.getIplImage(), tempImage.getIplImage(), NULL, 1);
		cvCanny(tempImage.getIplImage(), m_edgeImage.getIplImage(), 0, 1, 3);

		
		/*DrawBox(m_imb_edgeDetectedImage.getIplImage(), x1, y1, x2, y2, 1);
		(*numHands)++;*/
	}
}

float CPianoHand::CheckForHand(CIplImage *image, int x, int y, int *splitType)
{
	int evalX, evalY;
	float currMatch=0;

	//Calculate x, y coordinates to evaluate
	evalX = m_center.x + x;
	evalY = m_center.y + y;

	//Make sure within bounds
	if (evalX < 0)
		evalX = 0;
	if (evalX > IMAGE_WIDTH-1)
		evalX = IMAGE_WIDTH-1;
	if (evalY < 0)
		evalY = 0;
	if (evalY > IMAGE_HEIGHT-1)
		evalY = IMAGE_HEIGHT-1;

	*splitType = 0;

	//Compute Blob if it exists
	if ((unsigned char)image->getIplImage()->imageData[evalY*IMAGE_WIDTH+evalX] == 255)
	{
		CIplImage tempImage;
		tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

		int x1, x2, y1, y2;
		computeBlob(image, &tempImage, evalX, evalY, 100, &x1, &y1, &x2, &y2);

		if ((x2 - x1) > 15)
		{
			CPianoHand tempHand = *(new CPianoHand(0, x1, y1, x2, y2));
			int mySplitType;
			currMatch = Probability2(&tempHand, &mySplitType);
			*splitType = mySplitType;
		}
	}
	
	return currMatch;
}

float CPianoHand::Probability2(CPianoHand *currHand, int *splitType)
{
	float prob;

	float dist;
	float corner1Dist, corner2Dist;
	float velocityChange;
	float sizeChange;

	//Base probability on Distance difference between previous centre, 
	//and current centre
	dist = sqrt((abs(currHand->m_center.x - m_center.x) * abs(currHand->m_center.x - m_center.x))
			+ (abs(currHand->m_center.y - m_center.y) * abs(currHand->m_center.y - m_center.y)));

	corner1Dist = sqrt((abs(currHand->m_boundingBox.x - m_boundingBox.x) * abs(currHand->m_boundingBox.x - m_boundingBox.x))
			+ (abs(currHand->m_boundingBox.y - m_center.y) * abs(currHand->m_boundingBox.y - m_boundingBox.y)));


	int absX1, absX2, absY1, absY2; //Uses width to calculate absolute position

	absX1 = currHand->m_boundingBox.x + currHand->m_boundingBox.width;
	absY1 = currHand->m_boundingBox.y + currHand->m_boundingBox.height;
	absX2 = m_boundingBox.x + m_boundingBox.width;
	absY2 = m_boundingBox.y + m_boundingBox.height;

	corner2Dist = sqrt((abs(absX1 - absX2) * abs(absX1 - absX2))
			+ (abs(absY1 - absY2) * abs(absY1 - absY2)));

	velocityChange = sqrt(abs((m_center.x - currHand->m_center.x)*abs((m_center.x - currHand->m_center.x) - velocityX))
		+ (abs((m_center.y - currHand->m_center.y) - velocityY) * abs((m_center.y - currHand->m_center.y) - velocityY)));

	//Should change to percentage of totalwidth, instead of absolute
	sizeChange = abs(m_boundingBox.width - currHand->m_boundingBox.width) + abs(m_boundingBox.height - currHand->m_boundingBox.height);


	//Cut Off Values
	if (velocityChange > 20)
		velocityChange = 20;
	if (sizeChange > 30)
		sizeChange = 30;


	if (sizeChange > 10)
	{
		if ((corner1Dist <= corner2Dist))
		{
				//Based on top-left corner
			prob = (1-(corner1Dist/IMAGE_WIDTH))*0.5 + 
				//Based on Velocity Change
				(1-(velocityChange/20))*0.3 + 
				//Based on Size Change
				(1- (sizeChange/30)) * 0.2;

			*splitType = 1; //Split by using top-left corner as reference
		}
		else	
		{
				//Based on bottom-right corner
			prob = (1-(corner2Dist/IMAGE_WIDTH))*0.5 + 
				//Based on Velocity Change
				(1-(velocityChange/20))*0.3 + 
				//Based on Size Change
				(1- (sizeChange/30)) * 0.2;
			
			*splitType = 2; //Split by using bottom-right corner as reference
		}
	}
	else if (sizeChange > 5)
	{
			//Based on Centre
		prob = (1-(dist/IMAGE_WIDTH))*0.5 + 
			//Based on Velocity Change
			(1-(velocityChange/20))*0.3 + 
			//Based on Size Change
			(1- (sizeChange/30)) * 0.2;
			
		*splitType = 3; //Split by using center as reference, but don't allow size change

	}
	else
	{
			//Based on Centre
		prob = (1-(dist/IMAGE_WIDTH))*0.5 + 
			//Based on Velocity Change
			(1-(velocityChange/20))*0.3 + 
			//Based on Size Change
			(1- (sizeChange/30)) * 0.2;
			
		*splitType = 0; //Split by using center as reference
	}
	
	return prob;
}


void CPianoHand::computeBlob(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2) {

	if(!iplImage) return;

	int w = iplImage->getWidth();
	int h = iplImage->getHeight();

	if(rootX < 0 || rootX >= w ||
		rootY < 0 || rootY >= h) return ;

	int x=0, y=0;
	int stackSize=0;
	int *stencilBuffer = new int[w*h];  // buffer used to store visited pixels
	int *stack[2];
	int floodWindowSize = 3;
	stack[0] = new int[w*h];
	stack[1] = new int[w*h];

	//reset bounds variables
	*x1 = *y1 = 500;
	*x2 = *y2 = 0;

	// Initialize the stencil buffer
	for(x=0; x<w*h; x++) {
		stencilBuffer[x] = 0;
	}

	// Initialize the stack
	for(x=0; x<w*h; x++) {
		stack[0][x] = 0;
		stack[1][x] = 0;
	}

	x = rootX;
	y = rootY;
	stencilBuffer[y*w+x] = 1;

	stack[0][stackSize] = x;
	stack[1][stackSize] = y;
	stackSize++;

	while(stackSize>0) {
		stackSize--;
		x = stack[0][stackSize];
		y = stack[1][stackSize];
		stack[0][stackSize] = 0;
		stack[1][stackSize] = 0;

		// update the blob image
		m_imgBlobs->getIplImage()->imageData[y*w+x] = (unsigned char)value;
		iplImage->getIplImage()->imageData[y*w+x] = (unsigned char)value;

		//Bounds
		if (*x1 > x)
			*x1 = x;
		if (*x2 < x)
			*x2 = x;
		if (*y1 > y)
			*y1 = y;
		if (*y2 < y)
			*y2 = y;

		for(int ny=-floodWindowSize/2; ny<=floodWindowSize/2; ny++) {
			for(int nx=-floodWindowSize/2; nx<=floodWindowSize/2; nx++) {
				if(!(nx==0 && ny==0) && x+nx >= 0 && x+nx < w && y+ny >= 0 && y+ny < h &&
					!stencilBuffer[(y+ny)*w+(x+nx)] && ((unsigned char)iplImage->getIplImage()->imageData[(y+ny)*w+(x+nx)] == (unsigned char)255)) {

					// && isSkin(iplImage, x+nx, y+ny)
					stencilBuffer[(y+ny)*w+(x+nx)] = 1;
					if(stackSize < w*h) {
						stack[0][stackSize] = x+nx;
						stack[1][stackSize] = y+ny;
						stackSize++;
					}

				}
			}
		}
	}


	delete [] stencilBuffer;
	delete [] stack[0];
	delete [] stack[1];

}

void CPianoHand::SegmentBetter(CIplImage *image, CIplImage *segImage)
{
	int i, j;
	unsigned char avgIntensity;
	float totalIntensity, float totalPixels;

	totalIntensity=0;
	totalPixels=0;
	for (i=m_boundingBox.x; i < (m_boundingBox.x+m_boundingBox.width); i++)
	{
		for (j=m_boundingBox.y; j < (m_boundingBox.y+m_boundingBox.height); j++)
		{
			if ((unsigned char)segImage->getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				totalIntensity += (unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH+i];
				totalPixels++;
			}
		}
	}

	avgIntensity = unsigned char(totalIntensity/totalPixels);

	unsigned char currIntensity;
	unsigned char currIntensity2;
	unsigned char currIntensity3;

	unsigned char val;
	for (i=m_boundingBox.x; i < (m_boundingBox.x+m_boundingBox.width); i++)
	{
		for (j=m_boundingBox.y; j < (m_boundingBox.y+m_boundingBox.height); j++)
		{
			if ((unsigned char)segImage->getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				currIntensity = (unsigned char)image->getIplImage()->imageData[(j*IMAGE_WIDTH)+(i)];			

				val = (unsigned char)abs(currIntensity-avgIntensity);
				val += (unsigned char)(2*sqrt(
						(abs(i-m_center.x)*abs(i-m_center.x))+
						(abs(j-m_center.y)*abs(j-m_center.y))
						));

				if (val < 100)
				{
					val = 255;
					//val = (unsigned char) image->getIplImage()->imageData[j*IMAGE_WIDTH+i];
				}
				else
					val = 0;

				segImage->getIplImage()->imageData[j*IMAGE_WIDTH+i] = val;
			}
		}
	}
}


//////////Fingers//////////

void CPianoHand::UpdateFingers(CIplImage *image)
{

	MarkProbableFingers(image, &m_traceImage, &m_handsImage);
	RemoveSinglePixels(m_traceImage.getIplImage());

	CPianoHand tempHand;
	tempHand.m_nNumFingers=0;
	tempHand.m_boundingBox.x = m_boundingBox.x;
	tempHand.m_boundingBox.y = m_boundingBox.y;
	tempHand.m_boundingBox.width = m_boundingBox.width;
	tempHand.m_boundingBox.height = m_boundingBox.height;
	tempHand.m_center.x = m_center.x;
	tempHand.m_center.y = m_center.y;

	int i, j;
	int x1, y1, straight, val;
	for (i=1; i < IMAGE_WIDTH-1; i++)
	{
		for (j=1; j < IMAGE_HEIGHT-1; j++)
		{
			if ((unsigned char)m_traceImage.getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				val = LineSizeRecursive2(m_traceImage.getIplImage(), i, j, &x1, &y1, 0, 1, &straight);

				int r, s;
				bool edgeMatch=false;
				for (r=(i-1); r <= (i+1); r++)
				{
					for (s=(j-1); s <= (j+1); s++)
					{
						if((unsigned char)m_edgeImage.getIplImage()->imageData[s*IMAGE_WIDTH+r] == 255)
							edgeMatch=true;
					}
				}

				edgeMatch=true;//test
				if (val > 5 && edgeMatch == true)
				{
					tempHand.AddFinger(i, j);
					tempHand.m_fingers[tempHand.m_nNumFingers-1].knuckleX = x1;
					tempHand.m_fingers[tempHand.m_nNumFingers-1].knuckleY = y1;
				}
			}
		}
	}

	//UpdateFingersWithHand(&tempHand);
	m_nNumFingers=tempHand.m_nNumFingers;
	memcpy(m_fingers, tempHand.m_fingers, (m_nNumFingers*sizeof(CPianoFinger)));
	//*m_fingers = *tempHand.m_fingers;
}

void CPianoHand::MarkProbableFingers(CIplImage *colourImage, CIplImage *dest, CIplImage *mask)
{
	CIplImage intensityImage;
	intensityImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	intensityImage.copyBW(colourImage);

//	cvSmooth(intensityImage.getIplImage(), dest->getIplImage(), CV_BLUR, 3, 3, 0);
//	cvCopy(dest->getIplImage(), intensityImage.getIplImage(), NULL);

	//Mark All Relative Increases in Intensity, Horizontally
	int i, j;

	int prevIntensityTotal;
	int currIntensityTotal;

	bool down = false;
	bool up = false;
	bool upAgain = false;

	unsigned char recordColour;

	for (j=1; j < IMAGE_HEIGHT; j++)
	{
		prevIntensityTotal=0;
		down = false;
		up = false;
		upAgain=false;
		prevIntensityTotal=255;
		for (i=1; i < IMAGE_WIDTH; i++)
		{
			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH + i] == 255)
			{
				currIntensityTotal = (unsigned char)intensityImage.getIplImage()->imageData[(j)*IMAGE_WIDTH + i];

				if (currIntensityTotal > (prevIntensityTotal + 1))
				{
					prevIntensityTotal = currIntensityTotal;
					
					if (up == true && down == true)
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
						down = false;
						up = false;
					}
					else if (up == false && down == false)
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
						up = true;
					}
					else
					{
						up = true;
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					}
				}
				else if (currIntensityTotal < (prevIntensityTotal - 1))
				{
					down = true;
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					prevIntensityTotal = currIntensityTotal;
				}
			}
			else
			{
				//CHANGED
				if (down == true && up == true)
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 255;
				else
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;

				prevIntensityTotal=0;
				down = false;
				up = false;
			}
		}
	}
}

void CPianoHand::RemoveSinglePixels(IplImage *image)
{
	int i, j;

	int y, x;

	for (i=1; i < IMAGE_WIDTH-1; i++)
	{
		for (j=1; j < IMAGE_HEIGHT-1; j++)
		{
			if (image->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				if ((image->imageData[j*IMAGE_WIDTH+i-1] == 0) && (image->imageData[j*IMAGE_WIDTH+i+1] == 0) &&
					(image->imageData[(j-1)*IMAGE_WIDTH+i-1] == 0) && (image->imageData[(j-1)*IMAGE_WIDTH+i] == 0) && (image->imageData[(j-1)*IMAGE_WIDTH+i+1] == 0) &&
					(image->imageData[(j+1)*IMAGE_WIDTH+i-1] == 0) && (image->imageData[(j+1)*IMAGE_WIDTH+i] == 0) && (image->imageData[(j+1)*IMAGE_WIDTH+i+1] == 0))
						image->imageData[j*IMAGE_WIDTH+i] = 0;
				
			}
		}
	}
}

int CPianoHand::LineSizeRecursive2(IplImage *pImg, int i, int j, int *x, int *y, int gap, int dir, int *straight)
{
	int curTotal, curTotal2, curTotal3;
	int x1=0, x2=0, x3=0;
	int y1=0, y2=0, y3=0;
	int straight1, straight2, straight3;
	
	int totalLines = 0;
	unsigned char val = 0;
	unsigned tempVal = 0;

	int r, s;

	if (j >= 0 && j < IMAGE_HEIGHT && i >= 0 && i < IMAGE_WIDTH)
		val = (unsigned char)pImg->imageData[(j*IMAGE_WIDTH)+i];
	else
		val = 0;

	if ((val == (unsigned char)255)/* || (gap < 1)*/)
	{
		x1 = *x;
		y1 = *y;
		x2 = *x;
		y2 = *y;
		x3 = *x;
		y3 = *y;

		straight1 = *straight - abs(dir - 0);
		straight2 = *straight - abs(dir - 1);
		straight3 = *straight - abs(dir - 2);
		
		if (val == (unsigned char)0)
			gap++;
		else
			gap=0;

		curTotal = LineSizeRecursive2(pImg, i-1, j-1, &x1, &y1, gap, 0, &straight1);
		curTotal2 = LineSizeRecursive2(pImg, i, j-1, &x2, &y2, gap, 1, &straight2);
		curTotal3 = LineSizeRecursive2(pImg, i+1, j-1, &x3, &y3, gap, 2, &straight3);
		
		
		if ((curTotal >= curTotal2) && (curTotal >= curTotal3))
		{
			totalLines = curTotal;
			*x = ((x1) - 1);
			*y = ((y1) - 1);
			*straight = straight1;
		}
		else if ((curTotal2  >= curTotal) && (curTotal2 >= curTotal3))
		{
			totalLines = curTotal2;
			*x = x2;
			*y = ((y2) - 1);
			*straight = straight2;
		}
		else
		{
			totalLines = curTotal3;
			*x = ((x3) + 1);
			*y = ((y3) - 1);
			*straight = straight3;
		}
	}
	else
		return 0;
	
	pImg->imageData[(j*IMAGE_WIDTH)+i] = 50;


	return totalLines + 1;

}



void CPianoHand::UpdateFingersWithHand(CPianoHand *hand)
{
	int i, j;

	for (i=0; i < hand->m_nNumFingers; i++)
	{
		hand->m_fingers[i].mark = false;
		for (j=0; j < m_nNumFingers; j++)
		{
			m_fingers[j].mark = false;
			if (fabs(hand->m_fingers[i].sectionX - m_fingers[j].sectionX) < 0.1)
			{
				hand->m_fingers[i].mark = true;
				//m_fingers[j].mark = true;
				if (fabs(hand->m_fingers[i].sectionY - m_fingers[j].sectionY) < 0.1)
				{
					UpdateFinger(j, hand->m_fingers[i].x, hand->m_fingers[i].y);
					
				}
			}
		}

		if (hand->m_fingers[i].mark == false)
			AddFinger(hand->m_fingers[i].x, hand->m_fingers[i].y);
	}

	for (j=0; j < m_nNumFingers; j++)
	{
		if (m_fingers[j].mark == false)
			UpdateFinger(j);
	}
}