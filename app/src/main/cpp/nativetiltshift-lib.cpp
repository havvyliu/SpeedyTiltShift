#include <jni.h>
#include <string>
#include <math.h>
#include <arm_neon.h>
using namespace std;

JNIEXPORT jfloat JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_CalculateGaussianKernel(JNIEnv *env, jobject This, jint x, jint y, jfloat sigma) {
    return (float) (1.00/(2.00*M_PI*sigma*sigma) * exp((-1.00)*(x*x + y*y)/(2.00*sigma*sigma)));
}

extern "C" {
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

    jint r_far =(int) ceil(3*s_far);
    jint r_near = (int) ceil(3*s_near);

    // traverse through entire pixels array
    for(int x=0; x<width; x++) {
        for(int y=0; y<height; y++) {
            if(y <= a0) {
//                    Apply guassian blur(through weighted sum
                if (s_far >= 0.7) {
//                    CalculateGaussianKernel(width, a0+r_far, s_far);
//                    pixels[y * width + x] = Weight_matrix(r_far, y, x, pixels, width, s_far, height);
                }
            }
            else if(y <= a1) {
////                   Apply guassian blur(through weighted sum
                float sigma = s_far*(a1-y)/(a1-a0);
                if (sigma >= 0.7) {
//                    CalculateGaussianKernel(width, height, sigma); //TODO: fixme
//                    pixels[y * width + x] = Weight_matrix(r_far, y, x, pixels, width, sigma, height);
                }
            }
            else if(y <= a2) {
//                    No nlur
            }
            else if(y <= a3) {
                float sigma = s_far*(y-a2)/(a3-a2);
                if(sigma >= 0.7) {
//                    CalculateGaussianKernel(width, height, sigma);
//                    pixels[y * width + x] = Weight_matrix(r_near, y, x, pixels, width, sigma, height);
                }
            }
            else {
//                     Apply guassian blur(through weighted sum
                if (s_near >= 0.7) {
//                    CalculateGaussianKernel(width, height, s_near);
//                    pixels[y * width + x] = Weight_matrix(r_near, y, x, pixels, width, s_near, height);
                }
            }
        }
    }

    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);
    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return pixelsOut;
}

JNIEXPORT jintArray JNICALL
Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_nativeTiltShiftNeon(JNIEnv *env,
                                                                    jobject This,
                                                                    jintArray pixels_,
                                                                    jint width, jint height,
                                                                    jint a0, jint a1, jint a2, jint a3, jfloat s_far, jfloat s_near
) {
    int32x4_t sum_vec = vdupq_n_s32(0);
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    long length = env->GetArrayLength(pixels_);
    jintArray pixelsOut = env->NewIntArray(length);

    env->SetIntArrayRegion(pixelsOut, 0, length, pixels);
    env->ReleaseIntArrayElements(pixels_, pixels, 0);
    return pixelsOut;
}
}