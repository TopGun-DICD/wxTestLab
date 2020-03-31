#include <wx/app.h>

#include "MainFrame.hpp"

class TestLabApp : public wxApp {
public:
  virtual bool OnInit();
};

bool TestLabApp::OnInit() {
  MainFrame *p_frame = new MainFrame;
  if (!p_frame)
    return false;
  p_frame->Show(true);
  return true;
}


wxDECLARE_APP(TestLabApp);
wxIMPLEMENT_APP(TestLabApp);

