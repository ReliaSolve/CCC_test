/////////////////////////////////////////////////////////////////////////////
// Name:        parent_notify.cpp
// Purpose:     Tells parents to come find their children by flashing up a
//              small set of numbers in a box on the second screen in front
//              of the Powerpoint show.
// Author:      Russell M. Taylor II
// Created:     09/06/2008
// Copyright:   (c) Russell M. Taylor II
// Licence:     public domain
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PARENT_NOTIFY_H_
#define _WX_PARENT_NOTIFY_H_

// Define a new application type
class MyApp: public wxApp
{
public:
    bool OnInit();
};

class MyFrame: public wxFrame
{
public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

    virtual ~MyFrame();

private :

    void OnExit(wxCommandEvent& event);
    void WhenIdle(wxIdleEvent &);
    void OnCloseWindow(wxCloseEvent& event);
    void OnTextEnter(wxCommandEvent& event);

DECLARE_EVENT_TABLE()
};

#endif // #ifndef _WX_PARENT_NOTIFY_H_

