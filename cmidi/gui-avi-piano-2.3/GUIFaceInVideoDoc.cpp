// GUIFaceInVideoDoc.cpp : implementation of the CGUIFaceInVideoDoc class
//

#include "stdafx.h"
#include "GUIFaceInVideo.h"

#include "GUIFaceInVideoDoc.h"
#include "MainFrm.h"
#include "DeviceDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoDoc

IMPLEMENT_DYNCREATE(CGUIFaceInVideoDoc, CDocument)

BEGIN_MESSAGE_MAP(CGUIFaceInVideoDoc, CDocument)
	//{{AFX_MSG_MAP(CGUIFaceInVideoDoc)
	ON_COMMAND(ID_CONFIG_SELECT_CAMERA, OnConfigSelectCamera)
	ON_COMMAND(ID_CONFIG_SELECT_VIDEOFILE, OnConfigSelectVideofile)
	ON_COMMAND(ID_VIDEOFILE_PLAY, OnVideofilePlay)
	ON_UPDATE_COMMAND_UI(ID_VIDEOFILE_PLAY, OnUpdateVideofilePlay)
	ON_COMMAND(ID_VIDEOFILE_REWIND_FRAME, OnVideofileRewindFrame)
	ON_UPDATE_COMMAND_UI(ID_VIDEOFILE_REWIND_FRAME, OnUpdateVideofileRewindFrame)
	ON_COMMAND(ID_VIDEOFILE_REWIND_START, OnVideofileRewindStart)
	ON_UPDATE_COMMAND_UI(ID_VIDEOFILE_REWIND_START, OnUpdateVideofileRewindStart)
	ON_COMMAND(ID_VIDEOFILE_FORWARD_FRAME, OnVideofileForwardFrame)
	ON_UPDATE_COMMAND_UI(ID_VIDEOFILE_FORWARD_FRAME, OnUpdateVideofileForwardFrame)
	ON_COMMAND(ID_VIDEOFILE_FORWARD_END, OnVideofileForwardEnd)
	ON_UPDATE_COMMAND_UI(ID_VIDEOFILE_FORWARD_END, OnUpdateVideofileForwardEnd)
	ON_COMMAND(ID_VIDEOFILE_STOP, OnVideofileStop)
	ON_UPDATE_COMMAND_UI(ID_VIDEOFILE_STOP, OnUpdateVideofileStop)
	ON_COMMAND(ID_CONFIG_CAMERA_PROPERTIES, OnConfigCameraProperties)
	ON_COMMAND(ID_CONFIG_FILE1, OnConfigFile1)
	ON_COMMAND(ID_CONFIG_FILE2, OnConfigFile2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoDoc construction/destruction


CGUIFaceInVideoDoc::CGUIFaceInVideoDoc()
{
	m_recordCheck = false;

	m_fScenario = NULL;
	m_fScenarioTest = NULL;
	m_fLog = NULL;

	for(int i=0;i<MAX_STACK;i++)
		{
			strcpy(sStackY[i],""); 
			strcpy(sStackS[i],""); 
		}
	nLastFrameIDs=0;

	m_bVideoFileOpen = false;
	m_bVideoFilePaused = true;
	m_gui.check1 = 0;
	m_gui.check2 = 0; // read from file first
	m_gui.check3 = 0;

	m_gui.check_start = 0;
	m_gui.check_nonstop = 0;

	m_gui.slider1 = 7;
	m_gui.slider2 = 10;
	m_gui.slider3 = 80;
	m_gui.slider4 = 0;
	m_gui.check_motion = 0;
	m_gui.check_colour = 0;
	m_gui.check_intensity = 1;

//	m_fFeaturesR = fopen("_FR_featuresR.txt", "w");
//	m_fFeaturesB = fopen("_FR_featuresB.txt", "w");


#define MAX_FACE_WIDTH 200
#define MAX_FACE_HEIGHT 200

	m_fLog = NULL;
// Where do I use it ??
	m_arruFace = NULL;
	m_arruFace = (unsigned char *) malloc (MAX_FACE_WIDTH*MAX_FACE_HEIGHT*sizeof(unsigned char));

}


CGUIFaceInVideoDoc::~CGUIFaceInVideoDoc()
{
//	fclose(m_fFeaturesR);
//	fclose(m_fFeaturesB);

	if (m_fLog != NULL)
		fclose(m_fLog);


	if (m_arruFace != NULL)
		free (m_arruFace);

	m_imgIn.destroy();

}

int CGUIFaceInVideoDoc::openFaceList()
{
	char  str[800];
	m_fScenario = NULL;
	m_fScenario = fopen(m_strFaceList, "r");

	if (m_fScenario == NULL)
	{
//MessageBox("Lighting seems bad.\n Shall we try self-adjustment?",
//				NULL,MB_YESNO | MB_ICONWARNING) == IDYES)

		sprintf(str, "File with list of faces '%s' \nnot found! Continue?", m_strFaceList);
		if (AfxMessageBox(str,	MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			exit (0) ;
		else
			return -1;
	}
			

	fgets(str, 800, m_fScenario);  // title

	fgets(str, 800, m_fScenario);  // path 
	sscanf(str, "%s", m_strImagePath);

	sprintf(str, "The list of faces '%s' is opened.\nFaces from %s will be used", 
		m_strFaceList, m_strImagePath);
	AfxMessageBox(str,	MB_OK );

	fgets(str, 800, m_fScenario);  // comment

	m_fScenarioTest = NULL;
	m_fScenarioTest = fopen(m_strFaceList, "r");
//	m_fScenarioTest = fopen("face_data-test.txt", "r");

	return 1;
}

void CGUIFaceInVideoDoc::closeFaceList()
{
	if (m_fScenario != NULL)
		fclose(m_fScenario);
	if (m_fScenarioTest != NULL)
		fclose(m_fScenarioTest);
}

BOOL CGUIFaceInVideoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();


/*
	CSliderCtrl *pSlider = (CSliderCtrl*)pMainFrame->m_wndSettingBar.GetDlgItem(IDC_SLIDER1);
	m_gui.slider1 = (pSlider ? pSlider->GetPos() : 0);
	pSlider = (CSliderCtrl*)pMainFrame->m_wndSettingBar.GetDlgItem(IDC_SLIDER2);
	m_gui.slider1 = (pSlider ? pSlider->GetPos() : 0);
	pSlider = (CSliderCtrl*)pMainFrame->m_wndSettingBar.GetDlgItem(IDC_SLIDER3);
	m_gui.slider1 = (pSlider ? pSlider->GetPos() : 0);
	pSlider = (CSliderCtrl*)pMainFrame->m_wndSettingBar.GetDlgItem(IDC_SLIDER4);
	m_gui.slider1 = (pSlider ? pSlider->GetPos() : 0);
*/

/*	CButton *pCheck1 = (CButton*)pMainFrame->m_wndSettingBar.GetDlgItem(IDC_CHECK1);
	m_gui.check1 = (pCheck1 ? (pCheck1->GetState() & 0x0003) : 0);
	CButton *pCheck2 = (CButton*)pMainFrame->m_wndSettingBar.GetDlgItem(IDC_CHECK2);
	m_gui.check2 = (pCheck2 ? (pCheck2->GetState() & 0x0003) : 0);
*/

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoDoc serialization

void CGUIFaceInVideoDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoDoc diagnostics

#ifdef _DEBUG
void CGUIFaceInVideoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGUIFaceInVideoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGUIFaceInVideoDoc commands

void CGUIFaceInVideoDoc::OnConfigSelectCamera() 
{
	CDeviceDialog deviceDialog;

	if(deviceDialog.DoModal() == IDOK)
	{
		Sleep(1000);
		CVideoCameraEntry *pEntry = deviceDialog.getSelectedEntry();		
		if(pEntry != NULL)
		{
			// Get the camera view
			POSITION pos = GetFirstViewPosition(); // ?? 
			CView* pView = GetNextView(pos);  // ??

			m_videoCameraEntry = *pEntry;

			// Initialize the camera device
			if(!m_PVSystem.configureCamera(pView->m_hWnd, deviceDialog.getWidth(), deviceDialog.getHeight(), deviceDialog.getBpp(), pEntry))
				::AfxMessageBox("Cannot initialize camera. Check camera installation or DirectX version.");

			pView->InvalidateRect(0);

			m_bVideoFileOpen = false;
		}
		Sleep(1000);
	}	
}
void CGUIFaceInVideoDoc::OnConfigCameraProperties() 
{
	if(m_PVSystem.getVideoCamera())
		m_PVSystem.getVideoCamera()->showPropertiesDialog("Camera Configuration");
}

#if 1
void CGUIFaceInVideoDoc::OnConfigSelectVideofile() 
{
	CFileDialog fileDialog(TRUE, ".avi", NULL, OFN_OVERWRITEPROMPT, "AVI Video Files (*.avi)|*.avi||", NULL);
	if(fileDialog.DoModal() == IDOK)
	{
		// destroy the camera if it is in use
		m_PVSystem.destroyCamera();

		// load the video file
		if(m_PVSystem.openVideoFile(fileDialog.GetPathName().GetBuffer(0))) {
			m_bVideoFileOpen = true;
			m_bVideoFilePaused = true;
		}
	}
}
#else

void CGUIFaceInVideoDoc::OnConfigSelectVideofile() 
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");
	//	strcpy(strPath, IMAGE_DIRECTORY);
	
    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), 0, NULL,
		TEXT("All Files\0*.*\0AVI Files\0*.avi\0Mpg Files\0*.mpg\0\0"), NULL,
		0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
		TEXT("Open Image File"),
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
		TEXT(".gif"), 0, NULL, NULL };
	
	
	// Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        return;
    }
	
	//	strcpy(m_strFaceList, strFileName);	
	
	// destroy the camera if it is in use
	m_PVSystem.destroyCamera();
	
	// load the video file
	if(m_PVSystem.openVideoFile(strFileName)) {
		m_bVideoFileOpen = true;
		m_bVideoFilePaused = true;
	}
}
#endif

