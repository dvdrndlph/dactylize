// PianoKeyboard.cpp: implementation of the CPianoKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#include "PianoKeyboard.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPianoKeyboard::CPianoKeyboard()
{
	// DMITRY"S
	m_nKeyboardY1 = 30;
	m_nKeyboardY2 = 100;
	m_nC_X1 = 11;
	m_nC_X2 = 12;
	m_nOrigC_X1 = m_nC_X1;
	m_nOrigC_X2 = m_nC_X2;
	m_dDegreeRotation = 0;
	m_nBlackKeyLength = 5;
}

CPianoKeyboard::CPianoKeyboard(CIplImage *image)
{
	CIplImage tempImage;
	CIplImage tempImage2;
	
	tempImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	tempImage2.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	boxes.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	boxes.copyBW(image);

	bwImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	bwImage.copyBW(image);

	polarizedImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	tempImage2.copyBW(image);
	PolarizeImage(tempImage2.getIplImage(), tempImage.getIplImage(), BACKGROUND_POLARIZE_THRESHOLD);
	
	//Get Rid of little bits
	cvSmooth(tempImage.getIplImage(),  tempImage2.getIplImage(), CV_MEDIAN, 3, 3, 0);
//	cvErode(tempImage.getIplImage(), tempImage2.getIplImage(), NULL, 1);
	cvCopy(tempImage2.getIplImage(), tempImage.getIplImage(), NULL);
//	cvDilate(tempImage2.getIplImage(), tempImage.getIplImage(), NULL, 1);
	
	//Copy Polarized Image
	polarizedImage.copy(&tempImage);

	//Find Boxes
	FindShapes(&tempImage);

	//Copy Rotated Image to Background
	tempImage.copyBW(image);
	tempImage2.copyRotated(&tempImage, m_dDegreeRotation);
	
	PolarizeImage(tempImage2.getIplImage(), tempImage.getIplImage(), BACKGROUND_POLARIZE_THRESHOLD);
	AnalyzeKeys(&tempImage);

	int keyboardBottomX[IMAGE_WIDTH];	//Arrays to hold all the potential bottom locations
	int keyboardBottomY[IMAGE_WIDTH];

	//Use Black And White Rotated Background Image
	tempImage.copyBW(image);

	int i;
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		//Load Arrays with all potential Bottoms
		keyboardBottomX[i] = i;
		keyboardBottomY[i] = findTop(tempImage.getIplImage(), i, m_nKeyboardY1, BACKGROUND_KEYTRACE_THRESHOLD) - 2;
	}
	
	//Calculate Best Fitting line amongst all potential bottoms
	int x1, y1, x2, y2, elem;
	BestLine(keyboardBottomX, keyboardBottomY, IMAGE_WIDTH, 0.01, &x1, &y1, &x2, &y2, &elem);

	//Set Bottom as most likely bottom detected by BestLine
	//IF it is within the bounds of the estimated Key Length
	if (y2 > (m_nKeyboardY1 - m_nBlackKeyLength) && y2 < (m_nKeyboardY1 - int(float(m_nBlackKeyLength)/2)))
		m_nKeyboardY1 = y2;
	//OTHERWISE make predetermined estimated length
	else
		m_nKeyboardY1 = m_nKeyboardY1 - int(float(m_nBlackKeyLength)/2);

	//m_nKeyboardY1 = 0;
	m_nKeyboardY1 -= 8;

	m_nOrigC_X1 = m_nC_X1;
	m_nOrigC_X2 = m_nC_X2;

	tempImage.destroy();
	tempImage2.destroy();
}

CPianoKeyboard::~CPianoKeyboard()
{
	boxes.destroy();
	polarizedImage.destroy();
	bwImage.destroy();
}
	

//Polarizes Images by separating it into Thresholded Black and White
void CPianoKeyboard::PolarizeImage(IplImage *image, IplImage *dest, int threshold)
{
	int i, j;
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)image->imageData[j*IMAGE_WIDTH+i] > threshold)
				dest->imageData[j*IMAGE_WIDTH+i] = 0;
			else
				dest->imageData[j*IMAGE_WIDTH+i] = (unsigned char)255;
		}
	}
}

