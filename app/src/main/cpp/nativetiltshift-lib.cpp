#include <jni.h>
#include <string>
#include <math.h>
#include <arm_neon.h>

using namespace std;

extern "C" {

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
    jint p_GG = (p_new<<8) & 0xff;
    jint p_RR = (p_new<<16) & 0xff;
    jint p_AA = (p_new<<24) & 0xff;
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    if(first) {
        for (jint i = r * (-1); i <= r; i++) {
            float gau = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_CalculateGaussianVector(env, This, i, sigma);
            jint p = Java_meteor_asu_edu_speedytiltshift_SpeedyTiltShift_getPoint(env, This, x, y+i, pixels_, width, height);
            jint BB = p & 0xff;
            jint GG = (p << 8) & 0xff;
            jint RR = (p << 16) & 0xff;
            jint AA = (p << 24) & 0xff;
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
            jint GG = (p << 8) & 0xff;
            jint RR = (p << 16) & 0xff;
            jint AA = (p << 24) & 0xff;
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