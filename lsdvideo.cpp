#include <iostream> // for standard I/O
#include <sstream>  // string to number conversion
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

    stringstream conv;

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

    int N ;
    Mat _lines;
    vector<Vec4f> lines_lsd;
    Ptr<LineSegmentDetector> ls ;
    int minx, maxx, miny, maxy;
    Mat empty(refS.height, refS.width, CV_8UC3, Scalar(0,0,0)); // empty image
    while(1){
        if(state == STATE_PLAYING || state == STATE_RECORDING || state == STATE_NEXTFRAME){
          cout << "loading a frame" ;
          mouseX1=0;  mouseY1=0 ; mouseX2=0;  mouseY2=0;
          empty = Mat::zeros(refS.height, refS.width, CV_8UC3);
          frameNum++;
          do captRefrnc >> frame;
             while(frame.empty()) ; //some frames may be empty
          cvtColor(frame, gray, CV_BGR2GRAY);
          ls = createLineSegmentDetector(LSD_REFINE_STD);
          ls->detect(gray, lines_lsd);
          if (state == STATE_NEXTFRAME) state = STATE_EDITING;
        } else { // (state == STATE_EDITING )
            key = waitKey(100); // read command
            switch(key){
            case 'q': return 0 ; //quit
            case 'a': // add a line
                cout << "Add" <<endl ;
                lines_lsd.push_back(Vec4f(mouseX1,mouseY1, mouseX2, mouseY2)  );
                break;
            case 'r': // remove lines
                cout << "Remove" <<endl ;
                  // TODO : unwrap vector<Vec4f> , edit and wrap
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
                    }
                }
                break;
            case 's': //save, next frame
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
