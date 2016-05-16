// DeviceDialog.cpp : implementation file
//

#include "stdafx.h"
#include "GUIFaceInVideo.h"
#include "DeviceDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeviceDialog dialog


CDeviceDialog::CDeviceDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeviceDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pSelectedEntry = NULL;
	//m_nWidth = 320;  // Set Video capture width
	//m_nHeight = 240; // Set Video capture height
	m_nWidth = 160;  // Set Video capture width
	m_nHeight = 120; // Set Video capture height
	m_nBpp = 24; // 8<-- it appears coloured anyway???; // 24;

//Dmitry. Does not work this way!..
/*	for (int nSelection=0; nSelection<4;nSelection++)
	{
		m_pSelectedEntry = m_deviceList[nSelection];
		if (m_pSelectedEntry != NULL)
			break;
	}
*/
}


void CDeviceDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviceDialog)
	DDX_Control(pDX, IDC_DEVICE_LISTBOX, m_deviceListBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeviceDialog, CDialog)
	//{{AFX_MSG_MAP(CDeviceDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CDeviceDialog::~CDeviceDialog()
{
	vl_purgeDeviceList(m_deviceList);
}

/////////////////////////////////////////////////////////////////////////////
// CDeviceDialog message handlers

BOOL CDeviceDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	vl_getDeviceList(m_deviceList);

	CListBox* pListBox = (CListBox*)GetDlgItem( IDC_DEVICE_LISTBOX );
	
	if(pListBox)
	{
		pListBox->ResetContent();

		for(int ii = 0;ii < m_deviceList.size();ii++)
		{
			// Store the entry into the list
			pListBox->InsertString(ii, m_deviceList[ii]->getDeviceName());
		}			
	}

	return TRUE;	// return TRUE unless you set the focus to a control
						// EXCEPTION: OCX Property Pages should return FALSE
}


void CDeviceDialog::OnOK() 
{
	int nSelection = m_deviceListBox.GetCurSel();

	if(nSelection != CB_ERR)
		m_pSelectedEntry = m_deviceList[nSelection];
	else
		m_pSelectedEntry = NULL;
	
	CDialog::OnOK();
}
