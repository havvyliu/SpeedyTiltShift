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


//    Gaussian Function that is used to calculate the each entry of kernel function
    public static float GaussianResult (int x, int y, float sigma) {
        float x_f = (float) x;
        float y_f = (float) y;
        float G = (float) (1.00/(2.00*Math.PI*sigma*sigma) * (Math.pow(Math.E, (-1.00)*(x_f*x_f + y_f*y_f)/(2.00*sigma*sigma))));
//        Log.d("front",String.valueOf(Math.pow(Math.E, (-1.00)*(x_f*x_f + y_f*y_f)/(2.00*sigma*sigma))));
        return G;
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
//        for(int i = r*(-1); i<= r ; i++) {
//            int p = getPoint(x-r, y+i, pixels, width, height);
//            float gau = GaussianResult(i, (-1)*r, sigma);
//            int BB = p & 0xff;
//            int GG = (p<<8) & 0xff;
//            int RR = (p<<16) & 0xff;
//            int AA = (p<<24) & 0xff;
//            p_BB += BB*gau;
//            Log.d("p_BB",String.valueOf(BB*gau));
//            p_GG += GG*gau;
//            p_RR += RR*gau;
////            Log.d("p_BB",String.valueOf(gau));
//            p_AA += AA*gau;
//
////            p_new = (p_AA & 0xff) << 24 | (p_RR & 0xff) << 16 | (p_GG & 0xff) << 8 | (p_BB & 0xff);
////            p_new +=(int) (GaussianResult(i, (-1)*r, sigma)*getPoint(x-r, y+i, pixels, width, height));
////            p_new = p;
//        }
//        for(int i = r*(-1); i<= r ; i++) {
//            int p = getPoint(x, y+i, pixels, width, height);
//            float gau = GaussianResult(i, 0, sigma);
//            int BB = p & 0xff;
//            int GG = (p<<8) & 0xff;
//            int RR = (p<<16) & 0xff;
//            int AA = (p<<24) & 0xff;
//            p_BB += BB*gau;
//            p_GG += GG*gau;
//            p_RR += RR*gau;
//            p_AA += AA*gau;
//
////            p_new = (p_AA & 0xff) << 24 | (p_RR & 0xff) << 16 | (p_GG & 0xff) << 8 | (p_BB & 0xff);
////            p_new+=(int) (GaussianResult(i, (-1)*r, sigma)*getPoint(x, y+i, pixels, width, height));
////            p_new = p;
//        }
//        for(int i = r*(-1); i<= r ; i++) {
//            int p = getPoint(x+r, y+i, pixels, width, height);
//            float gau = GaussianResult(i, (1)*r, sigma);
//            int BB = p & 0xff;
//            int GG = (p<<8) & 0xff;
//            int RR = (p<<16) & 0xff;
//            int AA = (p<<24) & 0xff;
//            p_BB += BB*gau;
//            p_GG += GG*gau;
//            p_RR += RR*gau;
//            p_AA += AA*gau;
//
////            p_new = (p_AA & 0xff) << 24 | (p_RR & 0xff) << 16 | (p_GG & 0xff) << 8 | (p_BB & 0xff);
////            p_new +=(int) (GaussianResult(i, (-1)*r, sigma)*getPoint(x+r, y+i, pixels, width, height));
////            p_new = p;
//        }
//        p_new = (p_AA & 0xff) << 24 | (p_RR & 0xff) << 16 | (p_GG & 0xff) << 8 | (p_BB & 0xff);

//        return p_new;
//
        int length=pixels.length;
        int height_1=length/width;
        for( int i=r*(-1); i<=r; i++){
            if(x+i>=0 && x+i<width){
                for( int j=r*(-1); j<=r; j++){
                    if(y+j>=0 && y+j<height_1){
                        int p = getPoint(x+r, y+i, pixels, width, height);
                        float gau = GaussianResult(i, j, sigma);
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

        int[] BB=new int[width*height];
        for(int i=0;i<width*height;i++){
            BB[i]=(pixels[i]&0xff);
        }

        int[] GG=new int[width*height];
        for(int i=0;i<width*height;i++){
            GG[i]=((pixels[i]<<8)&0xff);
        }

        int[] RR=new int[width*height];
        for(int i=0;i<width*height;i++){
            RR[i]=((pixels[i]<<16)&0xff);
        }

        int[] AA=new int[width*height];
        for(int i=0;i<width*height;i++){
            AA[i]=((pixels[i]<<24)&0xff);
        }

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
//                    BB[y*width+x]=Weight_matrix(r_far, y, x, BB, width, s_far, height);
//                    GG[y*width+x]=Weight_matrix(r_far, y, x, GG, width, s_far, height);
//                    RR[y*width+x]=Weight_matrix(r_far, y, x, RR, width, s_far, height);
//                    AA[y*width+x]=Weight_matrix(r_far, y, x, AA, width, s_far, height);
//                    int color = (AA[y*width+x] & 0xff) << 24 | (RR[y*width+x] & 0xff) << 16 | (GG[y*width+x] & 0xff) << 8 | (BB[y*width+x] & 0xff);
//                    pixels[y*width+x] = color;

//                    Apply guassian blur(through weighted sum
                    if (s_far >= 0.7) {
                        pixels[y * width + x] = Weight_matrix(r_far, y, x, pixels, width, s_far, height);
                    }
                }
                else if(y <= a1) {
////                   Apply guassian blur(through weighted sum
                    float sigma = s_far*(a1-y)/(a1-a0);
                    if (sigma >= 0.7) {
                        pixels[y * width + x] = Weight_matrix(r_far, y, x, pixels, width, sigma, height);
                    }
                }
                else if(y <= a2) {
//                    No nlur
                }
                else if(y <= a3) {
                    float sigma = s_far*(y-a2)/(a3-a2);
                    if(sigma >= 0.7) {
                        pixels[y * width + x] = Weight_matrix(r_near, y, x, pixels, width, sigma, height);
                    }
                }
                else {
//                     Apply guassian blur(through weighted sum
                    if (s_near >= 0.7) {
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
