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

// Version 1.2.0:
//  Adds control over font size and labels on the sliders.

// Version 1.1.0:
//  Adds the ability to position the display window from within the application
//    and removes the border from the display window.  Unfortunately, there is not
//    a wxWidgets way of keeping this window always on top of other ones (like
//    Task Manager does) without always grabbing focus (by raising periodically).

// Version 1.0.0:
//  Basic version to try and get something going.

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// Turn off depracation warnings from WxWidgets
#ifdef  _WIN32
#pragma warning( disable : 4996 )
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/timer.h"
#include "wx/math.h"
#include "wx/image.h"
#include "wx/stattext.h"
#include "wx/textctrl.h"
#include "wx/slider.h"

#include "parent_notify.h"
#include "sample.xpm"
#include <string.h>

#define	VERSION_STRING	"1.1.0"

// The following part was written for wxWidgets 1.66
MyFrame *imageFrame = NULL;
MyFrame *GUIFrame = NULL;
wxLogWindow *LogWindow = NULL;

//------------------------------------------------------------------------------
// Initial values of things that need to be set in more than one place.

//------------------------------------------------------------------------------
// Global objects required by the callback nature of the code.

static  bool g_show_window = true;    // Show the summoning window?
static  char g_text_message[256];     // The text to show in the window

//------------------------------------------------------------------------------
// Global GUI objects that control our settings

static wxTextCtrl     *g_text_input = NULL;
static wxStaticText   *g_text_display = NULL;
static wxCheckBox     *g_show_summons = NULL;
static wxCheckBox     *g_no_border = NULL;
static wxSlider       *g_x_slider = NULL;
static wxSlider       *g_y_slider = NULL;
static wxSlider       *g_font_size = NULL;

static void InitDebug(wxWindow *parent)
{
  LogWindow = new wxLogWindow(parent, "Log", false, false);
  wxLog::SetActiveTarget(LogWindow);
  wxLogMessage("First message in Debug window\n");
}

static void InitGUI(MyFrame *parent)
{
  wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    static int s_maxWidth = 0;
    if ( !s_maxWidth ) {
        // calc it once only
        parent->GetTextExtent(_T("999 999 999 999 999"), &s_maxWidth, NULL);
    }

    g_text_input = new wxTextCtrl(parent, wxID_ANY, wxEmptyString,
                                      wxDefaultPosition,
                                      wxSize(s_maxWidth, wxDefaultCoord));
    sizer->Add(g_text_input);

    g_show_summons = new wxCheckBox(parent, wxID_ANY, "Show Summons");
    g_show_summons->SetValue(g_show_window);
    sizer->Add(g_show_summons, 0, wxALL | wxALIGN_LEFT, 1);

    g_no_border = new wxCheckBox(parent, wxID_ANY, "Hide Border");
    g_no_border->SetValue(true);
    sizer->Add(g_no_border, 0, wxALL | wxALIGN_LEFT, 1);

    wxStaticText *location_text = new wxStaticText(parent, wxID_ANY, "Location (x,y)");
    sizer->Add(location_text, 0, wxALL | wxALIGN_CENTER, 1);

    int flags = 0;
    flags |= wxSL_LABELS;
    flags |= wxSL_AUTOTICKS;
    flags |= wxSL_TOP;
    g_x_slider = new wxSlider(parent, -1,
                              0, 0, 2000,
                              wxDefaultPosition, wxDefaultSize,
                              flags);
    g_x_slider->SetBackgroundColour(parent->GetBackgroundColour());
    sizer->Add(g_x_slider, 0, wxALL, 1);
    g_y_slider = new wxSlider(parent, -1,
                              0, 0, 1024,
                              wxDefaultPosition, wxDefaultSize,
                              flags);
    g_y_slider->SetBackgroundColour(parent->GetBackgroundColour());
    sizer->Add(g_y_slider, 0, wxALL, 1);

    wxStaticText *fontsize_text = new wxStaticText(parent, wxID_ANY, "Font Size");
    sizer->Add(fontsize_text, 0, wxALL | wxALIGN_CENTER, 1);

    g_font_size = new wxSlider(parent, -1,
                              22, 18, 32,
                              wxDefaultPosition, wxDefaultSize,
                              flags);
    g_font_size->SetBackgroundColour(parent->GetBackgroundColour());
    sizer->Add(g_font_size, 0, wxALL, 1);

  parent->SetSizer(sizer);
  sizer->Fit(parent);
  sizer->SetSizeHints(parent);
}

