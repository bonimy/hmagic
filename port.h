#if !defined(AFX_PORT_H__6CA2B860_BAC4_11D9_B639_00045A899647__INCLUDED_)
#define AFX_PORT_H__6CA2B860_BAC4_11D9_B639_00045A899647__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// port.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// port dialog

class port : public CDialog
{
// Construction
public:
	port(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(port)
	enum { IDD = IDD_DIALOG25 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(port)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(port)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORT_H__6CA2B860_BAC4_11D9_B639_00045A899647__INCLUDED_)
