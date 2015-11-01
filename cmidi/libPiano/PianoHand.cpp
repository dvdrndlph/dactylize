// PianoHand.cpp: implementation of the CPianoHand class.
//
//////////////////////////////////////////////////////////////////////

#include "PianoHand.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CPianoHand() - Empty Constructor
CPianoHand::CPianoHand()
{
}

//CPianoHand - Constructor
CPianoHand::CPianoHand(int ID, int x1, int y1, int x2, int y2)
{
	//Set Bounding Box
	m_boundingBox.x = x1;
	m_boundingBox.y = y1;
	m_boundingBox.width = abs(x2 - x1);
	m_boundingBox.height = abs(y2 - y1);

	//Set Previous Values
	m_prevBoundingBox = m_boundingBox;
	m_prevCenter = m_center;

	//Set Center
	m_center.x = int(float(x1 + x2) / 2);
	m_center.y = int(float(y1 + y2) / 2);

	//Set Velocity
	velocityX = 0;
	velocityY = 0;

	//ID
	m_nHandID = ID;

	//Reset Number of Fingers
	m_nNumFingers = 0;

	//Initialize Hand Images
	m_handsImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_traceImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_edgeImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	ready = false;
	onScreen = true;

	widthOverTime = m_boundingBox.width;
}

//CPianoHand - Destructor
CPianoHand::~CPianoHand()
{
	destroy();
}

//Destroy
void CPianoHand::destroy()
{
	m_handsImage.destroy();
	m_traceImage.destroy();
	m_edgeImage.destroy();
}

//Copy All Parameters from one Hand to another
void CPianoHand::Copy(CPianoHand *hand)
{
	m_boundingBox.x = hand->m_boundingBox.x;
	m_boundingBox.y = hand->m_boundingBox.y;
	m_boundingBox.width = hand->m_boundingBox.width;
	m_boundingBox.height = hand->m_boundingBox.height;

	m_center.x = hand->m_center.x;
	m_center.y = hand->m_center.y;

	m_prevBoundingBox = m_boundingBox;
	m_prevCenter = m_center;

	velocityX = hand->velocityX;
	velocityY = hand->velocityY;

	//Initialize Hand Images
	m_handsImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_traceImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_edgeImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	ready = hand->ready;
	onScreen = hand->onScreen;
	m_nHandID = hand->m_nHandID;
	m_nNumFingers = hand->m_nNumFingers;

}

////////////////////////////////////////////////////////////////////////////
//  Hand Manipulation Functions
///////////////////////////////////////////////////////////////////////////

//Set Boundaries of Searchable Area For Hand
void CPianoHand::SetBounds(int top, int bottom)
{
	topBound = top;
	bottomBound = bottom;
}

//AddFinger - Adds Finger to Hand given (x, y)
void CPianoHand::AddFinger(int x, int y)
{
	//IF Less than 5 Fingers
	if (m_nNumFingers < 5)
	{
		//Add Finger
		m_fingers[m_nNumFingers] = *(new CPianoFinger(x, y));
		
		m_fingers[m_nNumFingers].relativeX = x - m_boundingBox.x;
		m_fingers[m_nNumFingers].relativeY = y - m_boundingBox.y;
		
		m_fingers[m_nNumFingers].sectionX = float(m_fingers[m_nNumFingers].relativeX) / float(m_boundingBox.width);
		m_fingers[m_nNumFingers].sectionY = float(m_fingers[m_nNumFingers].relativeY) / float(m_boundingBox.height);

		m_nNumFingers++;
	}
	
}

//UpdateFinger - Updates Finger at Index, with (x, y)
void CPianoHand::UpdateFinger(int index, int x, int y)
{
	m_fingers[index].x = x;
	m_fingers[index].y = y;
		
	m_fingers[index].relativeX = x - m_boundingBox.x;
	m_fingers[index].relativeY = y - m_boundingBox.y;
		
	m_fingers[index].sectionX = float(m_fingers[index].relativeX) / float(m_boundingBox.width);
	m_fingers[index].sectionY = float(m_fingers[index].relativeY) / float(m_boundingBox.height);
}