#define IMAGE_DIRECTORY "C:\\cpp\\face-data"

void CGUIFaceInVideoDoc::OnConfigFile1() 
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");
	strcpy(strPath, IMAGE_DIRECTORY);

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), 0, NULL,
                         TEXT("All Files\0*.*\0Jpg Files\0*.jpg\0Gif Files\0*.gif\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Image File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".gif"), 0, NULL, NULL };

    // Get the default media path (something like C:\WINDOWS\MEDIA)
/*   if( '\0' == strPath[0] )
    {
 //       GetWindowsDirectory( strPath, MAX_PATH );
        if( strcmp( &strPath[strlen(strPath)], TEXT("\\") ) )
            strcat( strPath, TEXT("\\") );
        strcat( strPath, TEXT("MEDIA") );
    }
*/

   // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        return;
    }

	strcpy(m_input.strFile1, strFileName);
	m_bWorkWithFiles = true;
}

void CGUIFaceInVideoDoc::OnConfigFile2() 
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");
//	strcpy(strPath, IMAGE_DIRECTORY);

    // Setup the OPENFILENAME structure
    OPENFILENAME ofn = { sizeof(OPENFILENAME), 0, NULL,
                         TEXT("Txt Files\0*.txt\0Scn Files\0*.scn\0All Files\0*.*\0\0"), NULL,
                         0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
                         TEXT("Open Scenario File"),
                         OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
                         TEXT(".gif"), 0, NULL, NULL };

    // Get the default media path (something like C:\WINDOWS\MEDIA)
