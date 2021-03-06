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

inline bool X1(int x, int y) {return (x==y);}
using namespace std;
using namespace cv;
void help() ;
void CallBackFunc(int event, int x, int y, int flags, void* userdata) ;
inline int max(int a, int b) { return a > b ? a : b;}
inline int min(int a, int b) { return a < b ? a : b;}
inline double distance_pow2(double pnt1x, double pnt1y, double pnt2x, double pnt2y) { return fabs(pow(pnt1x-pnt2x,2) +  pow(pnt1y-pnt2y,2) ) ;  }
inline double line_size(Vec4f l);
inline int merge_2lines( Vec4f l1, Vec4f l2, double gr, double dist);
int mouseX1=0, mouseY1=0, mouseX2=0, mouseY2=0;
int state  = STATE_PLAYING ;
int size_before_change;
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
    int merge_size_min, merge_size_max, remove_size_min ;
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
          remove_size_min =1 ;
          merge_size_min =1;
          merge_size_max = 50 ; //refS.height ;
          do captRefrnc >> frame;
             while(frame.empty()) ; //some frames may be empty
          cvtColor(frame, gray, CV_BGR2GRAY);
          ls = createLineSegmentDetector(LSD_REFINE_STD);
          ls->detect(gray, lines_lsd);
          stateVec.push_back(lines_lsd);
          size_before_change =lines_lsd.size() ;
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
                  if (distance_pow2(lines_lsd[i][0],lines_lsd[i][1], lines_lsd[i][2], lines_lsd[i][3] ) < merge_size_min * merge_size_min ) {
                      lines_lsd.erase(lines_lsd.begin() + i);
                      i--;
                    }
                }
                remove_size_min ++ ;
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
            case 'm': //merge lines with close gradient
                stateVec.push_back(lines_lsd) ;
                cout << "TTT"<<  size_before_change << "Merge size max  "<< merge_size_max << endl;
                for(int i=0 ; i <   size_before_change;  i++ ){
                    for(int j=i+1; j <  size_before_change ; j++ ){
                      int mr = merge_2lines(lines_lsd[i], lines_lsd[j],0.5, merge_size_max ) ;
                      if (mr != 0) cout << "merging" << mr << lines_lsd[i][0]<< lines_lsd[i][1] <<  lines_lsd[j][0]<< lines_lsd[j][1] << endl;
                      if( mr == 1) lines_lsd.push_back(Vec4f(lines_lsd[i][0],lines_lsd[i][1], lines_lsd[j][0], lines_lsd[j][1])  );
                      if( mr == 2) lines_lsd.push_back(Vec4f(lines_lsd[i][0],lines_lsd[i][1], lines_lsd[j][2], lines_lsd[j][3])  );
                      if( mr == 3) lines_lsd.push_back(Vec4f(lines_lsd[i][2],lines_lsd[i][3], lines_lsd[j][0], lines_lsd[j][1])  );
                      if( mr == 4) lines_lsd.push_back(Vec4f(lines_lsd[i][2],lines_lsd[i][3], lines_lsd[j][2], lines_lsd[j][3])  );
                    }
                }
                cout << merge_size_max << endl ;
                merge_size_max -- ;
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

inline double line_size(Vec4f l){
   return sqrt(   pow(l[0]-l[2],2)    +   pow(l[1] - l[3] ,2 ));
}

inline double line_gradient(Vec4f l){
   return fabs((l[0]-l[2]) / (l[1]-l[3])) ;
}

/*
gr : gradient
dist: distance
return
  1: l1x1,l1y1 & l2x1,l2y1
  2: l1x1,l1y1 & l2x2,l2y2
  3: l1x2,l1y2 & l2x1,l2y1
  4: l2x2,l2y2 & l2x2,l2y2
*/
int merge_2lines( Vec4f l1, Vec4f l2, double gr, double dist){
   if ( fabs(line_gradient(l1) - line_gradient(l2) ) < gr ){
     if ( X1(line_size(l1) + line_size(l2) / sqrt(distance_pow2(l1[0], l1[1], l2[0] , l2[1])), dist)) return 1 ;
     if ( X1(line_size(l1) + line_size(l2) / sqrt(distance_pow2(l1[0], l1[1], l2[2] , l2[3])), dist)) return 2 ;
     if ( X1(line_size(l1) + line_size(l2) / sqrt(distance_pow2(l1[2], l1[3], l2[0] , l2[1])), dist)) return 3 ;
     if ( X1(line_size(l1) + line_size(l2) / sqrt(distance_pow2(l1[2], l1[3], l2[2] , l2[3])), dist)) return 4 ;
     return 0;
   } else return 0;
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
