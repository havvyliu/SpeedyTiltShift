#include <jni.h>
#include <string>
#include <math.h>
//#include <arm_neon.h>
#include "NEON_2_SSE.h"
//#if defined(HAVE_NEON) && defined(HAVE_NEON_X86)
//#include "NENO_2_SSE.h"
//#elif defined(HAVE_NEON)
//#include <arm_neon.h>
//#endif

using namespace std;

extern "C" {

void SpeedyTiltShift_WeightMatrixThroughVector(){

}

// function to calculate the gaussian vector value
JNIEXPORT jfloat JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_CalculateGaussianVector(JNIEnv *env, jobject This, jint r, jfloat sigma) {
    return (float) (1.00/sqrt(2.00*M_PI*sigma*sigma) * exp((-1.00)*(r*r)/(2.00*sigma*sigma)));
}

JNIEXPORT jint JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_getPoint(JNIEnv *env, jobject This, jint x, jint y, jintArray pixels_, jint width, jint height) {
    jint index = y*width+x;
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    if(y < height && y >= 0 && x >= 0 && x < width) return pixels[index];
    else return 0;
}


JNIEXPORT jint JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_WeightMatrixThroughVector(JNIEnv *env, jobject This, jint r, jint y, jint x, jintArray pixels_, jint width, jfloat sigma, jint height, jboolean first) {
    jint p_new = 0;
    jint p_BB = p_new & 0xff;
    jint p_GG = (p_new>>8) & 0xff;
    jint p_RR = (p_new>>16) & 0xff;
    jint p_AA = (p_new<<24) & 0xff;
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    if(first) {
        for (jint i = r * (-1); i <= r; i++) {
            float gau = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_CalculateGaussianVector(env, This, i, sigma);
            jint p = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_getPoint(env, This, x, y+i, pixels_, width, height);
            jint BB = p & 0xff;
            jint GG = (p >> 8) & 0xff;
            jint RR = (p >> 16) & 0xff;
            jint AA = (p >> 24) & 0xff;
            p_BB += BB * gau;
            p_GG += GG * gau;
            p_RR += RR * gau;
            p_AA += AA * gau;
        }
    }
    else {
        for (jint i = r * (-1); i <= r; i++) {
            jfloat gau = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_CalculateGaussianVector(env, This, i, sigma);
            jint p = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_getPoint(env, This, x+i, y, pixels_, width, height);
            jint BB = p & 0xff;
            jint GG = (p >> 8) & 0xff;
            jint RR = (p >> 16) & 0xff;
            jint AA = (p >> 24) & 0xff;
            p_BB += BB * gau;
            p_GG += GG * gau;
            p_RR += RR * gau;
            p_AA += AA * gau;
        }
    }
    p_new = (p_AA & 0xff) << 24 | (p_RR & 0xff) << 16 | (p_GG & 0xff) << 8 | (p_BB & 0xff);
    return p_new;
}


JNIEXPORT jintArray JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_nativeTiltShift(JNIEnv *env,
                                                                    jobject This,
                                                                    jintArray pixels_,
                                                                    jint width, jint height,
                                                                    jint a0, jint a1, jint a2, jint a3, jfloat s_far, jfloat s_near
                                                                    ) {
    int32x4_t sum_vec = vdupq_n_s32(0);
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);
    jintArray pixelsOut = env->NewIntArray(length);

    jint r_far =(jint) ceil(3*s_far);
    jint r_near = (jint) ceil(3*s_near);

    // traverse through entire pixels array
    jint counter = 0;
    jboolean first = 1;
    while(counter!=2) {
        for (jint x = 0; x < width; x++) {
            for (jint y = 0; y < height; y++) {
                if (y <= a0) {
                    if (s_far >= 0.7) {
                        pixels[y * width + x] = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_WeightMatrixThroughVector(env, This, r_far, y, x, pixels_, width, s_far, height, first);
                    }
                } else if (y <= a1) {
                    float sigma = s_far * (a1 - y) / (a1 - a0);
                    if (sigma >= 0.7) {
                        pixels[y * width + x] = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_WeightMatrixThroughVector(env, This, r_far, y, x, pixels_, width, sigma, height, first);
                    }
                } else if (y <= a2) {
                    //                    No nlur
                } else if (y <= a3) {
                    jfloat sigma = s_far * (y - a2) / (a3 - a2);
                    if (sigma >= 0.7) {
                        pixels[y * width + x] = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_WeightMatrixThroughVector(env, This, r_near, y, x, pixels_, width, sigma, height, first);
                    }
                } else {
                    //                     Apply guassian blur(through weighted sum
                    if (s_near >= 0.7) {
                        pixels[y * width + x] = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_WeightMatrixThroughVector(env, This, r_near, y, x, pixels_, width, s_near, height, first);
                    }
                }
            }
        }
        counter++;
        first = 0;
    }

    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);
    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return pixels_;
}

