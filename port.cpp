// port.cpp : implementation file
//

#include "stdafx.h"
#include "z3ed.h"
#include "port.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// port dialog


port::port(CWnd* pParent /*=NULL*/)
	: CDialog(port::IDD, pParent)
{
	//{{AFX_DATA_INIT(port)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void port::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(port)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(port, CDialog)
	//{{AFX_MSG_MAP(port)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// port message handlers
