#include <iostream> // for standard I/O
#include <sstream>  // string to number conversion
#include <fstream>
#include <opencv2/core/core.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp> 

using namespace std;
using namespace cv;
bool playing = true ; 
static void help()
{
    cout
        << "------------------------------------------------------------------------------" << endl
        << "This program applys LSD filter to a video  "
        << "Usage:"                                                                         << endl
        << "./lsd video.avi Wait_Between_Frames " << endl
        << "------------------------------------------------------------------------------" << endl
        << endl;
}
void CallBackFunc(int event, int x, int y, int flags, void* userdata) ;
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        help();
        cout << "Not enough parameters" << endl;
        return -1;
    }

    stringstream conv;

    const string sourceReference = argv[1];

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
    const char* WIN_RF = "Video";

    // Windows
    namedWindow(WIN_RF, WINDOW_AUTOSIZE);
    moveWindow(WIN_RF, 400       , 0);

    Mat frame;
    Mat gray;

    ofstream fh;
    fh.open ("out.txt");
    bool nextframe = false ;
    while(1)
    {
        nextframe = false;
        frameNum++;
        do {
             captRefrnc >> frame;
        } while(frame.empty()) ; //some frames may be empty
        cvtColor(frame, gray, CV_BGR2GRAY);

        Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
        vector<Vec4f> lines_std;
        // Detect the lines
        ls->detect(gray, lines_std);
        Mat empty(refS.height, refS.width, CV_8UC3, Scalar(0,0,0)); //empty image
        ls->drawSegments(empty, lines_std);

        InputArray lines = lines_std ;
        Mat _lines;
        _lines = lines.getMat();
        int N = _lines.checkVector(4);
        waitKey(1);
            if(playing==false )
            {
            key = waitKey(0);
            switch(key){
            case 27: return 0 ;
            case 'r':
                cout << "rrrrrr" <<endl ;
                break;
            case 'n':
                cout << "nnnnnn" <<endl ;
                break;
            case 's':
                cout << "Recorded" << endl ;
                fh << "frame:" << frameNum << endl;
                fh << "lines:" << N << endl;
                for(int i = 0; i < N; ++i)
                {
                    const Vec4f& v = _lines.at<Vec4f>(i);
                    fh <<  v[0] << ","<<  v[1] << ","<<  v[2] << ","<<  v[3] << endl;
                }
                nextframe= true ;
                break;
             }
         }
        imshow(WIN_RF, empty);
        setMouseCallback(WIN_RF, CallBackFunc, NULL);
    }
    fh.close();
    return 0;
}


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
switch( event ){
    case CV_EVENT_LBUTTONDOWN: 
        cout << "CV_EVENT_LBUTTONDOWN"<<endl ;
        playing = !playing ;
        break;
    case CV_EVENT_RBUTTONDOWN:
        cout << "CV_EVENT_RBUTTONDOWN" <<endl;
        break;
    case CV_EVENT_RBUTTONUP:
        cout << "CV_EVENT_RBUTTONUP" <<endl;
        break;
    case CV_EVENT_MBUTTONDOWN:
        cout << "CV_EVENT_MBUTTONDOWN" <<endl;
        break;
    case CV_EVENT_MBUTTONUP:
        cout << "CV_EVENT_MBUTTONUP" <<endl;
        break;
    }
}