//Finds Orientation of the Keyboard based on the Location of Keys (rectangles)
//After Polarized image is given
void CPianoKeyboard::FindShapes(CIplImage *image)
{
	int i, j;								//Loop Variables
	int blobX1, blobX2, blobY1, blobY2;		//Current Blob Bounding Box

	int cornerX[20];			//Array of Blob Bounding Box Coordinates
	int cornerY[20];
	int bottomCornerX[20];
	int bottomCornerY[20];
	
	int totalCorners=0;			//Total Blobs

	//Clear Arrays
	for (i=0; i < 20; i++)
	{
		cornerX[i] = 0;
		cornerY[i] = 0;
		bottomCornerX[i] = 0;
		bottomCornerY[i] = 0;
	}

	//Image For Each Blob Isolated
	CIplImage blobImage;
	blobImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);
	blobImage.copy(image);


	//Go Through Each Pixel to determine Rectangular Blobs
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			//Reset Variables
			blobX1 = 0;
			blobX2 = 0;
			blobY1 = 0;
			blobY2 = 0;
			
		
			//IF Pixel needs to be checked (255)
			if ((unsigned char)image->getIplImage()->imageData[(j*IMAGE_WIDTH)+i] == (unsigned char)255)
			{
				//Clear Blob Image and Record Blob at current Location
				ClearImage(blobImage.getIplImage());
				computeBlob(image, &blobImage, i, j, 150, &blobX1, &blobY1, &blobX2, &blobY2);
							
				//isThereAnything Function was removed here, can be replaced
				//but it keeps crashing
				PVI_BLOB blob;
				blob.t = 0;
				
				float boxRatio;		//Ratio of length to width of blob Bounding Box
				
				//Rotation Calculation 
				//(currently set to 0 due to isThereAnything Function problems)
				int newX, newY;		
				newX = int(cos(-blob.t)*float(blobX2-blobX1) - sin(-blob.t)*float(blobY2-blobY1)) + blobX1;
				newY = int(sin(-blob.t)*float(blobX2-blobX1) + cos(-blob.t)*float(blobY2-blobY1)) + blobY1;

				//Calculate Ratio of length to width of box
				//avoid divide by 0 error
				if (blobY1 != newY)
					boxRatio = fabs(blobX1-newX) / fabs(blobY1-newY);
				else
					boxRatio = 1;
		
				//If Box Ratio is within certain bounds 
				//(resembling the dimensions of a black key)
				if (boxRatio > 0.0 && boxRatio < 0.30)
				{
					//Set Blob to Rotated variables (currently still rotated at 0)
					blobX2 = newX;
					blobY2 = newY;

					//Stay within bounds of array
					if (totalCorners < 20)
					{
						//Load Array with Blob Bounding Box
						//Parameters
						cornerX[totalCorners] = blobX2;
						cornerY[totalCorners] = blobY1;
						bottomCornerX[totalCorners] = blobX1;
						bottomCornerY[totalCorners] = blobY2;
					//	DrawLine(boxes.getIplImage(), blobX1, blobY1, blobX2, blobY2, 1);
						DrawBox(boxes.getIplImage(), cornerX[totalCorners], cornerY[totalCorners], 
									bottomCornerX[totalCorners], bottomCornerY[totalCorners], 1);
						totalCorners++;
					}//END IF in bounds of array
				}//END if Box Ratio
			}//END IF pixel is lit
		}//END FOR j
	}//END FOR i

	//Get Best matching line amongst corners of Blob bounding boxes
	//The best line is used to calculate the rotation of the keyboard
	int lineX1, lineY1, lineX2, lineY2, elem;
	BestLine(cornerX, cornerY, totalCorners, SLOPE_ERROR, &lineX1, &lineY1, &lineX2, &lineY2, &elem);
	DrawLine(boxes.getIplImage(), lineX1, lineY1, lineX2, lineY2, 1);

	//Calculate Rotation based on returned coordinates (degrees)
	m_dDegreeRotation = 0;
	if (lineX1 != lineX2)
		m_dDegreeRotation = 180*(asin(float(lineY2-lineY1) / float(lineX2-lineX1))/PI);
}