//UpdateFinger - Updates Finger using only an X Value
void CPianoHand::UpdateFinger(int index, int x)
{
	m_fingers[index].x = x;
	m_fingers[index].y = m_boundingBox.y + int(m_fingers[index].sectionY * m_boundingBox.height);
		
	m_fingers[index].relativeX = m_fingers[index].x - m_boundingBox.x;
	m_fingers[index].relativeY = m_fingers[index].y - m_boundingBox.y;
		
	m_fingers[index].sectionX = float(m_fingers[index].relativeX) / float(m_boundingBox.width);
	m_fingers[index].sectionY = float(m_fingers[index].relativeY) / float(m_boundingBox.height);
}

//UpdateFinger - Updates Finger at Index, using estimation values
void CPianoHand::UpdateFinger(int index)
{
	//Update Finger based on estimated values
	m_fingers[index].x = m_boundingBox.x + int(m_fingers[index].sectionX * m_boundingBox.width);
	m_fingers[index].y = m_boundingBox.y + int(m_fingers[index].sectionY * m_boundingBox.height);
		
	m_fingers[index].relativeX = m_fingers[index].x - m_boundingBox.x;
	m_fingers[index].relativeY = m_fingers[index].y - m_boundingBox.y;
}

//UpdateWithHand - Updates Current Hand using currHand
void CPianoHand::UpdateWithHand(CPianoHand *currHand)
{
	//Update Previous Box
	if (((m_boundingBox.x != currHand->m_boundingBox.x) && (m_boundingBox.y != currHand->m_boundingBox.y)
		&& (m_boundingBox.width != currHand->m_boundingBox.width) && (m_boundingBox.height != currHand->m_boundingBox.height))
		|| ((m_center.x != currHand->m_center.x) && (m_center.y != currHand->m_center.y)))
	{
		m_prevBoundingBox.x = m_boundingBox.x;
		m_prevBoundingBox.y = m_boundingBox.y;
		m_prevBoundingBox.width = m_boundingBox.width;
		m_prevBoundingBox.height = m_boundingBox.height;
		m_prevCenter.x = m_center.x;
		m_prevCenter.y = m_center.y;
	}

	//Calculate Velocity
	velocityX = m_center.x - currHand->m_center.x;
	velocityY = m_center.y - currHand->m_center.y;

	//Update Box and Center
	m_boundingBox.x = currHand->m_boundingBox.x;
	m_boundingBox.y = currHand->m_boundingBox.y;
	m_boundingBox.width = currHand->m_boundingBox.width;
	m_boundingBox.height = currHand->m_boundingBox.height;
	m_center.x = currHand->m_center.x;
	m_center.y = currHand->m_center.y;

}


///////////////////////////////////////////////////////
//  Hand Location Functions
//////////////////////////////////////////////////////

