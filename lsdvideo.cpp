#include <iostream>
#include <sstream>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>


#define STATE_PLAYING 0
#define STATE_RECORDING 1 // save as it is
#define STATE_EDITING 2  // editing & recording
#define STATE_NEXTFRAME 3 // almost like editing, load next frame

using namespace std;
using namespace cv;
void help() ;
void CallBackFunc(int event, int x, int y, int flags, void* userdata) ;
inline int max(int a, int b) { return a > b ? a : b;}
inline int min(int a, int b) { return a < b ? a : b;}
inline double distance(double pnt1x, double pnt1y, double pnt2x, double pnt2y) { return fabs(pow(pnt1x-pnt2x,2) +  pow(pnt1y-pnt2y,2) ) ;  }

int mouseX1=0, mouseY1=0, mouseX2=0, mouseY2=0;
int state  = STATE_PLAYING ;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        help();
        cout << "Not enough parameters" << endl;
        return -1;
    }

    const string sourceReference = argv[1];
    const string output = argv[2];
    char key;
    int frameNum = 0;

    VideoCapture captRefrnc(sourceReference);

    if (!captRefrnc.isOpened())
    {
        cout  << "Could not open reference " << sourceReference << endl;
        return -1;
    }

    Size refS = Size((int) captRefrnc.get(CAP_PROP_FRAME_WIDTH),
                     (int) captRefrnc.get(CAP_PROP_FRAME_HEIGHT));
    const char* WIN_RF = "LSD editor";

    // Windows
    namedWindow(WIN_RF, WINDOW_AUTOSIZE);
    //moveWindow(WIN_RF, 400 , 0);

    Mat frame;
    Mat gray;

    ofstream fh;
    fh.open (output.c_str());
    int size_limit ;
    int N ;
    Mat _lines;
    vector<Vec4f> lines_lsd;
    vector<vector<Vec4f> > stateVec ;
    Ptr<LineSegmentDetector> ls ;
    int minx, maxx, miny, maxy;
    Mat empty(refS.height, refS.width, CV_8UC1, Scalar(0,0,0)); // empty image
    while(1){
        if(state == STATE_PLAYING || state == STATE_RECORDING || state == STATE_NEXTFRAME){
          stateVec.clear() ;
          cout << "loading a frame" ;
          mouseX1=-1;  mouseY1=-1 ; mouseX2=-1;  mouseY2=-1;
          empty = Mat::zeros(refS.height, refS.width, CV_8UC1);
          frameNum++;
          size_limit =1 ;
          do captRefrnc >> frame;
             while(frame.empty()) ; //some frames may be empty
          cvtColor(frame, gray, CV_BGR2GRAY);
          ls = createLineSegmentDetector(LSD_REFINE_STD);
          ls->detect(gray, lines_lsd);
          stateVec.push_back(lines_lsd);

          if (state == STATE_NEXTFRAME) state = STATE_EDITING;
        } else { // (state == STATE_EDITING )
            key = waitKey(100); // read command
            switch(key){
            case 'q':
                fh.close();
                return 0 ; //quit
            case 'u': // undo
                if(!stateVec.empty()){
                    lines_lsd = stateVec.back();
                    stateVec.pop_back();
                    cout << "Undo" <<endl ;
                }else{
                    cout << "Undo blocked" << endl ;
                }
                break;
            case 'a': // add a line
                stateVec.push_back(lines_lsd) ;
                cout << "Add" <<endl ;
                lines_lsd.push_back(Vec4f(mouseX1,mouseY1, mouseX2, mouseY2)  );
                break;
            case 'c': // removing short lines
                stateVec.push_back(lines_lsd) ;
                for(int i=0 ; i < lines_lsd.size() ;  i++ ){
                  if (distance(lines_lsd[i][0],lines_lsd[i][1], lines_lsd[i][2], lines_lsd[i][3] ) < size_limit * size_limit ) {
                      lines_lsd.erase(lines_lsd.begin() + i);
                      i--;
                    }
                }
                size_limit ++ ;
                break;
            case 'r': // remove lines
                stateVec.push_back(lines_lsd) ;
                cout << "Remove" <<endl ;
                minx = min(mouseX1, mouseX2);
                maxx = max(mouseX1, mouseX2);
                miny = min(mouseY1, mouseY2);
                maxy = max(mouseY1, mouseY2);
                for(int i=0 ; i < lines_lsd.size() ;  i++ ){
                  if ((  lines_lsd[i][0] > minx
                      && lines_lsd[i][0] < maxx
                      && lines_lsd[i][1] > miny
                      && lines_lsd[i][1] < maxy
                    ) ||
                    (    lines_lsd[i][2] > minx
                      && lines_lsd[i][2] < maxx
                      && lines_lsd[i][3] > miny
                      && lines_lsd[i][3] < maxy
                    )) {
                      lines_lsd.erase(lines_lsd.begin() + i);
                      i-- ;  // new indexes when an index is removed
                    }
                }
                break;
            case 's': //save, next frame
                stateVec.clear() ;

                cout << "Recorded" << endl ;
                fh << "frame:" << frameNum << endl;
                fh << "lines:" <<  lines_lsd.size() << endl;
                for(int i= 0 ; i <  lines_lsd.size(); i++){
                  fh <<  lines_lsd[i][0] << ","<<  lines_lsd[i][1] << ","
                     <<  lines_lsd[i][2] << ","<<  lines_lsd[i][3] << endl;
                }
                state = STATE_NEXTFRAME ;
                break;
             }
         }
         ls->drawSegments(empty, lines_lsd);
         if (mouseX1 > 0 && mouseY1 > 0 ){
           circle(empty, Point( mouseX1, mouseY1) , 3, Scalar( 255, 0 , 255 ), 1, 8, 0);
         }
         if (mouseX2 > 0 && mouseY2 > 0 ){
           circle(empty, Point( mouseX2, mouseY2) , 3, Scalar( 255, 0 , 0 ), 1, 8, 0);
         }

         waitKey(1); // Don't remove this.
         imshow(WIN_RF, empty);
         setMouseCallback(WIN_RF, CallBackFunc, NULL);
    }
    fh.close();
    return 0;
}


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
switch( event ){
    case CV_EVENT_RBUTTONDOWN:
        cout << "CV_EVENT_RBUTTONDOWN"<<endl ;
        if (state == STATE_EDITING) {
          state = STATE_PLAYING ;
        }else {
          state = STATE_EDITING ;
        }
        break;
    case CV_EVENT_LBUTTONUP:
        cout << "CV_EVENT_LBUTTONDOWN:" << x << "-" << y << endl ;
        mouseX1 = x  ;
        mouseY1 = y ;
        break ;
    case CV_EVENT_LBUTTONDOWN:
        cout << "CV_EVENT_LBUTTONUP:" << x << "-" << y << endl ;
        mouseX2 = x ;
        mouseY2 = y ;
        break;

    //case CV_EVENT_MBUTTONDOWN:
    case CV_EVENT_MBUTTONDOWN:
        cout << "CV_EVENT_MBUTTONDOWN" <<endl;
        break;
    }
}

void help()
{
    cout
        << "------------------------------------------------------------------------------" << endl
        << "This program applys LSD filter to a video  "                                    << endl
        << "Usage:"                                                                         << endl
        << "./lsd video.avi output.lsd "                                                    << endl
        << "------------------------------------------------------------------------------" << endl << endl;
}
// for debugging
//cout << lines_lsd[i][0] << "," << lines_lsd[i][1] << ","
//     << lines_lsd[i][2] << "," << lines_lsd[i][3] << endl ;