//BestLine - used to calculate line which passes through the most points
void CPianoKeyboard::BestLine(int *x, int *y, int totalPoints, float maxError, int *x1, int *y1, int *x2, int *y2, int *elem)
{
	int i, j, z;
	float slope;
	float checkSlope;
	int totalFit;
	
	int highFit=0;
		
	//For Each Starting Point
	for (i=0; i < (totalPoints-1); i++)
	{
		//For Each End Point
		for (j=(i+1); j < totalPoints; j++)
		{
			//Calculate Slope
			if (x[j] != x[i])
				slope = float(y[j] - y[i]) / float(x[j] - x[i]);
			else
				slope = 8000; // Number instead of infinite slope

			if (slope == 0)
				slope = 0.00001;

			totalFit = 0;
			//For Each Point to lie on Slope of Start and End
			for (z=0; z < totalPoints; z++)
			{
				if (x[z] != x[i])
					checkSlope = float(y[z] - y[i]) / float(x[z] - x[i]);
				else 
					checkSlope = 3000; //Number instead of infinite slope

				//Check Percentage Error
				if (fabs((slope - checkSlope)) < maxError)
					totalFit++;
			}

			//Update Highest Match
			if (totalFit > highFit)
			{
				highFit = totalFit;
				*x1 = x[i];
				*y1 = y[i];
				*x2 = x[j];
				*y2 = y[j];
				*elem = i;
			}
		}
	}

	//To Rectify Pixel level errors, and find actual best fitting line
	//by using Average

	//Create New Array with only Best Fitting points
	int newPtsX[20];
	int newPtsY[20];
	int counter=0;

	int avgX=0, avgY=0;

	//Slope
	slope = float(*y2 - *y1)/float(*x2 - *x1);
		
	//For Each Point to lie on Slope of Start and End
	for (z=0; z < totalPoints; z++)
	{
		checkSlope = float(y[z] - *y1) / float(x[z] - *x1);
		
		//Check Percentage Error
		if (fabs((slope - checkSlope)) < maxError && (counter < 20))
		{
			newPtsX[counter] = x[z];
			newPtsY[counter] = y[z];
			avgX += x[z];
			avgY += y[z];
			counter++;
		}
	}

	//Use Average Point locations to find endpoint, to find slope
	*x1 = newPtsX[0];
	*y1 = newPtsY[0];
	*x2 = int(float(avgX)/counter);
	*y2 = int(float(avgY)/counter);
}

//BestSize - Finds Highest Match of Key Size, used to define keyboard location
int CPianoKeyboard::BestSize(int *cornerY, int *bottomCornerY, int totalLines)
{
	int lengthList[20];
	int i;
	int topLength = 0;

	//Load Array with Lengths
	for (i=0; i < totalLines; i++)
	{
		if (i < 20)
			lengthList[i] = abs(cornerY[i] - bottomCornerY[i]);
	}
	
	//Make Sure we are within Array Bounds
	if (totalLines >= 20)
		totalLines = 20;

	//Find the largest match, to accomdate all keys
	for (i=0; i < totalLines; i++)
	{
		if ((lengthList[i] > topLength) && i < 20)
			topLength = lengthList[i];
	}
	
	//Return Length
	return topLength;
}