//Search For Hand In Immediate Vicinity
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

	//Loop Through 20x20 pixel block surrounding initial center of hand
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
			//Loop bottom row
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
			//Loop left column
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
			//Loop right column
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

	//Make sure a match is actually found
	if (highMatch > 0)
	{
		//Backup Image of Hand
		int x1, y1, x2, y2;
		//Restore Backup Image to Handimage
		cvCopy(backupImage.getIplImage(), image->getIplImage(), NULL);
		
		//Get Bounding Box of Blob
		computeBlob(&backupImage, &backupImage, m_center.x+highValX, m_center.y+highValY, 100, &x1, &y1, &x2, &y2);

		//Create new Hand by taking splitting information
		CPianoHand tempHand; 
		if (highSplitType == 0)	//Center Reference
			tempHand = *(new CPianoHand(0, x1, y1, x2, y2));
		else if (highSplitType == 1)//Top-left reference
			tempHand = *(new CPianoHand(0, x1, y1, x1+m_boundingBox.width, y2));
		else if (highSplitType == 2)	//bottom-right reference
			tempHand = *(new CPianoHand(0, x2-m_boundingBox.width, y1, x2, y2));
		else if (highSplitType == 3)	//Center reference, without much width change
			tempHand = *(new CPianoHand(0, x1, y1, x1+m_boundingBox.width, y2));
		else	//Leave Hand As Is
			tempHand = *(new CPianoHand(0, m_boundingBox.x, m_boundingBox.y, 
						m_boundingBox.x+m_boundingBox.width, m_boundingBox.y+m_boundingBox.height));

		
		//Update Current Hand, with newly created Hand
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
							//Copy Hand to HandsImage
							m_handsImage.getIplImage()->imageData[y*IMAGE_WIDTH+x] = 
								(unsigned char)image->getIplImage()->imageData[y*IMAGE_WIDTH+x];
					}
				}
			}
		}


		//Create Hand Mask Image, and Edge Image		
		onScreen = true;
		tempHand.destroy();
	}
	//IF Hand cannot be found
	else
	{
		//IF it is close to the bounds of the image
		if (m_boundingBox.x < 5 || m_boundingBox.y < (topBound+5) || 
			(m_boundingBox.x+m_boundingBox.width) > (IMAGE_WIDTH - 5) ||
			(m_boundingBox.y+m_boundingBox.height) > (bottomBound - 5))
			onScreen = false; //Remove Hand
		else
		{
			m_boundingBox.x += velocityX;
			m_boundingBox.y += velocityY;
			m_center.x += velocityX;
			m_center.y += velocityY;
		}
	}
	backupImage.destroy();
}

//CheckForHand at (x, y)
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

		//The Width is enough to be a Hand
		if ((x2 - x1) > 15)
		{
			//Create Temporary Hand Object
			CPianoHand tempHand = *(new CPianoHand(0, x1, y1, x2, y2));
			int mySplitType;
			
			//Check the Probability that it is the same as this Hand
			currMatch = Probability2(&tempHand, &mySplitType);
			*splitType = mySplitType;

			tempHand.destroy();
		}
		
		tempImage.destroy();
	}
	
	//Return Percentage Match
	return currMatch;
}

