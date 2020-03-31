#include "OpenCVCameraWindow.hpp"

#define wxID_TIMER        1234
#define CROP_WIDTH        40
#define CROP_HEIGHT       40
#define THRESHOLD_BLACK   50
#define THRESHOLD_WHITE   120

#include <wx/dcclient.h>
#include <vector>
using namespace std;

#pragma comment(lib, "opencv_world347d.lib")

Mat img_color;

//URL : https://note.nkmk.me/en/python-opencv-bgr-rgb-cvtcolor/
OpenCVCameraWindow::OpenCVCameraWindow(wxWindow *parent, int fps, int capture_id) : wxWindow(parent, wxID_ANY), timerFPS(this, wxID_TIMER), captureDevice(capture_id), captureWidth(0), captureHeight(0)  {
  Connect(wxEVT_PAINT, wxPaintEventHandler(OpenCVCameraWindow::OnPaint));
  
  captureWidth   = captureDevice.get(CV_CAP_PROP_FRAME_WIDTH);
  captureHeight  = captureDevice.get(CV_CAP_PROP_FRAME_HEIGHT);
  captureImage.Create(wxSize(captureWidth, captureHeight), true);

  Connect(timerFPS.GetId(), wxEVT_TIMER, wxTimerEventHandler(OpenCVCameraWindow::OnTimer), nullptr, this);
  timerFPS.Start(fps);
}

OpenCVCameraWindow::~OpenCVCameraWindow() {
  timerFPS.Stop();
}

void OpenCVCameraWindow::OnPaint(wxPaintEvent &evt) {
  wxPaintDC dc(this);

  if (!img_color.data)
    return;

  captureImage.SetData(img_color.data, true);
  dc.DrawBitmap(wxBitmap(captureImage), wxPoint(0, 0));

}

void OpenCVCameraWindow::OnTimer(wxTimerEvent &evt) {
  if (!captureDevice.isOpened())
    return;

  // Get next frame from device
  captureDevice >> img_color;
  cvtColor(img_color, img_color, COLOR_BGR2RGB);
  flip(img_color, img_color, 1);

  // Create gray area from image scanning
  cv::Mat tmp;
  cv::Rect roi(340, 20, 280, 440);
  cv::cvtColor(img_color, tmp, CV_BGR2GRAY);
  cv::cvtColor(tmp, tmp, CV_GRAY2BGR);
  tmp(roi).copyTo(img_color(roi));

  Point pointLeftTop,
        pointRightTop,
        pointLeftBottom,
        pointRightBottom;

  bool markerLeftTop      = DetectMarkerAtLeftTopCorner(pointLeftTop);
  bool markerRightTop     = DetectMarkerAtRightTopCorner(pointRightTop);
  bool markerLeftBottom   = DetectMarkerAtLeftBottomCorner(pointLeftBottom);
  bool markerRightBottom  = DetectMarkerAtRightBottomCorner(pointRightBottom);

  // Big rectangle
  if(markerLeftTop && markerRightTop && markerLeftBottom && markerLeftBottom)
    rectangle(img_color, Rect(340, 20, 280, 440), Scalar(0, 255, 0));
  else
    rectangle(img_color, Rect(340, 20, 280, 440), Scalar(255, 0, 0));

  // Left and Top
  if (markerLeftTop)
    rectangle(img_color, Rect(340, 20, CROP_WIDTH, CROP_HEIGHT), Scalar(0, 255, 0));
  else
    rectangle(img_color, Rect(340, 20, CROP_WIDTH, CROP_HEIGHT), Scalar(255, 0, 0));

  // Right and Top
  if (markerRightTop)
    rectangle(img_color, Rect(580, 20, CROP_WIDTH, CROP_HEIGHT), Scalar(0, 255, 0));
  else
    rectangle(img_color, Rect(580, 20, CROP_WIDTH, CROP_HEIGHT), Scalar(255, 0, 0));

  // Left and Bottom
  if (markerLeftBottom)
    rectangle(img_color, Rect(340, 420, CROP_WIDTH, CROP_HEIGHT), Scalar(0, 255, 0));
  else
    rectangle(img_color, Rect(340, 420, CROP_WIDTH, CROP_HEIGHT), Scalar(255, 0, 0));

  // Right and Bottom
  if (markerRightBottom)
    rectangle(img_color, Rect(580, 420, CROP_WIDTH, CROP_HEIGHT), Scalar(0, 255, 0));
  else
    rectangle(img_color, Rect(580, 420, CROP_WIDTH, CROP_HEIGHT), Scalar(255, 0, 0));

  Refresh();
}

