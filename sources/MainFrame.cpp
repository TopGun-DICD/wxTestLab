#include "MainFrame.hpp"

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, wxT("Лаборатория тестов, версия 0.0.1а"), wxDefaultPosition, wxSize(800, 600)) {
  p_camWindow = new OpenCVCameraWindow(this, 200);
}

MainFrame::~MainFrame() {
}