static void InitImage(MyFrame *parent)
{
  wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    // Leave room for as many characters as we may need.
    g_text_display = new wxStaticText(parent, wxID_ANY, "                       ");
    g_text_display->SetFont(wxFont(g_font_size->GetValue(), wxROMAN, wxNORMAL, wxNORMAL));
    sizer->Add(g_text_display, 0, wxALL | wxALIGN_CENTER, 1);

  parent->SetSizer(sizer);
  sizer->Fit(parent);
  sizer->SetSizeHints(parent);
}

static bool Args(int argc, wxChar **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (wxStrcmp(argv[i], _T("-sb")) == 0) {
            ;
        } else if (wxStrcmp(argv[i], _T("-va")) == 0) {
            ;
        } else {
            wxString msg = _T("Bad option: ");
            msg += argv[i];
            wxMessageBox(msg);
            return false;
        }
    }

    return true;
}

IMPLEMENT_APP(MyApp)

enum
{
  Menu_File_Quit = wxID_EXIT,
};

void  CreateImageFrame(void)
{
  // If there is already an image frame, then destroy it.
  if (imageFrame) {
    delete imageFrame;
    imageFrame = NULL;
  }

  // Create the image frame and initialize all of its elements
  long style = wxDEFAULT_FRAME_STYLE;
  if (g_no_border->GetValue()) {
    style = wxSTAY_ON_TOP || wxSIMPLE_BORDER;
  }
  imageFrame = new MyFrame(NULL, wxT("Parent_notifier" " " VERSION_STRING),
      wxPoint(g_x_slider->GetValue(), g_y_slider->GetValue()), wxSize(100,130), style);
  imageFrame->SetIcon(wxIcon(_T("mondrian")));
  imageFrame->Show(g_show_window);
  InitImage(imageFrame);
}

// `Main program' equivalent, creating windows and returning main app frame
bool MyApp::OnInit()
{
    sprintf(g_text_message, "");
    Args(argc, argv);

    // Create the GUI frame first so we have the widgets we need when we
    // create the display window.
    GUIFrame = new MyFrame(NULL, wxT("Who to summon"),
        wxPoint(710, 0), wxDefaultSize);
    GUIFrame->SetIcon(wxIcon(_T("mondrian")));
    GUIFrame->Show(1);
    InitGUI(GUIFrame);

    CreateImageFrame();

    // It just needs to be passed a valid window, as it creates its own
    InitDebug(GUIFrame);

    //----------------------------------------------------------------------
    return true;
}

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Menu_File_Quit, MyFrame::OnExit)
    EVT_IDLE(MyFrame::WhenIdle)
    EVT_CLOSE(MyFrame::OnCloseWindow)
    EVT_TEXT_ENTER(wxID_ANY, MyFrame::OnTextEnter)
END_EVENT_TABLE()

// My frame constructor
MyFrame::MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style)
{
    SetIcon(wxIcon(sample_xpm));
}

MyFrame::~MyFrame()
{
}

// Intercept menu commands
void MyFrame::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    wxTheApp->ExitMainLoop();
}

// If the user closes one of the windows, exit the
// program to avoid causing a seg fault when the window
// gets destroyed and we still have references to objects
// in it.
void MyFrame::OnCloseWindow( wxCloseEvent& event )
{
  wxTheApp->ExitMainLoop();
}

// What to do when idle
void MyFrame::WhenIdle( wxIdleEvent & )
{
  // Sleep to avoid eating the whole processor.
  wxMilliSleep(100);

  // Set the text to be displayed
  g_text_display->SetLabel(g_text_message);

  // If the details of the text display window have changed, then
  // recreate it.
  static bool on_top = g_no_border->GetValue();
  if (on_top != g_no_border->GetValue()) {
    CreateImageFrame();
    on_top = g_no_border->GetValue();
  }
  static int x = g_x_slider->GetValue();
  static int y = g_y_slider->GetValue();
  if ( (x != g_x_slider->GetValue()) || (y != g_y_slider->GetValue()) ) {
    CreateImageFrame();
    x = g_x_slider->GetValue();
    y = g_y_slider->GetValue();
  }
  static int fontsize = g_font_size->GetValue();
  if (fontsize != g_font_size->GetValue()) {
    CreateImageFrame();
    fontsize = g_font_size->GetValue();
  }

  // Show or hide the display window based on whether the
  // check-box is on or off (hide or show the window when
  // the setting doesn't match).
  if (g_show_window != g_show_summons->GetValue()) {
    imageFrame->Show(g_show_summons->GetValue());
    g_show_window = g_show_summons->GetValue();
  }
}

void MyFrame::OnTextEnter(wxCommandEvent& event)
{
  strncpy(g_text_message, event.GetString().c_str(), 255);
}