//Probability that it is the same Hand
float CPianoHand::Probability2(CPianoHand *currHand, int *splitType)
{
	double prob;
	double dist;
	double corner1Dist, corner2Dist;
	double velocityChange;
	double sizeChange;

	//Base probability on Distance difference between previous centre, 
	//and current centre
	dist = sqrt((abs(currHand->m_center.x - m_center.x) * abs(currHand->m_center.x - m_center.x))
			+ (abs(currHand->m_center.y - m_center.y) * abs(currHand->m_center.y - m_center.y)));

	//Base probability on Distance difference between previous Top Corner, 
	//and current Top Corner
	corner1Dist = sqrt((abs(currHand->m_boundingBox.x - m_boundingBox.x) * abs(currHand->m_boundingBox.x - m_boundingBox.x))
			+ (abs(currHand->m_boundingBox.y - m_boundingBox.y) * abs(currHand->m_boundingBox.y - m_boundingBox.y)));


	int absX1, absX2, absY1, absY2; //Uses width to calculate absolute position
	absX1 = currHand->m_boundingBox.x + currHand->m_boundingBox.width;
	absY1 = currHand->m_boundingBox.y + currHand->m_boundingBox.height;
	absX2 = m_boundingBox.x + m_boundingBox.width;
	absY2 = m_boundingBox.y + m_boundingBox.height;

	//Base probability on Distance difference between previous Bottom Corner, 
	//and current Bottom Corner
	corner2Dist = sqrt((abs(absX1 - absX2) * abs(absX1 - absX2))
			+ (abs(absY1 - absY2) * abs(absY1 - absY2)));


	//Calculate Velocity Change component
	velocityChange = sqrt(abs((m_center.x - currHand->m_center.x)*abs((m_center.x - currHand->m_center.x) - velocityX))
		+ (abs((m_center.y - currHand->m_center.y) - velocityY) * abs((m_center.y - currHand->m_center.y) - velocityY)));

	//Calculate Width Change Percentage component
	sizeChange = fabs(m_boundingBox.width - currHand->m_boundingBox.width)/double(m_boundingBox.width);


	//Cut Off Values
	if (velocityChange > 20)
		velocityChange = 20;
	
	if (sizeChange > 2)
		sizeChange = 2;

	//IF Size Change is unusually large (greater than 50%)
	if (sizeChange >= .6)
	{
		//IF top corner change is smaller, SPLIT
		if ((corner1Dist <= corner2Dist) && (corner1Dist < (m_boundingBox.width*0.3)))
		{
				//Based on top-left corner
			prob = (1-(corner1Dist/IMAGE_WIDTH))*0.5 + 
				//Based on Velocity Change
				(1-(velocityChange/20))*0.3 + 
				//Based on Size Change
				(1- (sizeChange/2)) * 0.2;

			*splitType = 1; //Split by using top-left corner as reference
		}
		//IF bottom corner change is smaller, SPLIT
		else if ((corner1Dist >= corner2Dist)  && (corner2Dist < (m_boundingBox.width*0.3)))
		{
				//Based on bottom-right corner
			prob = (1-(corner2Dist/IMAGE_WIDTH))*0.5 + 
				//Based on Velocity Change
				(1-(velocityChange/20))*0.3 + 
				//Based on Size Change
				(1- (sizeChange/2)) * 0.2;
			
			*splitType = 2; //Split by using bottom-right corner as reference
		}
		else
		//Else
		//IF blob is in the middle
		{
			//Based on Centre
			prob = (1-(dist/IMAGE_WIDTH))*0.5 + 
			//Based on Velocity Change
			(1-(velocityChange/20))*0.3 + 
			//Based on Size Change
			(1- (sizeChange/2)) * 0.2;
			
			*splitType = 4; //Leave Hand As Is
		}
	}
	
	//IF size change is likely but too large to change
	else if (sizeChange > 0.4)
	{
			//Based on Centre
		prob = (1-(dist/IMAGE_WIDTH))*0.5 + 
			//Based on Velocity Change
			(1-(velocityChange/20))*0.3 + 
			//Based on Size Change
			(1- (sizeChange/2)) * 0.2;
			
		*splitType = 3; //Split by using center as reference, but don't allow size change

	}
	//IF size change is likely but small
	else
	{
			//Based on Centre
		prob = (1-(dist/IMAGE_WIDTH))*0.5 + 
			//Based on Velocity Change
			(1-(velocityChange/20))*0.3 + 
			//Based on Size Change
			(1- (sizeChange/2)) * 0.2;
			
		*splitType = 0; //Split by using center as reference
	}
	
	//Return calculated probability
	//based on weighted change in Velocity Change, and Size Change, and Distance
	return prob;
}

//Segment Hand With Higher Accuracy
void CPianoHand::SegmentBetter(CIplImage *image, CIplImage *segImage)
{
	int i, j;
	unsigned char avgIntensity;
	float totalIntensity, totalPixels;

	//Calculate Average Intensity of Hand
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

	//Segment Image using Average Intensity
	unsigned char currIntensity;
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

				//Binarize as a function of colour difference and Distance
				if (val < 100)
					val = 255;
				else
					val = 0;

				//Record Decided Pixel
				segImage->getIplImage()->imageData[j*IMAGE_WIDTH+i] = val;
			}
		}
	}
}

//////////////////////////////////////////////
//  Finger Detection Functions
/////////////////////////////////////////////