/*   if( '\0' == strPath[0] )
    {
 //       GetWindowsDirectory( strPath, MAX_PATH );
        if( strcmp( &strPath[strlen(strPath)], TEXT("\\") ) )
            strcat( strPath, TEXT("\\") );
        strcat( strPath, TEXT("MEDIA") );
    }
*/

   // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        return;
    }

	strcpy(m_strFaceList, strFileName);	
	closeFaceList();
	openFaceList();

	m_bWorkWithFiles = true;
}


void CGUIFaceInVideoDoc::OnVideofilePlay() 
{
	m_bVideoFilePaused = false;
	m_PVSystem.playVideo();
}

void CGUIFaceInVideoDoc::OnUpdateVideofilePlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bVideoFileOpen);
}

void CGUIFaceInVideoDoc::OnVideofileRewindFrame() 
{
	m_bVideoFilePaused = true;
	m_PVSystem.singleFrameRewind();
}

void CGUIFaceInVideoDoc::OnUpdateVideofileRewindFrame(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bVideoFileOpen);
}

void CGUIFaceInVideoDoc::OnVideofileRewindStart() 
{
	m_bVideoFilePaused = true;
	m_PVSystem.rewindToStart();
}

void CGUIFaceInVideoDoc::OnUpdateVideofileRewindStart(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bVideoFileOpen);
}

void CGUIFaceInVideoDoc::OnVideofileForwardFrame() 
{
	m_bVideoFilePaused = true;
	m_PVSystem.singleFrameForward();
}

void CGUIFaceInVideoDoc::OnUpdateVideofileForwardFrame(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bVideoFileOpen);
}

void CGUIFaceInVideoDoc::OnVideofileForwardEnd() 
{
	m_bVideoFilePaused = true;
	m_PVSystem.forwardToEnd();
}

void CGUIFaceInVideoDoc::OnUpdateVideofileForwardEnd(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bVideoFileOpen);
}

void CGUIFaceInVideoDoc::OnVideofileStop() 
{
	m_bVideoFilePaused = true;
	m_PVSystem.pauseVideo();
}

void CGUIFaceInVideoDoc::OnUpdateVideofileStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bVideoFileOpen);
}

/*
// Dmitry
void CGUIFaceInVideoDoc::OnCloseDocument() 
{
	POSITION pos = GetFirstViewPosition();
   if(pos != NULL)
   {

	CPerceptualVisionSystem *pPVS = getPVS();

      CTrackerUIView* pView = (CTrackerUIView*)GetNextView(pos);
      
		// Save the tracker information to the registry
		if(pView != NULL)
			pView->

		getPVS()->saveToRegistry(pView->getVideoCameraEntry());
   }
	
	CDocument::OnCloseDocument();
}

  */