JNIEXPORT jintArray JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_nativeTiltShiftNeon(JNIEnv *env,
                                                                    jobject This,
                                                                    jintArray pixels_,
                                                                    jint width, jint height,
                                                                    jint a0, jint a1, jint a2, jint a3, jfloat s_far, jfloat s_near
) {

    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);

    pixels = env->GetIntArrayElements(pixels_, NULL);
    jintArray pixelsOut = env->NewIntArray(length);
    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);

    int32_t arrayIn[length];





    for(int i=0;i<length; i++){
        arrayIn[i]=pixels[i];
    }


    jint *h=new jint[length];//store result

    for(int i=0;i<length; i++){
        h[i]=pixels[i];
    }


    int arrlen=length/4;

    jfloat sig=0;


    for(int j=0;j<length;j++){

        float32x4_t sum_vecA = vdupq_n_f32(0);
        float32x4_t sum_vecR = vdupq_n_f32(0);
        float32x4_t sum_vecG = vdupq_n_f32(0);
        float32x4_t sum_vecB = vdupq_n_f32(0);


        if(j<=(a0*width)){
            sig=s_far;
            if(sig<0.7){
                continue;
            }
        }

        if((a0*width)<j&&j<=(a1*width)){
            int len=floor(j/width);
            sig=s_far*((a1-len)/(a1-a0));
            if(sig<0.7){
                continue;
            }
        }

        if((a1*width)<j&&j<=(a2*width)){
            continue;
        }

        if((a2*width)<j&&j<=(a3*width)){
            int len=floor(j/width);
            sig=s_near*((len-a2)/(a3-a2));
            if(sig<0.7){
                continue;
            }
        }

        if((a3*width)<j){
            sig=s_near;
            if(sig<0.7){
                continue;
            }
        }

        int r=ceil(3*sig);
        int size=(2*r+1)*(2*r+1);


        float32_t weightlane[2*r+1];

        for(int i=0;i<2*r+1;i++){
            float ii=(float)i;
            float rr=(float)r;
            weightlane[i]=(float) exp(-(ii-rr)*(ii-rr)/(2*sig*sig))/sqrt(6.28*sig*sig);
        }// construct lane weight matrix



        float32_t weight[size];
        float32_t imageA[size];
        float32_t imageR[size];
        float32_t imageG[size];
        float32_t imageB[size];



        for(int m=0;m<2*r+1;m++){
            for(int n=0;n<2*r+1;n++){
                float mf=(float) m-r;
                float nf=(float) n-r;
                weight[m*(2*r+1)+n]=exp(-(mf*mf+nf*nf)/(2*sig*sig))/(2*3.14*sig*sig);
            }
        }//construct weight matrix

        float32_t * weightPtr=(float32_t *)weight;

        int loc_x=j%width;
        int loc_y=floor(j/width);



        for(int n=0;n<2*r+1;n++){
            for(int m=0;m<2*r+1;m++){
                if(loc_x-r+m<0||loc_x-r+m>width) {

                        imageA[n*(2*r+1)+m]=0;

                        imageR[n*(2*r+1)+m]=0;
                        imageG[n*(2*r+1)+m]=0;
                        imageB[n*(2*r+1)+m]=0;

                }else{
                    if(loc_y-r+n<0||loc_y-r+n>height){

                        imageA[n*(2*r+1)+m]=0;
                        imageR[n*(2*r+1)+m]=0;
                        imageG[n*(2*r+1)+m]=0;
                        imageB[n*(2*r+1)+m]=0;
                    }else{
                        imageA[n*(2*r+1)+m]=(float)((arrayIn[(loc_y-r+n)*width+(loc_x-r+m)]>>24)&0xff);
                        imageR[n*(2*r+1)+m]=(float)((arrayIn[(loc_y-r+n)*width+(loc_x-r+m)]>>16)&0xff);
                        imageG[n*(2*r+1)+m]=(float)((arrayIn[(loc_y-r+n)*width+(loc_x-r+m)]>>8)&0xff);
                        imageB[n*(2*r+1)+m]=(float)((arrayIn[(loc_y-r+n)*width+(loc_x-r+m)])&0xff);
                    }

                }


            }

        }//construct image matrix

        float32_t * imageAPtr=(float32_t *)imageA;
        float32_t * imageRPtr=(float32_t *)imageR;
        float32_t * imageGPtr=(float32_t *)imageG;
        float32_t * imageBPtr=(float32_t *)imageB;


        for(int i=0;i<(size/4);i++){
            float32x4_t Avec = vld1q_f32(imageAPtr);
            float32x4_t Rvec = vld1q_f32(imageRPtr);
            float32x4_t Gvec = vld1q_f32(imageGPtr);
            float32x4_t Bvec = vld1q_f32(imageBPtr);
            float32x4_t wvec = vld1q_f32(weightPtr);



           sum_vecA = vmlaq_f32(sum_vecA,Avec,wvec);
           sum_vecR = vmlaq_f32(sum_vecR,Rvec,wvec);
           sum_vecG = vmlaq_f32(sum_vecG,Gvec,wvec);
           sum_vecB = vmlaq_f32(sum_vecB,Bvec,wvec);
            weightPtr +=4;
            imageAPtr += 4;
            imageRPtr += 4;
            imageGPtr += 4;
            imageBPtr += 4;
        }

        float sumA=0;
        float sumR=0;
        float sumG=0;
        float sumB=0;

        sumA +=vgetq_lane_f32(sum_vecA,0);
        sumA +=vgetq_lane_f32(sum_vecA,1);
        sumA +=vgetq_lane_f32(sum_vecA,2);
        sumA +=vgetq_lane_f32(sum_vecA,3);


        sumR +=vgetq_lane_f32(sum_vecR,0);
        sumR +=vgetq_lane_f32(sum_vecR,1);
        sumR +=vgetq_lane_f32(sum_vecR,2);
        sumR +=vgetq_lane_f32(sum_vecR,3);

        sumG +=vgetq_lane_f32(sum_vecG,0);
        sumG +=vgetq_lane_f32(sum_vecG,1);
        sumG +=vgetq_lane_f32(sum_vecG,2);
        sumG +=vgetq_lane_f32(sum_vecG,3);

        sumB +=vgetq_lane_f32(sum_vecB,0);
        sumB +=vgetq_lane_f32(sum_vecB,1);
        sumB +=vgetq_lane_f32(sum_vecB,2);
        sumB +=vgetq_lane_f32(sum_vecB,3);


        uint8_t finalA=(uint8_t)sumA;
        uint8_t finalR=(uint8_t)sumR;
        uint8_t finalG=(uint8_t)sumG;
        uint8_t finalB=(uint8_t)sumB;

        h[j]=(finalA&0xff)<<24|(finalR&0xff)<<16|(finalG&0xff)<<8|(finalB&0xff);

    }




   env->SetIntArrayRegion(pixelsOut,0,length,h);
    return pixelsOut;

}
}