//Analyze Keys - used to find which keys are at what location
//based on pattern of keys
void CPianoKeyboard::AnalyzeKeys(CIplImage *image)
{
	boxes.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 8);

	int i, j;	//Counter Variables

	int blobX1, blobX2, blobY1, blobY2;	//Bounding Box of each Blob

	//Bounding Box Arrays
	int cornerX[20];
	int cornerY[20];
	int bottomCornerX[20];
	int bottomCornerY[20];
	
	int totalCorners=0;	//Total Blobss

	//Clear all Arrays
	for (i=0; i < 20; i++)
	{
		cornerX[i] = 0;
		cornerY[i] = 0;
		bottomCornerX[i] = 0;
		bottomCornerY[i] = 0;
	}

	//Image for each Blob
	CIplImage blobImage;
	blobImage.initialize(IMAGE_WIDTH, IMAGE_HEIGHT, 24);
	blobImage.copy(image);
	
	//FOR each pixel
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			//Reset Bounding Box Vars
			blobX1 = 0;
			blobX2 = 0;
			blobY1 = 0;
			blobY2 = 0;
			
			//IF Pixel is lit
			if ((unsigned char)image->getIplImage()->imageData[(j*IMAGE_WIDTH)+i] == (unsigned char)255)
			{
				//Clear Blob Image then show current blob
				ClearImage(blobImage.getIplImage());
				computeBlob(image, &blobImage, i, j, 150, &blobX1, &blobY1, &blobX2, &blobY2);
				
				float boxRatio;//Ratio of length to width, of box

				//Calculate box Ratio
				//avoid Divide by 0 error
				if (blobY1 != blobY2)
					boxRatio = fabs(blobX1-blobX2) / fabs(blobY1-blobY2);
				else
					boxRatio = 1;

				//If box is similar to black key dimensions
				if (boxRatio > 0.0 && boxRatio < 0.30)
				{
					//Load Array with Blob Bounding Box values
					if (totalCorners < 20)
					{
						cornerX[totalCorners] = blobX2;
						cornerY[totalCorners] = blobY1;
						bottomCornerX[totalCorners] = blobX1;
						bottomCornerY[totalCorners] = blobY2;
						totalCorners++;
					}//END IF < 20
				}//END IF boxRatio
			}//END IF pixel lit
		}//END FOR j
	}//END FOR i

	
	//Variables to keep track of updated usable points
	int newBottomY[20], newTopY[20];
		
	//Variables to keep track of which points are actually usable
	int elemList[20];
	int elemCount;
	
	//Identify Key Patterns to find the "C" key
	//also used to update usable points for Key Length purposes
	KeyPattern(cornerX, cornerY, totalCorners, 0.005, elemList, &elemCount);

	//Load Array with usable points
	for (i=0; i < elemCount; i++)
	{
		newBottomY[i] = bottomCornerY[elemList[i]];
		newTopY[i] = cornerY[elemList[i]];
	}

	//DMITRY ADDED:
	if (elemCount<=0)
	{ 
		Beep(440,300);
		return;
	}

	//Calculate Key Length based on new usable points
	int keyLength = BestSize(newTopY, newBottomY, elemCount);

	//Record Black Key Length
	m_nBlackKeyLength = keyLength;
	
	//Record top and bottom of keyboard
	m_nKeyboardY1 = newTopY[0];
	m_nKeyboardY2 = newTopY[0] + keyLength;

	//Make Sure Coordinates are within Bounds
	if ((m_nC_X1 < 0) || (m_nC_X1 > IMAGE_WIDTH))
		m_nC_X1 = 50;
	
	//Trace to find left and right bounds of current key
	//Using Single Row
	/*int tempCX = FirstColourChange(&bwImage, m_nC_X1, m_nKeyboardY1-1, -1);
	m_nC_X2 = FirstColourChange(&bwImage, m_nC_X1, m_nKeyboardY1-1, 1);
	m_nC_X1 = tempCX;*/

	//**USE MEDIAN SORTING TO FIND BEST X1 and X2
	//Instead of current Mean
	int sum1=0, sum2=0;
	for (i=0; i < 5; i++)
	{
		sum1 += FirstColourChange(&bwImage, m_nC_X1, m_nKeyboardY1-i, -1);
		sum2 += FirstColourChange(&bwImage, m_nC_X1, m_nKeyboardY1-i, 1);
	}
	m_nC_X1 = int(float(sum1)/5);
	m_nC_X2 = int(float(sum2)/5);
	
}