void CPianoHand::DetectFingers(CIplImage *image)
{
	//Initialize Finger Mask Image
	m_fingerImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	m_fingerImage.copy(&m_handsImage);

	//Make Temporary Hand with the same characteristics of this Hand
	CPianoHand tempHand;
	tempHand.m_nNumFingers=0;
	tempHand.m_boundingBox.x = m_boundingBox.x;
	tempHand.m_boundingBox.y = m_boundingBox.y;
	tempHand.m_boundingBox.width = m_boundingBox.width;
	tempHand.m_boundingBox.height = m_boundingBox.height;
	tempHand.m_center.x = m_center.x;
	tempHand.m_center.y = m_center.y;

	//FOR each possible finger
	//**change to something more flexible**
	m_traceImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	
	//Copy All Potential Fingers
	cvSet(m_edgeImage.getIplImage(), cvScalar(0), NULL);
	MarkAllProbableFingers(image, &m_edgeImage, &m_fingerImage);

	int i;
	for (i=0; i < 5; i++)
	{
		//Initialize Individual Finger Masks
		m_fingImg[i].initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

		//Next Probable Finger is marked by two edges
		MarkNextProbableFinger(image, &m_traceImage, &m_fingerImage);
		

		//Copy Into Individual Finger Images
		m_fingImg[i].copy(&m_traceImage);
	
		//Create Finger Blob From Edges
		CreateFingerBlob(&m_traceImage);
		
		//Make Sure Finger Falls within Hand Mask
		cvAnd(m_traceImage.getIplImage(), m_fingerImage.getIplImage(), m_traceImage.getIplImage(), NULL);


		//Analyze Blob to Detect Finger
		CvPoint point = FindFingerTip(&m_traceImage, &m_fingerImage);
				
		//Add Finger if Found
		if (point.x != 0 && point.y != 0)
			tempHand.AddFinger(point.x, point.y);
	}

	//Copy Fingers To Hand
	m_nNumFingers=tempHand.m_nNumFingers;
	memcpy(m_fingers, tempHand.m_fingers, (m_nNumFingers*sizeof(CPianoFinger)));

	//Correct Finger Positions
	CorrectFingers();
}


//Update Fingers With a Temp Hand
void CPianoHand::CorrectFingers()
{
	int i;
	int fingX, fingY;

	//For Each Finger
	for (i=0; i < 5; i++)
	{
		//Calculate Probable X, Y values for Current Finger
		fingX=m_boundingBox.x+int(0.2*i*m_boundingBox.width)+int(0.1*m_boundingBox.width);
		fingY=m_boundingBox.y+m_boundingBox.height;
		
		//Find Lowest Lit Pixel, to indicate Finger Tip on current X line
		int j;
		for (j=(m_boundingBox.y+m_boundingBox.height); j >= m_boundingBox.y; j--)
		{
			if ((unsigned char)m_handsImage.getIplImage()->imageData[j*IMAGE_WIDTH+fingX] == 255)
			{
				fingY = j;
				j=0;	//Exit Loop
			}
		}

		//IF Finger is within probable value range
		if (m_fingers[i].relativeX > (0.2*i) && m_fingers[i].relativeX < (0.2*(i+1)))
		{
			//Update Finger's Y Value
			UpdateFinger(i, m_fingers[i].x, fingY);
		}
		else
		{
			//Update Finger's X and Y Values
			UpdateFinger(i, fingX, fingY);
		}
	}
}


//Mark Next Probable Finger with new Edges
void CPianoHand::MarkNextProbableFinger(CIplImage *colourImage, CIplImage *dest, CIplImage *mask)
{
	CIplImage intensityImage;
	intensityImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	intensityImage.copyBW(colourImage);

	//Mark All Relative Increases in Intensity, Horizontally
	int i, j;

	int prevIntensityTotal;
	int currIntensityTotal;

	bool down = false;
	bool up = false;
	bool upAgain = false;

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
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = (unsigned char)255;
						down = false;
						up = false;
						i = IMAGE_WIDTH; //exit loop
					}
					else if (up == false && down == false)
					{
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = (unsigned char)255;
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
				dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;

				prevIntensityTotal=0;
				down = false;
				up = false;
			}
		}
	}
	
	intensityImage.destroy();
}

