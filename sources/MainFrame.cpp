#include "MainFrame.hpp"

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, wxT("����������� ������, ������ 0.0.1�"), wxDefaultPosition, wxSize(800, 600)) {
  p_camWindow = new OpenCVCameraWindow(this, 200);
}

MainFrame::~MainFrame() {
}