//KeyPattern - Matches recurring point distances into a pattern, comparing it to the actual keyboard pattern
//until a match is found, indicating the "C" key
void CPianoKeyboard::KeyPattern(int *x, int *y, int totalPoints, float maxError, int *elemList, int *elemCount)
{
	int i, j, z;		//Loop variables
	float slope;		//Slope checking variables
	float checkSlope;
	int totalFit;		//Total Points that fit the line

	int x1, x2, y1, y2, elem;//Temporary variables recording bounding box of each key
	int highFit=0;			//Counter to count how many matching points are on the line

	//For Each Starting Point
	for (i=0; i < (totalPoints-1); i++)
	{
		//For Each End Point
		for (j=(i+1); j < totalPoints; j++)
		{
			//Calculate Slope
			if (x[j] != x[i])
				slope = float(y[j] - y[i]) / float(x[j] - x[i]);
			else
				slope = 8000; // Number instead of infinite slope

			//Avoid divide by 0 errors
			if (slope == 0)
				slope = 0.00001;

			totalFit = 0;
			//For Each Point to lie on Slope of Start and End
			for (z=0; z < totalPoints; z++)
			{
				if (x[z] != x[i])
					checkSlope = float(y[z] - y[i]) / float(x[z] - x[i]);
				else 
					checkSlope = 3000; //Number instead of infinite slope

				//Check Percentage Error
				if (fabs((slope - checkSlope)) < maxError)
					totalFit++;
			}

			//Record Highest Matches
			if (totalFit > highFit)
			{
				highFit = totalFit;
				x1 = x[i];
				y1 = y[i];
				x2 = x[j];
				y2 = y[j];
				elem = i;
			}

		}//END FOR j
	}//END FOR i

	//To Rectify Pixel level errors, and find actual best fitting line
	//Create New Array with only Best Fitting points
	int newPtsX[20];
	int newPtsY[20];
	int counter=0;

	//Slope
	slope = float(y2 - y1)/float(x2 - x1);
	int keyWidth = abs(x2 - x1);
	
	//For Each Point to lie on Slope of Start and End
	for (z=0; z < totalPoints; z++)
	{
		checkSlope = float(y[z] - y1) / float(x[z] - x1);
			
		//Check Percentage Error
		//Then Load Updated Point Array
		if (fabs((slope - checkSlope)) < maxError)
		{
			newPtsX[counter] = x[z];
			newPtsY[counter] = y[z];
			elemList[counter] = z;
			counter++;
		}
	}

	//Total Number of Matching Points
	*elemCount = counter;

	//Calculate Large distance and Small distance, between keys
	//for Key Matching Pattern
	int smallDist = 1000, largeDist = 0;
	int tempDist;
	int smallLoc1=0, smallLoc2=0;
	int largeLoc1=0, largeLoc2=0;
	for (z=0; z < counter-1; z++)
	{
		tempDist = abs(newPtsX[z] - newPtsX[z+1]);
		
		if (tempDist < smallDist)
		{
			smallDist = tempDist;
			smallLoc1 = z;
			smallLoc2 = z+1;
		}
		
		if ((tempDist>largeDist) && (tempDist < (smallDist*2)))
		{
			largeDist = tempDist;
			largeLoc1 = z;
			largeLoc2 = z+1;
		}
	}

	//Load Array with Distance Pattern
	int distArray[20];
	for (z=0; z < counter-1; z++)
	{
		//0s for Short Distance, 1s for Long Distance between Keys
		if ((abs(newPtsX[z] - newPtsX[z+1])<=(smallDist+2)) && 
			(abs(newPtsX[z] - newPtsX[z+1])>=(smallDist-2)))
			distArray[z] = 0;
		else
			distArray[z] = 1;
	}

	//Go Through Pattern Array to find matching F# G# A# C# pattern
	for (z=0; z < counter-3; z++)
	{
		//IF Pattern is matched, record location of the "C" (current "E")
		if (distArray[z] == 0 && distArray[z+1] == 0
			&& distArray[z+2] == 1)
		{
			int keyWidth = int((1/3) * smallDist);
			m_nC_X1 = newPtsX[z+2] + smallDist - int(float(keyWidth)/2);
			m_nC_X2 = newPtsX[z+2] + smallDist - int(float(keyWidth)/2) + smallDist - keyWidth;
			//m_nC_X1 = newPtsX[z+2] + smallDist - int(float(keyWidth)/2);
			//m_nC_X2 = m_nC_X1 + smallDist/* - (keyWidth/2) - 1*/;
		}
	}

}

//findTop - Find Bottom of Keyboard (based on first drastic change in intensities excluding 
//black to white transition because of black and white keys)
int CPianoKeyboard::findTop(IplImage *image, int startX, int startY, int threshold)
{
	int j = startY;	//Set Start Point

	//Intensity of Starting point
	unsigned char startIntensity = (unsigned char)image->imageData[j*IMAGE_WIDTH + startX];
	unsigned char currIntensity; //Variable to hold current intensity
	
	//WHILE not at top of image
	while (j > 0)
	{
		//Get Intensity of current pixel
		currIntensity = (unsigned char)image->imageData[j*IMAGE_WIDTH + startX];
			
		//IF the intensity differences are within the threshold
		if (abs(currIntensity-startIntensity) < threshold)
		{
			//Use new intensity from now on, to allow gradual change
			startIntensity = currIntensity;
			j--;	//Move one pixel upwards
		}
		//If It Goes Black to White (in the case of black to white key change)
		else if ((currIntensity-startIntensity) > KEY_COLOUR_DIFFERENCE_THRESHOLD)
		{
			//Use new lighter intensity, as a starting intensity
			startIntensity = currIntensity;
			j--;	//Move one pixel upwards
		}
		//ELSE: intensity difference is significant, return as bottom
		else
			return j;	//Return the coordinate of bottom
	}

	return 0;	//Worst case, the top of the image is the bottom of keyboard
}

