#pragma once

#include <wx/window.h>
#include <wx/timer.h>
#include <wx/image.h>

#include <opencv2/opencv.hpp>
using namespace cv;

enum CameraWindowMode {
  cwm_idle = 0,
  cwm_scan,
};

//URL : http://larryo.org/work/information/wxopencv/index.html
class OpenCVCameraWindow : public wxWindow {
  wxTimer           timerFPS;
  VideoCapture      captureDevice;
  int               captureWidth,
                    captureHeight;
  wxImage           captureImage;
  CameraWindowMode  mode;
public:
  OpenCVCameraWindow(wxWindow *parent, int fps = 1000, int capture_id = 0);
 ~OpenCVCameraWindow();
public:
  void QueryFrameFromCamera();
private:
  void OnPaint(wxPaintEvent &evt);
  void OnTimer(wxTimerEvent &evt);
private:
  bool DetectMarkerAtLeftTopCorner(Point &centerPoint);
  bool DetectMarkerAtRightTopCorner(Point &centerPoint);
  bool DetectMarkerAtLeftBottomCorner(Point &centerPoint);
  bool DetectMarkerAtRightBottomCorner(Point &centerPoint);
};