void CPianoHand::MarkAllProbableFingers(CIplImage *colourImage, CIplImage *dest, CIplImage *mask)
{
	CIplImage intensityImage;
	CIplImage tempBlurImage;
	intensityImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	tempBlurImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	
	intensityImage.copyBW(colourImage);
	cvSmooth(intensityImage.getIplImage(), tempBlurImage.getIplImage(), CV_BLUR, 3, 3, 0);
	cvCopy(tempBlurImage.getIplImage(), intensityImage.getIplImage(), NULL);

	//Mark All Relative Increases in Intensity, Horizontally
	int i, j;

	int prevIntensityTotal;
	int currIntensityTotal;

	bool down = false;
	bool up = false;
	bool upAgain = false;

	int topCol, botCol;

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

				//****CHANGED JAN 8, 2006****//
				if (prevIntensityTotal == -1)			//IF just started Mask
					prevIntensityTotal = currIntensityTotal;


				else if (currIntensityTotal > (prevIntensityTotal + 3))
				{
					//****CHANGED JAN 8, 2006****//
					botCol = prevIntensityTotal;	//Set Bottom Intensity of Change
					
					
					prevIntensityTotal = currIntensityTotal;
					
					if (up == true && down == true)
					{
//						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = (unsigned char)255;
						//****CHANGED JAN 8, 2006****//
						//Show Degree of Intensity Change
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = (unsigned char)abs(topCol - botCol)*30;

						down = false;
						up = false;
					}
					else if (up == false && down == false)
					{
						//dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = (unsigned char)255;
						//****CHANGED JAN 8, 2006****//
						//Show Degree of Intensity Change
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = (unsigned char)abs(topCol - botCol) + 128;
						up = true;
					}
					else
					{
						up = true;
						dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					}
				}
				else if (currIntensityTotal < (prevIntensityTotal - 3))
				{
					down = true;
					dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
					
					//****CHANGED JAN 8, 2006****//
					topCol = prevIntensityTotal;	//Set Top Intensity of Change

					prevIntensityTotal = currIntensityTotal;
				}
			}
			else
			{
				dest->getIplImage()->imageData[j*IMAGE_WIDTH + i] = 0;
				prevIntensityTotal=-1;	//****CHANGED JAN 8, 2006****//
				down = false;
				up = false;
			}
		}
	}

	intensityImage.destroy();
}