void CPianoKeyboard::ClearImage(IplImage *pImg)
{
	int x, y;

	for (x=0; x < IMAGE_WIDTH; x++)
	{
		for (y=0; y < IMAGE_HEIGHT; y++)
			pImg->imageData[(y*IMAGE_WIDTH) + x] = 0;
	}
}

void CPianoKeyboard::computeBlob(CIplImage *iplImage, CIplImage *m_imgBlobs, int rootX, int rootY, int value,
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
		else if (*x2 < x)
			*x2 = x;
		if (*y1 > y)
			*y1 = y;
		else if (*y2 < y)
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


CvRect CPianoKeyboard::ReturnKeyBounds(int key, int octave)
{
	bool white;
	int whiteKey;
	int blackKey;

	if (key==0 || key==2 || key==4 || key==5 || key==7 ||
		key==9 || key==11)
		white = true;
	else
		white = false; // dmitry

	if (key==0)
		whiteKey=0;
	else if (key==2)
		whiteKey=1;
	else if (key==4)
		whiteKey=2;
	else if (key==5)
		whiteKey=3;
	else if (key==7)
		whiteKey=4;
	else if (key==9)
		whiteKey=5;
	else if (key==11)
		whiteKey=6;

	if (key==1)
		blackKey=0;
	else if (key==3)
		blackKey=1;
	else if (key==6)
		blackKey=2;
	else if (key==8)
		blackKey=3;
	else if (key==10)
		blackKey=4;

	//Since keyboard is upside
	int width = abs(m_nC_X1-m_nC_X2);
	int octaveOffset = (6-octave)*(7*width);
	int keyOffset;
	
	CvRect bounds;
	if (white == true)
	{
		keyOffset = m_nC_X1 + ((2-whiteKey)*width);
		bounds.x = keyOffset + octaveOffset;
		bounds.width = width;
		bounds.y = m_nKeyboardY1;
		bounds.height = m_nKeyboardY2 - m_nKeyboardY1;
	}
	else
	{
		int blackWidth = (3*width)/4;
		if (blackKey == 0 || blackKey == 1)
			keyOffset = m_nC_X1 + ((2-blackKey)*width) - (width/2);
		else if (blackKey == 2 || blackKey == 3 || blackKey == 4)
			keyOffset = m_nC_X1 + ((2-blackKey)*width) - (width) - (width/2);

		bounds.x = keyOffset + octaveOffset;
		bounds.width = blackWidth;
		bounds.y = m_nKeyboardY2 - int(float(m_nKeyboardY2 - m_nKeyboardY1)/1.5);
		bounds.height = int(float(m_nKeyboardY2 - m_nKeyboardY1)/1.5);
	}



	/*int keyNum;
	int keyNote;

	int width = abs(m_nC_X1-m_nC_X2);

	bounds.x = ((5 - octave) * 7 * width) + m_nC_X1 - (key*width);
	bounds.width = width;
	
	bounds.y = m_nKeyboardY1;
	bounds.height = m_nKeyboardY2 - m_nKeyboardY1;*/
	
	return bounds;
}

char CPianoKeyboard::SelectKey(int x, int y)
{
	int keyNote;

	int width = abs(m_nC_X1-m_nC_X2);

	int keyPos;
	int CLoc;

	keyPos = int(float(x)/float(width));

	keyNote = keyPos % 7;
	CLoc = int(float(m_nC_X1)/float(width))%7;

	if (keyNote == CLoc)
		return 'E';
	else if (keyNote == ((CLoc+1)%7))
		return 'D';
	else if (keyNote == ((CLoc+2)%7))
		return 'C';
	else if (keyNote == ((CLoc+3)%7))
		return 'B';
	else if (keyNote == ((CLoc+4)%7))
		return 'A';
	else if (keyNote == ((CLoc+5)%7))
		return 'G';
	else
		return 'F';
	
}

void CPianoKeyboard::DrawBox(IplImage *pImg, int x1, int y1, int x2, int y2, int bpp)
{
	DrawLine(pImg, x1, y1, x2, y1, bpp);
	DrawLine(pImg, x2, y1, x2, y2, bpp);
	DrawLine(pImg, x2, y2, x1, y2, bpp);
	DrawLine(pImg, x1, y2, x1, y1, bpp);
}

void CPianoKeyboard::DrawLine(IplImage *pImg, int x, int y, int x2, int y2, int bpp)
{
	int dx = abs(x - x2);
    int dy = abs(y - y2);
    double s = double(0.99/(dx>dy?dx:dy));
    double t = 0.0;
    while(t < 1.0)
    {
        dx = int((1.0 - t)*x + t*x2);
        dy = int((1.0 - t)*y + t*y2);
        //putPix(dx, dy, col);
		if ((dx >= 0) && (dx < IMAGE_WIDTH) && (dy >= 0) && (dy < IMAGE_HEIGHT))
		{
			int q;
			for (q=0; q<bpp; q++)
				pImg->imageData[(dy*IMAGE_WIDTH*bpp) + (dx*bpp) + q] = (unsigned char)255;
		}
		else
			t = 1.0; //END LOOP
        t += s; 
    }
}

void CPianoKeyboard::UpdateBackground(CIplImage *image, CIplImage *bg, CIplImage *mask)
{
	int i, j;
	
	for (i=0; i < IMAGE_WIDTH; i++)
	{
		for (j=0; j < IMAGE_HEIGHT; j++)
		{
			if ((unsigned char)mask->getIplImage()->imageData[j*IMAGE_WIDTH+i] == (unsigned char)0)
			{
				bg->getIplImage()->imageData[(j*3*IMAGE_WIDTH)+(i*3)+0] = (unsigned char)image->getIplImage()->imageData[(j*3*IMAGE_WIDTH)+(i*3)+0];
				bg->getIplImage()->imageData[(j*3*IMAGE_WIDTH)+(i*3)+1] = (unsigned char)image->getIplImage()->imageData[(j*3*IMAGE_WIDTH)+(i*3)+1];
				bg->getIplImage()->imageData[(j*3*IMAGE_WIDTH)+(i*3)+2] = (unsigned char)image->getIplImage()->imageData[(j*3*IMAGE_WIDTH)+(i*3)+2];
			}
		}
	}
}

void CPianoKeyboard::UpdateOctave(int key, int octave)
{
	//int width = m_nC_X2 - m_nC_X1;
	int width = m_nOrigC_X2 - m_nOrigC_X1;
	m_nC_X1 = m_nOrigC_X1 + octave*(width*7);
	m_nC_X2 = m_nC_X1 + width;

	m_nC_X1 += (key*width);
	m_nC_X2 += (key*width);
}

int CPianoKeyboard::FirstColourChange(CIplImage *image, int x, int y, int dx)
{
	unsigned char temp;
	unsigned char prevVal;
	prevVal = (unsigned char)image->getIplImage()->imageData[y*IMAGE_WIDTH+x];
	int i;
	if (dx == -1)
	{
		for (i=x; i > 0; i--)
		{
			temp = (unsigned char)image->getIplImage()->imageData[y*IMAGE_WIDTH+i];
			
			//Make sure Key Colour Difference is significant &   It gets darker to indicate bounds of key
			/*if (abs(temp-prevVal) < KEY_COLOUR_DIFFERENCE_THRESHOLD && (temp >= prevVal))
				prevVal = temp;
			else
				return i;*/
			if (abs(temp-prevVal) > KEY_COLOUR_DIFFERENCE_THRESHOLD && (temp < prevVal))
				return i;
			else
				prevVal = temp;
		}
	}
	else
	{
		for (i=x; i < IMAGE_WIDTH; i++)
		{
			temp = (unsigned char)image->getIplImage()->imageData[y*IMAGE_WIDTH+i];
			
			if (abs(temp-prevVal) > KEY_COLOUR_DIFFERENCE_THRESHOLD && (temp < prevVal))
				return i;
			else
				prevVal = temp;
			//Make sure Key Colour Difference is significant &   It gets darker to indicate bounds of key
			/*if (abs(temp-prevVal) < KEY_COLOUR_DIFFERENCE_THRESHOLD && (temp >= prevVal))
				prevVal = temp;
			else
				return i;*/
		}
	}
	return 0;
}