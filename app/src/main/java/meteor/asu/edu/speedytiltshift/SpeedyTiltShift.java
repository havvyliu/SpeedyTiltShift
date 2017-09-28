package meteor.asu.edu.speedytiltshift;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

/**
 * Created by roblkw on 7/26/17.
 */

public class SpeedyTiltShift {

    static {
        System.loadLibrary("nativetiltshift-lib");
    }

    private static float[][] GaussianKernel;

    //     Calculate the guassian matrix in advance
    public static void CalculateGaussianKernel (int x, int y, float sigma) {
        GaussianKernel = new float[x][y];
        for(int i=0; i<x; i++) {
            for(int j=0; i<y; j++) {
                GaussianKernel[i][j] =(float) (1.00/(2.00*Math.PI*sigma*sigma) * (Math.pow(Math.E, (-1.00)*(x*x + y*y)/(2.00*sigma*sigma))));
            }
        }
    }

    public static int getPoint(int x, int y, int[] pixels, int width, int height) {
        int index = y*width+x;
        if(y < height && y >= 0 && x >= 0 && x < width) return pixels[index];
        else return 0;
    }

//    Gaussian blur using weight matrix approach
    public static int Weight_matrix (int r, int y, int x, int[] pixels, int width, float sigma, int height) {
        int p_new = 0;
        int p_BB = p_new & 0xff;
        int p_GG = (p_new<<8) & 0xff;
        int p_RR = (p_new<<16) & 0xff;
        int p_AA = (p_new<<24) & 0xff;
        int length=pixels.length;
        int height_1=length/width;
        for( int i=r*(-1); i<=r; i++){
            if(x+i>=0 && x+i<width){
                for( int j=r*(-1); j<=r; j++){
                    if(y+j>=0 && y+j<height_1){
                        int p = getPoint(x+i, y+j, pixels, width, height);
                        float gau = GaussianKernel[x+i][y+j];
                        int BB = p & 0xff;
                        int GG = (p<<8) & 0xff;
                        int RR = (p<<16) & 0xff;
                        int AA = (p<<24) & 0xff;
                        p_BB += BB*gau;
                        p_GG += GG*gau;
                        p_RR += RR*gau;
                        p_AA += AA*gau;
                    }
                }
            }
        }
        p_new = (p_AA & 0xff) << 24 | (p_RR & 0xff) << 16 | (p_GG & 0xff) << 8 | (p_BB & 0xff);


        return p_new;
    }

    public static Bitmap tiltshift_java(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        Bitmap out;
        out=in.copy(in.getConfig(),true);

        int width=in.getWidth();
        int height=in.getHeight();

        int r_far =(int) Math.ceil(3*s_far);
        int r_near = (int) Math.ceil(3*s_near);



        Log.d("TILTSHIFT_JAVA","hey:"+width+","+height);
        int[] pixels = new int[width*height];


        int offset=0;
        int stride = width;
        in.getPixels(pixels,offset,stride,0,0,width,height);
        for (int y=0; y<height; y++){
            for (int x = 0; x<width; x++){
//                // From Google Developer: int color = (A & 0xff) << 24 | (R & 0xff) << 16 | (G & 0xff) << 16 | (B & 0xff);
//                int p = pixels[y*width+x];
//                int BB = p & 0xff;
//                int GG = (p<<8)& 0xff;
//                int RR = 0xff;//(p<<16)& 0xff; //set red high
//                int AA = (p<<24)& 0xff;
//                int color = (AA & 0xff) << 24 | (RR & 0xff) << 16 | (GG & 0xff) << 8 | (BB & 0xff);
//                pixels[y*width+x] = color;

//                Divide by height and apply according procedure
                if(y <= a0) {
//                    Apply guassian blur(through weighted sum
                    if (s_far >= 0.7) {
                        CalculateGaussianKernel(width, a0+r_far, s_far);
                        pixels[y * width + x] = Weight_matrix(r_far, y, x, pixels, width, s_far, height);
                    }
                }
                else if(y <= a1) {
////                   Apply guassian blur(through weighted sum
                    float sigma = s_far*(a1-y)/(a1-a0);
                    if (sigma >= 0.7) {
                        CalculateGaussianKernel(width, height, sigma); //TODO: fixme
                        pixels[y * width + x] = Weight_matrix(r_far, y, x, pixels, width, sigma, height);
                    }
                }
                else if(y <= a2) {
//                    No nlur
                }
                else if(y <= a3) {
                    float sigma = s_far*(y-a2)/(a3-a2);
                    if(sigma >= 0.7) {
                        CalculateGaussianKernel(width, height, sigma);
                        pixels[y * width + x] = Weight_matrix(r_near, y, x, pixels, width, sigma, height);
                    }
                }
                else {
//                     Apply guassian blur(through weighted sum
                    if (s_near >= 0.7) {
                        CalculateGaussianKernel(width, height, s_near);
                        pixels[y * width + x] = Weight_matrix(r_near, y, x, pixels, width, s_near, height);
                    }
                }
            }
        }
        out.setPixels(pixels,offset,stride,0,0,width,height);

//        Log.d("TILTSHIFT_JAVA","hey2");
        return out;
    }
    public static Bitmap tiltshift_cpp(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        int width = in.getWidth();
        int height = in.getHeight();
        int[] pixels = new int[width*height];
        int[] outpixels = nativeTiltShift(pixels, width, height, a0, a1, a2, a3, s_far, s_near);
        Bitmap out = in.copy(in.getConfig(), true);
        out.setPixels(outpixels, 0, width, 0, 0, width, height);
        return in;
    }
    public static Bitmap tiltshift_neon(Bitmap in, int a0, int a1, int a2, int a3, float s_far, float s_near){
        return in;
    }


    private static native int[] nativeTiltShift(int[] pixels, int imgW, int imgH, int a0, int a1, int a2, int a3, float s_far, float s_near);
    private static native int[] nativeTiltShiftNeon(int[] pixels, int imgW, int imgH, int a0, int a1, int a2, int a3, float s_far, float s_near);

}