// URL : https://www.learnopencv.com/find-center-of-blob-centroid-using-opencv-cpp-python/
// URL : https://gist.github.com/yoggy/1750751
// URL : http://opencvexamples.blogspot.com/2013/09/find-contour.html
bool DetectRectangleInImage(cv::Mat &src_img, cv::Point &result) {
  Mat gray;

  cvtColor(src_img, gray, CV_RGB2GRAY);
  threshold(gray, gray, 127, 255, THRESH_BINARY_INV);
  //imshow("gray", gray); 
  int largest_area = 0;
  int largest_contour_index = 0;
  Rect bounding_rect;
  vector<vector<Point>> contours; // Vector for storing contour
  vector<Vec4i> hierarchy;
  findContours(gray, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

  for (int i = 0; i < contours.size(); ++i) {
    double a = contourArea(contours[i], false);
    if (a < 10)
      continue;
    if (a > largest_area) {
      largest_area = a;
      largest_contour_index = i;
      bounding_rect = boundingRect(contours[i]);

      Moments m = moments(contours[i], true);
      result = Point(m.m10 / m.m00, m.m01 / m.m00);
    }
    //drawContours(src_img, contours, i, Scalar(0, 255, 255), 1, 8, hierarchy);
  }

  return true;
}

bool OpenCVCameraWindow::DetectMarkerAtLeftTopCorner(Point &centerPoint) {
  const int cropLeftCoord = 340;
  const int cropTopCoord = 20;
  Rect crop(cropLeftCoord, cropTopCoord, CROP_WIDTH, CROP_HEIGHT);
  Mat img_roi = img_color(crop);

  // Detect marker in top left area
  int numOfBlackColors = 0;
  int numOfWhiteColors = 0;
  for (int y = 1; y < CROP_HEIGHT; ++y) {
    for (int x = 1; x < CROP_WIDTH; ++x) {
      Vec3b & color = img_roi.at<Vec3b>(y, x);
      if (color[0] < THRESHOLD_BLACK && color[1] < THRESHOLD_BLACK && color[2] < THRESHOLD_BLACK) {
        ++numOfBlackColors;
        color[0] = 0;
        color[1] = 0;
        color[2] = 255;
      }
      else
        if (color[0] > THRESHOLD_WHITE && color[1] > THRESHOLD_WHITE && color[2] > THRESHOLD_WHITE)
          ++numOfWhiteColors;
    }
  }

  bool blackColorDetected = (numOfBlackColors > 50 && numOfBlackColors < 200 && numOfWhiteColors > 1000);
  if (!blackColorDetected)
    return false;

  DetectRectangleInImage(img_roi, centerPoint);
  circle(img_color, Point(centerPoint.x + cropLeftCoord, centerPoint.y + cropTopCoord), 3, Scalar(255, 255, 0));
  //img_roi.copyTo(img_color(Rect(10, 10, 40, 40)));
  char buf[32];
  sprintf(buf, "%d", numOfBlackColors);
  putText(img_color, String(buf), Point(cropLeftCoord, cropTopCoord), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
  return true;
}

bool OpenCVCameraWindow::DetectMarkerAtRightTopCorner(Point &centerPoint) {
  const int cropLeftCoord = 580;
  const int cropTopCoord = 20;
  Rect crop(cropLeftCoord, cropTopCoord, CROP_WIDTH, CROP_HEIGHT);
  Mat img_roi = img_color(crop);

  // Detect marker in top left area
  int numOfBlackColors = 0;
  int numOfWhiteColors = 0;
  for (int y = 1; y < CROP_HEIGHT; ++y) {
    for (int x = 1; x < CROP_WIDTH; ++x) {
      Vec3b & color = img_roi.at<Vec3b>(y, x);
      if (color[0] < THRESHOLD_BLACK && color[1] < THRESHOLD_BLACK && color[2] < THRESHOLD_BLACK) {
        ++numOfBlackColors;
        color[0] = 0;
        color[1] = 0;
        color[2] = 255;
      }
      else
        if (color[0] > THRESHOLD_WHITE && color[1] > THRESHOLD_WHITE && color[2] > THRESHOLD_WHITE)
          ++numOfWhiteColors;
    }
  }

  bool blackColorDetected = (numOfBlackColors > 50 && numOfBlackColors < 200 && numOfWhiteColors > 1000);
  if (!blackColorDetected)
    return false;

  DetectRectangleInImage(img_roi, centerPoint);
  circle(img_color, Point(centerPoint.x + cropLeftCoord, centerPoint.y + cropTopCoord), 3, Scalar(255, 255, 0));
  //img_roi.copyTo(img_color(Rect(10, 10, 40, 40)));
  char buf[32];
  sprintf(buf, "%d", numOfBlackColors);
  putText(img_color, String(buf), Point(cropLeftCoord, cropTopCoord), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
  return true;
}

bool OpenCVCameraWindow::DetectMarkerAtLeftBottomCorner(Point &centerPoint) {
  const int cropLeftCoord = 340;
  const int cropTopCoord = 420;
  Rect crop(cropLeftCoord, cropTopCoord, CROP_WIDTH, CROP_HEIGHT);
  Mat img_roi = img_color(crop);

  // Detect marker in top left area
  int numOfBlackColors = 0;
  int numOfWhiteColors = 0;
  for (int y = 1; y < CROP_HEIGHT; ++y) {
    for (int x = 1; x < CROP_WIDTH; ++x) {
      Vec3b & color = img_roi.at<Vec3b>(y, x);
      if (color[0] < THRESHOLD_BLACK && color[1] < THRESHOLD_BLACK && color[2] < THRESHOLD_BLACK) {
        ++numOfBlackColors;
        color[0] = 0;
        color[1] = 0;
        color[2] = 255;
      }
      else
        if (color[0] > THRESHOLD_WHITE && color[1] > THRESHOLD_WHITE && color[2] > THRESHOLD_WHITE)
          ++numOfWhiteColors;
    }
  }

  bool blackColorDetected = (numOfBlackColors > 50 && numOfBlackColors < 200 && numOfWhiteColors > 1000);
  if (!blackColorDetected)
    return false;

  DetectRectangleInImage(img_roi, centerPoint);
  circle(img_color, Point(centerPoint.x + cropLeftCoord, centerPoint.y + cropTopCoord), 3, Scalar(255, 255, 0));
  //img_roi.copyTo(img_color(Rect(10, 10, 40, 40)));
  char buf[32];
  sprintf(buf, "%d", numOfBlackColors);
  putText(img_color, String(buf), Point(cropLeftCoord, cropTopCoord), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
  return true;
}

bool OpenCVCameraWindow::DetectMarkerAtRightBottomCorner(Point &centerPoint) {
  const int cropLeftCoord = 580;
  const int cropTopCoord = 420;
  Rect crop(cropLeftCoord, cropTopCoord, CROP_WIDTH, CROP_HEIGHT);
  Mat img_roi = img_color(crop);

  // Detect marker in top left area
  int numOfBlackColors = 0;
  int numOfWhiteColors = 0;
  for (int y = 1; y < CROP_HEIGHT; ++y) {
    for (int x = 1; x < CROP_WIDTH; ++x) {
      Vec3b & color = img_roi.at<Vec3b>(y, x);
      if (color[0] < THRESHOLD_BLACK && color[1] < THRESHOLD_BLACK && color[2] < THRESHOLD_BLACK) {
        ++numOfBlackColors;
        color[0] = 0;
        color[1] = 0;
        color[2] = 255;
      }
      else
        if (color[0] > THRESHOLD_WHITE && color[1] > THRESHOLD_WHITE && color[2] > THRESHOLD_WHITE)
          ++numOfWhiteColors;
    }
  }

  bool blackColorDetected = (numOfBlackColors > 50 && numOfBlackColors < 200 && numOfWhiteColors > 1000);
  if (!blackColorDetected)
    return false;

  DetectRectangleInImage(img_roi, centerPoint);
  circle(img_color, Point(centerPoint.x + cropLeftCoord, centerPoint.y + cropTopCoord), 3, Scalar(255, 255, 0));
  //img_roi.copyTo(img_color(Rect(10, 10, 40, 40)));
  char buf[32];
  sprintf(buf, "%d", numOfBlackColors);
  putText(img_color, String(buf), Point(cropLeftCoord, cropTopCoord), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
  return true;
}

