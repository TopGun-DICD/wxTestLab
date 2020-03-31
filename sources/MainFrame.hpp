#pragma once

#include <wx/frame.h>

#include "OpenCVCameraWindow.hpp"

class MainFrame : public wxFrame {
  OpenCVCameraWindow *p_camWindow;
public:
  MainFrame();
 ~MainFrame();
};

