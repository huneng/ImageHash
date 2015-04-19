#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <stdio.h>
#include <string.h>


#define COEF_SIZE 32
#define N 32
#define N2 8
#define PI 3.1415926
#define LEN 64

#define SHOW_IMAGE(x) {cv::imshow(#x, x); cv::waitKey();}

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
            dctcoef1[i][j] = cos( (2 * i + 1) / (2.0 * N) * j * PI);
            dctcoef2[i][j] = coef[i] * coef[j]/4.0;
        }
    }
}


void dct(double *data, double *dst)
{
    for(int u = 0; u < N2; u++)
    {
        for(int v = 0; v < N2; v++)
        {
            double sum = 0.0;

            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++)
                    sum += dctcoef1[i][u] * dctcoef1[j][v] * data[i * N + j];

            dst[u * N2 + v] = sum * dctcoef2[u][v];
        }
    }
}


void gen_hash_code(cv::Mat& img, uint32_t* hashCode)
{
    cv::Mat gray, smallImg;

    double *data, *dctres;
    double avg = 0;

    int y, x;
    int i;

    if(img.type() == CV_8UC3)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    else if(img.type() == CV_8UC1)
        gray = img.clone();

    cv::resize(gray, smallImg, cv::Size(N, N));

    data = new double[N * N];

    for(y = 0; y < smallImg.rows; y++)
        for(x = 0; x < smallImg.cols; x++)
            data[y * smallImg.cols + x] = smallImg.at<uchar>(y, x);


    dctres = new double[N2 * N2];

    dct(data, dctres);

    int size = N2 * N2;

    for(int i = 1; i < size; i++){
        avg += dctres[i];
    }

    avg = avg/(size - 1);

    uint32_t hashZero = 0x00000000;
    uint32_t hashOne  = 0x00000001;
    uint32_t code;

    int half = size >> 1;

    for(int i = 1; i < half ; i++)
    {
        code = dctres[i] > avg;
        hashZero <<= 1;
        hashZero |= code;
    }

    hashCode[0] = hashZero;

    hashZero = 0x00000000;

    for(i = half; i < size; i++)
    {
        code = dctres[i] > avg;
        hashZero <<= 1;
        hashZero |= code;
    }

    hashCode[1] = hashZero;

    delete [] dctres;
    delete [] data;
}

int hamdist(uint32_t *a, uint32_t *b, int len);
void print_bin(uint32_t code);


void read_image_list(const char *fileName, std::vector<std::string>& imageList);

int main_single_test(int argc, char **argv);
int main_multi_test(int argc, char ** argv);

int main(int argc, char **argv)
{
    /*
    //input arguments is image list file and output file
    main_multi_test(argc, argv);
    */

    //input arguments is two image used to matching
    main_single_test(argc, argv);
    return 0;
}


int main_multi_test(int argc, char ** argv)
{
    if(argc < 3)
    {
        printf("Usage: %s[image list] [res file]\n", argv[0]);
        return 1;
    }

    std::vector<std::string> imageList;

    read_image_list(argv[1], imageList);

    FILE *fp = fopen(argv[2], "w");
    if(fp == NULL)
    {
        printf("Can't open file %s\n", argv[2]);
        return 1;
    }

    int size = imageList.size();

    uint32_t *code = new uint32_t[2 * size];

    init_coef();

    printf("Generate code\n");

    for(int i = 0; i < size; i++)
    {
        cv::Mat img = cv::imread(imageList[i], 0);
        gen_hash_code(img, code + (i << 1));

        printf("%d\r", i);
        fflush(stdout);
    }

    printf("Matching\n");

    uint32_t* pCode1 = code;

    for(int i = 0; i < size; i++, pCode1 += 2)
    {
        uint32_t* pCode2 = pCode1 + 2;

        for(int j = i+1; j < size; j++, pCode2 += 2)
        {
            int dist = hamdist(pCode1, pCode2, 2);

            fprintf(fp, "%s %s %d\n", imageList[i].c_str(), imageList[j].c_str(), dist);
        }
        printf("%d\r", i);
        fflush(stdout);
    }

    fclose(fp);

    return 0;
}


int main_single_test(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage: %s [image 1] [image 2]\n", argv[0]);
        return 1;
    }

    cv::Mat img = cv::imread(argv[1], 0);
    cv::Mat img2 = cv::imread(argv[2], 0);

    if(img.empty() || img2.empty())
    {
        printf("Can't open image error\n");
        return 1;
    }

    uint32_t code1[2], code2[2];

    init_coef();

    gen_hash_code(img, code1);
    gen_hash_code(img2, code2);

    int dist = hamdist(code1, code2, 2);

    printf("%s %s dist = %d\n", argv[1], argv[2], dist);

    return 0;
}


void read_image_list(const char *fileName, std::vector<std::string>& imageList)
{
    FILE *fp = fopen(fileName, "r");
    char line[256];

    if(fp == NULL)
    {
        printf("Can't open file %s\n", fileName);
        exit(0);
    }

    while(fscanf(fp, "%s\n", line) != EOF)
    {
        imageList.push_back(std::string(line));
    }
}


const unsigned char bit_count[256] =
{
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};


// compute Hamming Distance between two binary streams
int hamdist(uint32_t *a, uint32_t *b, int len)
{
    int i;
    int dist = 0;
    uint32_t mask = 0x000000ff;

    for(i = 0; i < len; i++)
    {
        uint32_t res = a[i] ^ b[i];

        dist += bit_count[(res >> 24) & mask] +
            bit_count[(res >> 8) & mask] +
            bit_count[res & mask];
    }

    return dist;
}


void print_bin(uint32_t code)
{
    uint32_t hashOne = 0x00000001;

    char out[33];

    for(int i = 31; i >= 0 ; i--)
    {
        out[i] = (code & hashOne) + '0';
        code >>= 1;
    }

    out[32] = '\0';
    printf("%s ", out);
}
