#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include "obcore/base/tools.h"
#include "obcore/base/Timer.h"
#include "obdevice/UvcCam.h"
#include "obgraphic/Obvious2D.h"
#include "3D_uvc.h"

using namespace std;
using namespace obvious;

UvcCam* _cam;
unsigned char* _img;
unsigned char* _img1;
unsigned char* _img2;
unsigned char* _addedImg;


unsigned int _widthPrev = 640;//1920;
unsigned int _heightPrev = 360;//1080;
unsigned int _rotation = 0;


void toggleMJPEG()
{
   unsigned int width = _widthPrev;
   unsigned int height = _heightPrev;
   _widthPrev = _cam->getWidth();
   _heightPrev = _cam->getHeight();
   _cam->disconnect();
   delete [] _img;
   _img = new unsigned char[width*height*3];
   _cam->connect();
   _cam->setFormat(width, height, V4L2_PIX_FMT_MJPEG);
   _cam->startStreaming();
}

void saveHighresImage()
{
  unsigned int width = 1920;
  unsigned int height = 1080;
  char* path;

  _widthPrev = _cam->getWidth();
  _heightPrev = _cam->getHeight();
  _cam->disconnect();
  delete [] _img;
  _img = new unsigned char[width*height*3];
  _cam->connect();
  _cam->setFormat(width, height, V4L2_PIX_FMT_YUYV);
  _cam->startStreaming();
  unsigned int bytes;
  _cam->grab(_img, &bytes);
  sprintf(path, "pic/uvc_image_%i.ppm",_rotation);
  serializePPM(path, _img, _cam->getWidth(), _cam->getHeight(), 0);
  toggleMJPEG();
  cout << "snapshot saved to " << path << endl;
}

void addingImage()
{
  char* path;

  _widthPrev = _cam->getWidth();
  _heightPrev = _cam->getHeight();
  _cam->disconnect();
  delete [] _img1;
  _img1 = new unsigned char[_widthPrev*_heightPrev*3];
  delete [] _img2;
  _img2 = new unsigned char[_widthPrev*_heightPrev*3];

  _cam->connect();
  _cam->setFormat(_widthPrev, _heightPrev, V4L2_PIX_FMT_YUYV);
  _cam->startStreaming();
  unsigned int bytes;

// first record
  _cam->grab(_img1, &bytes);
  sprintf(path, "pic/uvc_image_%i.ppm",_rotation);
  serializePPM(path, _img1, _cam->getWidth(), _cam->getHeight(), 0);
  cout << "saved to " << path << endl;
  cout << "rotation: " << _rotation << endl;


  cout << "sleep"<< endl;
    sleep(5);
  cout << "wakeup"<< endl;

// Next step
  _rotation = _rotation + 150;

// next record
  _cam->grab(_img2, &bytes);
  sprintf(path, "pic/uvc_image_%i.ppm",_rotation);
  serializePPM(path, _img2, _cam->getWidth(), _cam->getHeight(), 0);
  cout << "saved to " << path << endl;
  cout << "rotation: " << _rotation << endl;

  cout << "sleep"<< endl;
    sleep(5);
  cout << "wakeup"<< endl;

// Adding
   sprintf(path, "pic/uvc_added_%i.ppm",_rotation);
   filePPM(path, _img1, _img2, _cam->getWidth(), _cam->getHeight(), _rotation);
   cout << "added saved to " << path << endl;
// Changing back camera view settings
   toggleMJPEG();

}

int filePPM(const char* szFilename, void* pvBuffer1, void* pvBuffer2, unsigned int nWidth, unsigned int nHeight, unsigned int rotation)
{
  int nRowInc = 0;
  int nRowStart = 0;
  int i = 0;
  int j = 0;
  int k = 0;

  FILE *pFile;

  //checking if saving one image or a added images
  nWidth = 2*nWidth;

  // Open file
  pFile = fopen(szFilename, "wb");
  if (pFile == NULL) return 0;

  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", nWidth, nHeight);

  // Write pixel data
  unsigned char* buffer1 = (unsigned char*) pvBuffer1;
  unsigned char* buffer2 = (unsigned char*) pvBuffer2;

  /* Go to the end of the buffer */
  buffer1 += nRowStart;
  buffer2 += nRowStart;

  for (i = 0; i < nHeight; i++)
  {
    for (j = 0; j < nWidth/2; j++)
    {
      putc(buffer1[0], pFile); /* b */
      putc(buffer1[1], pFile); /* g */
      putc(buffer1[2], pFile); /* r */
      buffer1 += 3;
    }
    for (k = 0; k < (nWidth/2); k++)
    {
      putc(buffer2[0], pFile); /* b */
      putc(buffer2[1], pFile); /* g */
      putc(buffer2[2], pFile); /* r */
      buffer2 += 3;
    }
    /* Move to next line */
    buffer1 += 2 * nRowInc;
    buffer2 += 2 * nRowInc;

  }

  // Close file
  fclose(pFile);

  return 1;
}


int main(int argc, char* argv[])
{
   // Default parameters
   char* dev           = (char*)"/dev/video0";
   unsigned int width       = 640;
   unsigned int height      = 360;

   if(argc>1) dev = argv[1];
   if(argc>2)
   {
      if(argc!=4)
      {
         cout << "usage: " << argv[0] << " [device] [width height]" << endl;
         return 0;
      }
      width           = atoi(argv[2]);
      height          = atoi(argv[3]);
   }

   _cam = new UvcCam(dev, width, height);

   EnumCameraError retval = _cam->connect();
   if(retval!=CAMSUCCESS) return -1;

   retval = _cam->setFormat(width, height, V4L2_PIX_FMT_MJPEG);
   if(retval!=CAMSUCCESS) return -1;

   Obvious2D viewer(1280, 720, "UVC streaming example");
   viewer.registerKeyboardCallback(' ', toggleMJPEG);
   viewer.registerKeyboardCallback('s', saveHighresImage);
   viewer.registerKeyboardCallback('r', addingImage);
   _img = new unsigned char[width*height*3];

   retval = _cam->setFramerate(1,15);
   if(retval!=CAMSUCCESS) return -1;

   retval = _cam->startStreaming();
   if(retval!=CAMSUCCESS) return -1;

   Timer t;
   while(retval==CAMSUCCESS && viewer.isAlive())
   {
      unsigned int bytes;
      retval = _cam->grab(_img, &bytes);
      if(retval==CAMSUCCESS)
      {
         viewer.draw(_img, _cam->getWidth(), _cam->getHeight(), 3, (_cam->getFormat()==V4L2_PIX_FMT_MJPEG), bytes);
      //   cout << "Elapsed: " << t.reset() << " ms" << endl;
      }

   }

   delete [] _img;
   delete _cam;
   return 0;
}
