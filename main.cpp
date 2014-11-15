#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <stdio.h>
#include <string.h>


#define COEF_SIZE 32
#define N 32
#define PI 3.1415926
#define LEN 64

double dctcoef1[N][N];
double dctcoef2[N][N];

void init_coef()
{
    double coef[N];

    for(int i = 0; i < N; i++)
        coef[i] = 1;

    coef[0] = 1/sqrt(2.0);

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++)
        {
            dctcoef1[i][j] = cos((2*i+1)/(2.0*N)*j*PI);
            dctcoef2[i][j] = coef[i]*coef[j]/4.0;
        }
    }
}


void dct(double** data, double** dst)
{
    for(int u = 0; u < N; u++)
    {
        for(int v = 0; v < N; v++)
        {
            double sum = 0.0;

            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++)
                    sum += dctcoef1[i][u]*dctcoef1[j][v]*data[i][j];

            dst[u][v] = sum*dctcoef2[u][v];
        }
    }
}


void gen_hash_code(cv::Mat& img, char* hashCode)
{
    cv::Mat gray;

    if(img.channels()==3){
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    }
    else if(img.channels()==1)
        gray = img.clone();

    else {
        printf("Not normal image\n");
        exit(-1);
    }

    cv::Mat smallImg;
    cv::resize(gray, smallImg, cv::Size(N, N), cv::INTER_CUBIC);

    double **data = new double*[N];
    for(int i = 0; i < N; i++)
        data[i] = new double[N];

    for(int x = 0; x < smallImg.cols; x++)
        for(int y = 0; y < smallImg.rows; y++)
            data[x][y] = smallImg.at<uchar>(y, x);


    double **dctres = new double*[N];

    for(int i = 0; i < N; i++)
        dctres[i] = new double[N];

    dct(data, dctres);

    double total = 0;
    for(int x = 0; x < 8; x++)
        for(int y = 0; y < 8; y++)
            total += dctres[x][y];

    total -= dctres[0][0];
    double avg = total/63;

    int count = 0;
    for(int x = 0; x < 8; x++)
        for(int y = 0; y < 8; y++)
            if(x!=0 && y!=0)
                hashCode[count++] = (dctres[x][y]>avg?'1':'0');

    hashCode[count] = '\0';


    for(int i = 0; i < N; i++){
        delete [] dctres[i];
        delete [] data[i];
    }

    delete [] dctres;
    delete [] data;
}


int distance(char *hashCode1, char* hashCode2)
{
    int len = 0;

    for(int i = 0; i < LEN; i++)
        if(hashCode1[i]!=hashCode2[i])
            len++;

    return len;
}


int thresh = 0;


int main(int argc, char** argv)
{
    if(argc<2)
    {
        printf("Usage:%s [video] [thresh=10]\n", argv[0]);
        return 0;
    }
    if(argc==3)
    {
        thresh = atoi(argv[2]);
    }

    cv::VideoCapture cap(argv[1]);
    if(!cap.isOpened())
    {
        printf("Open video error\n");
        return -1;
    }

    long totalFrameNumber = long(cap.get(CV_CAP_PROP_FRAME_COUNT));

    init_coef();

    char *hc1 = new char[LEN];
    char *hc2 = new char[LEN];

    cv::Mat frontFrame, frame;

       cap>>frontFrame;

    gen_hash_code(frontFrame, hc1);

    for(long frameNumber = 1; frameNumber < totalFrameNumber; frameNumber++){

        cap>>frame;

        gen_hash_code(frame, hc2);

        if(distance(hc1, hc2)==11)
        {
            char fileName[30];
            sprintf(fileName, "img/%ld.jpg", frameNumber);
            cv::imwrite(fileName, frontFrame);
            sprintf(fileName, "img/%ld.jpg", frameNumber+1);
            cv::imwrite(fileName, frame);
        }
        printf("%7.2lf%% \r", 1.0*frameNumber/totalFrameNumber*100);
//        printf("%ld %s %s %d\n", frameNumber, hc1, hc2, distance(hc1, hc2));

        char *p = hc1;

        hc1 = hc2;
        hc2 = p;
        frontFrame = frame.clone();
    }

    printf("\n");
    delete [] hc1;
    delete [] hc2;
    return 0;
}
