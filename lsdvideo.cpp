#include <iostream> // for standard I/O
#include <sstream>  // string to number conversion

#include <opencv2/core/core.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp> 

using namespace std;
using namespace cv;

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

int main(int argc, char *argv[])
{
    help();

    if (argc != 3)
    {
        cout << "Not enough parameters" << endl;
        return -1;
    }

    stringstream conv;

    const string sourceReference = argv[1];
    int delay;
    conv << argv[2]; 
    conv >> delay;

    char c;
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
    moveWindow(WIN_RF, 400       , 0);         //750,  2 (bernat =0)

    Mat frame;
    Mat gray;

    for(;;)
    {
        frameNum++;
        do {
             captRefrnc >> frame;
        } while(frame.empty()) ; //some frames may be empty

        cvtColor(frame, gray, CV_BGR2GRAY);
        cout << "Frame: " << frameNum << "#" << endl;

        Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
        vector<Vec4f> lines_std;

        // Detect the lines
        ls->detect(gray, lines_std);

        Mat empty(refS.height, refS.width, CV_8UC3, Scalar(0,0,0)); //empty image
        ls->drawSegments(empty, lines_std);

        imshow(WIN_RF, empty);

        c = (char)waitKey(delay);
        if (c == 27) break;
    }
    return 0;
}

