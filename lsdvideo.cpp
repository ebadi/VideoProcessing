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
    Mat empty(refS.height, refS.width, CV_8UC3, Scalar(0,0,0)); // empty image
    while(1){
        if(state == STATE_PLAYING || state == STATE_RECORDING || state == STATE_NEXTFRAME){
          empty = Mat::zeros(refS.height, refS.width, CV_8UC3);
          frameNum++;
          do captRefrnc >> frame;
             while(frame.empty()) ; //some frames may be empty
          cvtColor(frame, gray, CV_BGR2GRAY);
          ls = createLineSegmentDetector(LSD_REFINE_STD);
          ls->detect(gray, lines_lsd);
          ls->drawSegments(empty, lines_lsd);
          waitKey(1); // Don't remove this.
          imshow(WIN_RF, empty);
          setMouseCallback(WIN_RF, CallBackFunc, NULL);

          if (state == STATE_NEXTFRAME) state = STATE_EDITING;
        } else { // (state == STATE_EDITING )
            key = waitKey(100); // read command
            switch(key){
            case 'q': return 0 ; //quit
            case 'l': // add a line
                cout << "Add" <<endl ;
                break;
            case 'r': // remove lines
                cout << "Remove" <<endl ;
                // TODO : unwrap vector<Vec4f> , edit and wrap
                // probably using push_back ? 

                break;
            case 's': //save, next frame

                cout << "Recorded" << endl ;
                fh << "frame:" << frameNum << endl;
                fh << "lines:" << N << endl;

                InputArray lines = lines_lsd ;
                _lines = lines.getMat();
                N = _lines.checkVector(4);
                for(int i = 0; i < N; ++i)
                {
                    const Vec4f& v = _lines.at<Vec4f>(i);
                    fh <<  v[0] << ","<<  v[1] << ","<<  v[2] << ","<<  v[3] << endl;
                }
                state == STATE_NEXTFRAME ;
                break;
             }
         }

    }
    fh.close();
    return 0;
}


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
switch( event ){
    case CV_EVENT_LBUTTONDOWN:
        cout << "CV_EVENT_LBUTTONDOWN"<<endl ;
        if (state == STATE_EDITING) {
          state = STATE_PLAYING ;
        }else {
          state = STATE_EDITING ;
        }
        break;
    //case CV_EVENT_RBUTTONUP:
    case CV_EVENT_RBUTTONDOWN:
        cout << "CV_EVENT_RBUTTONDOWN" <<endl;
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
