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
    jintArray pixelsOut = env->NewIntArray(length);
    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);

    float32_t arrayIn[length];
    float32_t arrayOut[length];

    for(int i=0;i<length; i++){
        arrayIn[i]=pixels[i];
        arrayOut[i]=pixels[i];
    }


    int arrlen=length/4;
    jfloat sig=0;

    for(int j=0;j<arrlen;j++){
        float32x4_t sum_vecA = vdupq_n_f32(0);
        float32x4_t sum_vecR = vdupq_n_f32(0);
        float32x4_t sum_vecG = vdupq_n_f32(0);
        float32x4_t sum_vecB = vdupq_n_f32(0);
        if(j<=(a0*width/4)){
            sig=s_far;
        }

        if((a0*width/4)<j&&j<=(a1*width/4)){
            int len=floor(4*j/width);
            sig=s_far*((a1-len)/(a1-a0));
        }

        if((a1*width/4)<j&&j<=(a2*width/4)){
            break;
        }

        if((a2*width/4)<j&&j<=(a3*width/4)){
            int len=floor(4*j/width);
            sig=s_near*((len-a2)/(a3-a2));
        }

        if((a3*width/4)<j){
            sig=s_far;
        }

        int r=ceil(3*sig);
        int size=(2*r+1)*(2*r+1);
        int mid=(size-1)/2;
        float32_t weight[size];
        float32_t image[size*4];

        for(jint m=0;m<2*r+1;m++){
            for(int n=0;n<2*r+1;n++){
                weight[m*r+n]=exp(-(m*m+n*n)/(2*sig*sig))/(2*3.14*sig*sig);
            }
        }//construct weight matrix

        int loc_x=4*j%width;
        int loc_y=floor(4*j/width);
        float32_t * weightPtr=(float32_t *)weight;


        for(int m=0;m<2*r+1;m++){
            for(int n=0;n<2*r+1;n++){
                if(loc_x-4*r+4*m<0||loc_x-4*r+4*m>width) {
                    for(int a=0;a<4;a++){
                        image[n*(2*r+1)+4*m+a]=0;
                    }
                }else{
                    if(loc_y-r+n<0||loc_y-r+n>height){
                        for(int a=0;a<4;a++){
                            image[n*(2*r+1)+4*m+a]=0;
                        }
                    }
                }
            }

        }//construct image matrix

        float32_t * imagePtr=(float32_t *)image;

        for(int i=0;i<(size/4);i++){
            float32x4x4_t ivec=vld4q_f32(imagePtr);
            float32x4_t wvec=vld1q_f32(weight);
//            sum_vecA = vmla_f32(sum_vecA,ivec.val[0],wvec);
//            sum_vecR = vmla_f32(sum_vecR,ivec.val[1],wvec);
//            sum_vecG = vmla_f32(sum_vecG,ivec.val[2],wvec);
//            sum_vecB = vmla_f32(sum_vecB,ivec.val[3],wvec);
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
        sumB +=vgetq_lane_f32(sum_vecB,3);//calculate A,R,G,B distribute

        arrayOut[j*4]=sumA;
        arrayOut[j*4+1]=sumR;
        arrayOut[j*4+2]=sumG;
        arrayOut[j*4+3]=sumB;//cppy image value to arrayout
    }
    jint *h=new jint[length];
    for(int i=0;i<length;i++){
        h[i]=(jint)arrayOut[i];
    }

    env->SetIntArrayRegion(pixelsOut,0,length,h);
    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return pixelsOut;
}
}