//Create Finger Blob based on Traced Image
void CPianoHand::CreateFingerBlob(CIplImage *image)
{
	CIplImage temp;
	temp.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	int i, j;
	bool started=false;
	bool ended=false;

	bool fingerStart=false;
	bool fingerEnd=false;

	unsigned char checkPix;
	int approxWidth=-1;
	int startI;
	int endI;

	int prevStartI=-1, prevI=-1;

	int dS=0, dE=0;

	int noDataCount=0;  //Checking how many times in a row we must interpolate the values

	int top, bottom;

	//Can Be Changed to Localize Processing
	top=0;
	bottom=IMAGE_HEIGHT;
	
	//LOOP Through All Pixels in Region
	for (j=top; j < bottom; j++)
	{
		//Set Variable Values for Current Row
		started = false;
		ended = false;
		startI=0;
		endI=0;

		for (i=0; i < IMAGE_WIDTH; i++)
		{
			//Get Pixel Value
			checkPix = (unsigned char)image->getIplImage()->imageData[j*IMAGE_WIDTH+i];

			//Find Bounds of current row

			//IF First Pixel
			if (checkPix == 255 && started == false)
			{
				startI = i;
				if (prevStartI == -1)
					prevStartI = i;

				started = true;
			}
			//IF Next (Last) Pixel
			else if (checkPix == 255 && started == true && ended == false)
			{
				endI = i;
				if (prevI == -1)
					prevI = i;

				ended = true;
				i = IMAGE_WIDTH;	//EXIT LOOP
			}
		}//END ROW

		//IF both bounds have been found
		if (started==true && ended==true)
		{
			noDataCount=0;
			
			//**SHOULD BE MODIFIED TO FIND MOST LIKELY WIDTH OVER A CERTAIN AREA
			//**INSTEAD OF JUST USING THE FIRST ROW WIDTH AS THE MOST LIKELY

			//IF Approximate Width has not yet been set
			if (approxWidth == -1)
			{
				approxWidth = endI - startI;
				fingerStart=true;	//Top of Finger is found
			}
			//IF it has been set
			else
			{
				//If row values are shifted, fix it
				if (abs(prevStartI-startI) > 2)
					startI = prevStartI;
				if (abs(prevI-endI) > 2)
					endI = prevI;

				int currWidth = endI-startI;

				//IF percentage width change is not significant
				if (((fabs(approxWidth - currWidth)/float(approxWidth)) < 0.4) && currWidth <= approxWidth)
				{
					approxWidth=currWidth;

					//Mark slopes using changes from previous and current values
					dS = startI-prevStartI;
					dE = endI-prevI;
				}
				else
				{
					//Use slopes
					startI += dS;
					endI += dE;
				}
			}
		}
		//IF Bounds were not found
		else
		{
			//Use Previous bounds
			//With Previous Slope values
			startI = prevStartI+dS;
			endI = prevI+dE;
			noDataCount++;
		}

		//IF Finger can be drawn, Drawr it
		if (fingerStart==true && fingerEnd == false)
		{
			//FILL IN GAP
			int z;
			//Clear Past this point
			for (z=endI; z < IMAGE_WIDTH; z++)
				temp.getIplImage()->imageData[j*IMAGE_WIDTH+z] = 0;
			//Fill up to this point
			for (z=startI; z < endI; z++)
				temp.getIplImage()->imageData[j*IMAGE_WIDTH+z] = (unsigned char)255;
			//Clear Before Starting point
			for (z=0; z < startI; z++)
				temp.getIplImage()->imageData[j*IMAGE_WIDTH+z] = 0;

			//Determine End of Finger (by using a really big number, because it will be
			//ANDed with the Mask Image later
			if (noDataCount >= 24)
				fingerEnd=true;
		}

		//Update Prev Values
		prevStartI = startI;
		prevI = endI;

		//IF Finger Has reached its end
		if (fingerEnd == true)
			j = IMAGE_HEIGHT;  //Exit Loop
	}

	//Copy Into Actual Image
	image->copy(&temp);
	temp.destroy();
}

//Find Finger Tip of Blob (And Remove Finger From Mask)
CvPoint CPianoHand::FindFingerTip(CIplImage *trace, CIplImage *mask)
{
	int i, j;

	//Remove Current Blob from Hand Mask, so it won't be detected Again
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)trace->getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
				mask->getIplImage()->imageData[j*IMAGE_WIDTH+i] = 0;
		}
	}
	
	//Go From Bottom Right, to Top Left
	for (i=IMAGE_WIDTH-1; i >= 0; i--)
	{
		for (j=IMAGE_HEIGHT-1; j >= 0; j--)
		{
			int tipX, tipY;
			tipX=0, tipY=0;

			//IF Blob exists
			if ((unsigned char)trace->getIplImage()->imageData[j*IMAGE_WIDTH+i] == 255)
			{
				//Get Finger Tip
				FingerBlob(trace, i, j, &tipX, &tipY);

				//IF it Exists
				if ((tipY < IMAGE_HEIGHT) && (tipY != 0))
					return cvPoint(tipX, tipY);		//Return Point
			}
		}
	}

	//IF Nothing has been found, return 0, 0, as default point to indicate error
	return cvPoint(0, 0);
}

