// play-pause

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <windows.h>

using namespace cv;
using namespace std;

#define CONTOUR_MAX 20			// maximum number of contours
#define MIN_AREA 1000			// minimum contour area
Point screen_resolution(1366,768);		// resolution of the desktop screen

void segment(Mat,Mat);
int cursor_state(Mat,vector<Point>);				// gives the possition of cursor
void cursor_action(int);

double dWidth;
double dHeight;
char confK;						// 0 means top right configuration
char ini;						// represent the initiation of program	
int cur_speed = 8;				// speed of the cursor
Point cur_pos[2];					// store relative coordinates of cursor
char click_arr[3];

int main()  {

	// opening video stream
    VideoCapture cap; 
		//cap.open("http://192.168.0.107:8080/video?x.mjpeg");		// het the feed from IP camera
	cap.open(0);
    if (!cap.isOpened()) {						
        cout << "Cannot open the video cam" << endl;
		return -1;   
	}
   dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);			//get the width of frames of the video
   dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);			//get the height of frames of the video

	
   cout<<"For calibration press: c\nTo commit it again press: c\n\n";
	cout<<"To change hand detection configuration press: t\n Top Right <=> Top Left\n\n";
	char key,Tkey=0;		// 0 => normal flow
	pair<int,int> seg_value[3];			// HSV min, max values for coloue segmentation
		seg_value[0].first=0; seg_value[1].first=42; seg_value[2].first=5;
		seg_value[0].second=61; seg_value[1].second=150; seg_value[2].second=99;
	int erode_value=4,dilate_value=6,blur_value=14,th_value=109;

    Mat Finput,Fhsv,Fthre;
    while (1)
    {
        if (!cap.read(Finput)) {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }
		flip(Finput,Finput,180);			// To correct perspective order
		//GaussianBlur(Finput,Finput,Size(3,3),0);

		//    /*     Hand Segmentation				 
		cvtColor(Finput,Fhsv,CV_BGR2HSV);

		inRange(Fhsv,Scalar(seg_value[0].first,seg_value[1].first,seg_value[2].first),
			Scalar(seg_value[0].second,seg_value[1].second,seg_value[2].second),Fthre);

		erode(Fthre,Fthre,getStructuringElement(MORPH_RECT,Size(erode_value+1,erode_value+1)));
		dilate(Fthre,Fthre,getStructuringElement(MORPH_RECT,Size(dilate_value+1,dilate_value+1)));
		blur(Fthre, Fthre, Size(blur_value+1, blur_value+1), Point(-1, -1), BORDER_DEFAULT);
		threshold(Fthre, Fthre, th_value, 255, THRESH_BINARY);

		if(Tkey == 1) {                       // ***
			imshow("Segmentation",Fthre);				
		} else {
			segment(Fthre,Finput);

		}
	

        if ((key=waitKey(10)) == 27) {
            cout << "Closing gesture control.." << endl;
            break;
        } else if(key == 'c' || key == 'C') {
				// Give interface for calibration
			if(Tkey ==0 ) {
				namedWindow("Segmentation",WINDOW_AUTOSIZE);
				namedWindow("Trackbar",WINDOW_FREERATIO);
				createTrackbar("H_min","Trackbar",&seg_value[0].first,255);
				createTrackbar("H_max","Trackbar",&seg_value[0].second,255);
				createTrackbar("S_min","Trackbar",&seg_value[1].first,255);
				createTrackbar("S_max","Trackbar",&seg_value[1].second,255);
				createTrackbar("V_min","Trackbar",&seg_value[2].first,255);
				createTrackbar("V_max","Trackbar",&seg_value[2].second,255);
				createTrackbar("erode","Trackbar",&erode_value,15);
				createTrackbar("dilate","Trackbar",&dilate_value,15);
				createTrackbar("blur","Trackbar",&blur_value,150);
				createTrackbar("thresh","Trackbar",&th_value,255);
			} else {
				destroyWindow("Trackbar");
				destroyWindow("Segmentation");
				ini=0;
			}
			Tkey = !Tkey;
		} else if(key == 'T' || key == 't') {
				// change hand detection configuration
			if(confK==0) {
				putText(Finput,"Configration changed: Top Left",Point((int)dWidth/5,(int)dHeight/4),1,1,Scalar(255,0,0),2);
			}else {
				putText(Finput,"Configration changed: Top Right",Point((int)dWidth/5,(int)dHeight/4),1,1,Scalar(255,0,0),2);
			}
			confK = !confK;
		}

				imshow("MyVideo", Finput); 
		if(key == 'T' || key == 't') {
			waitKey(500);
		}
    }

    return 0;
}


void segment(Mat Fthre, Mat Finput) {
	vector<vector<Point> > contours;
	Moments m;
	int index;
	Point cTemp = Point(0,0),cursor;
	findContours(Fthre,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
					
						// no contour detected or lot of contours detected
	if(contours.size() == 0 || contours.size() > CONTOUR_MAX) {
		putText(Finput, "FILTER NOT ADJUSTED PROPERLY PRESS: C To ADJUST", Point((int)dWidth/20,(int)dHeight/20), 1, 1, Scalar(0, 0, 255), 2);
		return ; 
	} else {		
		for(int i=0;i<contours.size();++i) {
			m = moments(contours[i]);
			if(m.m00 < MIN_AREA)
				continue;
			cursor.x = int(m.m10/m.m00);
			cursor.y = int(m.m01/m.m00);
			if(confK) {
				// Top left
				if(cursor.x<cTemp.x || cTemp.x==0){
					cTemp = cursor;
					index = i;
				}
			} else {
				// Top right
				if(cursor.x>cTemp.x) {
					cTemp = cursor;
					index = i;
				}
			}
		}
			cursor = cTemp;
			drawContours(Finput,contours,index,Scalar(255,0,0),2);		
		//	circle(Finput,cursor,0,Scalar(0,0,255),4);
	}


	int state;
	state = cursor_state(Finput,contours[index]);
	cursor_action(state);
}


int cursor_state(Mat Finput,vector<Point> contour) {
	vector<int> Ihull;
	vector<Vec4i> defects;
	convexHull(contour,Ihull,false);
	convexityDefects(contour,Ihull,defects);

/*
	for(int i=0;i<Ihull.size();++i) {
		circle(Finput,contour[Ihull[i]],0,Scalar(0,255,0),4);
	}
*/
	int cnt=0;
	for(int i=0;i<defects.size();++i) {	
		if(int(dHeight*0.06) < defects[i][3]/255) {
			++cnt;
		}
			//circle(Finput,contour[defects[i][2]],0,Scalar(0,0,255),5);
			//circle(Finput,contour[defects[0][i][0]],0,Scalar(0,0,255),5);
			//circle(Finput,contour[defects[0][i][2]], 0,Scalar(0,255,255),5);
	}

	if(cnt == 5) return 1;
	else return 0;
	
}




void cursor_action(int action) {

	if(action==1) {
		keybd_event(VkKeyScan(' '),0,0,0);
		keybd_event(VkKeyScan(' '),0,KEYEVENTF_KEYUP,0);
	} 
}