//Find Finger Tip based on computeBlob
void CPianoHand::FingerBlob(CIplImage *iplImage, int rootX, int rootY, int *tipX, int *tipY) 
{
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
	*tipX = *tipY = 0;

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
		iplImage->getIplImage()->imageData[y*w+x] = 100;

		//Find Lower Tip
		if (*tipY < y)
		{
			*tipY = y;
			*tipX = x;
		}

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

//Basic Blob Computing Function
void CPianoHand::computeBlob(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
								 int *x1, int *y1, int *x2, int *y2) 
{

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


//Recursive Function to Determine Length and Ends of Connected String of Pixels
int CPianoHand::LineSizeRecursive(IplImage *pImg, int i, int j, int *x, int *y)
{
	int curTotal, curTotal2, curTotal3;
	int x1=0, x2=0, x3=0;
	int y1=0, y2=0, y3=0;
	//int x1 = 0, y1 = 0;
	int totalLines = 0;
	unsigned char val = 0;
	unsigned tempVal = 0;

	if (j >= 0 && j < IMAGE_HEIGHT && i >= 0 && i < IMAGE_WIDTH)
		val = (unsigned char)pImg->imageData[(j*IMAGE_WIDTH)+i];
	else
		val = 0;

	if (val == (unsigned char)255)
	{
			x1 = *x;
			y1 = *y;
			x2 = *x;
			y2 = *y;
			x3 = *x;
			y3 = *y;

			curTotal = LineSizeRecursive(pImg, i-1, j-1, &x1, &y1);

			curTotal2 = LineSizeRecursive(pImg, i, j-1, &x2, &y2);

			curTotal3 = LineSizeRecursive(pImg, i+1, j-1, &x3, &y3);

		if ((curTotal >= curTotal2) && (curTotal >= curTotal3))
		{
			totalLines = curTotal;
			*x = ((x1) - 1);
			*y = ((y1) - 1);
		}
		else if ((curTotal2  >= curTotal) && (curTotal2 >= curTotal3))
		{
			totalLines = curTotal2;
			*x = x2;
			*y = ((y2) - 1);
		}
		else
		{
			totalLines = curTotal3;
			*x = ((x3) + 1);
			*y = ((y3) - 1);
		}
	}
	else
		return 0;
	
	pImg->imageData[(j*IMAGE_WIDTH)+i] = 0;
	return totalLines + 1;
}

//Finds Longest String of Pixels in Image and Returns Lowest and Highest Y Values
//within a CvPoint Variable
//NOT CURRENTLY USED
CvPoint CPianoHand::FindLongestLine(CIplImage *orig, CIplImage *output)
{
	int x, y;
	int xx, yy;
	int val;
	int topX=0, topY=0, topVal=0;
	int bottomY=0;
	for (y=IMAGE_HEIGHT-1; y >= 0; y--)
	{
		for (x=0; x < IMAGE_WIDTH; x++)
		{
			if ((unsigned char)orig->getIplImage()->imageData[y*IMAGE_WIDTH+x] == 255)
			{
				xx=0, yy=0;
				val = LineSizeRecursive(orig->getIplImage(), x, y, &xx, &yy);

				xx=0, yy=0;
				//Keep Only The Biggest Line
				if (val > topVal)
				{
					//Get Rid of Old Line
					LineSizeRecursive(output->getIplImage(), topX, topY, &xx, &yy);
					topVal = val;
					topX = x;
					topY = y;
					bottomY = yy;
				}
				//Get Rid of Line
				else
					val = LineSizeRecursive(output->getIplImage(), x, y, &xx, &yy);
			}
		}
	}
	return cvPoint(topY, (topY+bottomY));
}

//Select Closest Finger to Given Rectangle
int CPianoHand::SelectFinger(CvRect *rect)
{
	//Middle of Key Box
	int mid = (rect->x+(rect->x+rect->width))/2;
	
	//To find closest finger
	int smallDist = 1000, smallKey = -1;
	int currDist;
	
	//Sort to find closest Finger
	int i;
	for (i=0; i < m_nNumFingers; i++)
	{
		currDist = abs(m_fingers[i].x - mid);

		if (currDist < smallDist)
		{
			smallDist = currDist;
			smallKey = i;
		}
	}
	return smallKey;	//Return Selected Key
}
