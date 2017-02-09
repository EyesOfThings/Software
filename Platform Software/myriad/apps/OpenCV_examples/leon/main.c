#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>

#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <mv_types.h>
#include <rtems/cpuuse.h>
#include <bsp.h>

#include "rtems_config.h"

//#include "WifiFunctions.h"
//#include "PulgaMQTTBrokerControl.h"

#include <SDCardIO.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>







// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
static int rc1;
static pthread_t thread1;
static sem_t sem;


// 4: Static Local Data
// ----------------------------------------------------------------------------
void *mainFunction(void *arg);

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------


void POSIX_Init (void *args)
{
    int result;
    pthread_attr_t attr;

    initClocksAndMemory();
    printk ("\n");
    printk ("RTEMS connectToAP started\n");  /* initialise variables */


    if(pthread_attr_init(&attr) !=0) {
        printk("pthread_attr_init error");
    }
    if(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
        printk("pthread_attr_setinheritsched error");
    }
    if(pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
        printk("pthread_attr_setschedpolicy error");
    }

    if(sem_init(&sem, 0, 0) == -1) {
        printk("sem_init error\n");
    }

    if ((rc1=pthread_create(&thread1, &attr, &mainFunction, NULL))) {
        printk("Thread 1 creation failed: %d\n", rc1);
    }
    else {
        printk("Thread 1 created\n");
    }

    // wait for thread to finish
    result = pthread_join( thread1, NULL);
    if(result != 0) {
        printk("pthread_join error (%d)!\n", result);
    }

    exit(0);
    return;
}

void myassert(int a, int b){
    if(a==b){
        printf("Test passed: OK\n");
    }else{
        printf("Test passed: Error\n");
    }
}


// edge.c
int testCanny(char* src, char* dst){
    IplImage* image = cvLoadImage("/mnt/sdcard/OpenCVTests/data/nature.png",CV_LOAD_IMAGE_COLOR);
    if(!image) return -1;
    // Create the output image
    IplImage* cedge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 3);

    cvShowImage("Canny", image);
    
    // Convert to grayscale
    IplImage* gray = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    IplImage* edge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(image, gray, CV_BGR2GRAY);

    cvSmooth( gray, edge, CV_BLUR, 3, 3, 0, 0 );
    cvNot( gray, edge );

    // Run the edge detector on grayscale
    int edge_thresh = 1;
    cvCanny(gray, edge, (float)edge_thresh, (float)edge_thresh*3, 3);
  
    cvZero( cedge );
    // copy edge points
    cvCopy( image, cedge, edge );

    int res = cvSaveImage("/mnt/sdcard/OpenCVTests/results/resultEdge.png", cedge);
    if(res<=0) return -1;

    // Wait for a key stroke; the same function arranges events processing
    cvReleaseImage(&image);
    cvReleaseImage(&gray);
    cvReleaseImage(&edge);
    return 0;
}


//delaunay.c
/* the script demostrates iterative construction of
   delaunay triangulation and voronoi tesselation */
CvSubdiv2D* init_delaunay( CvMemStorage* storage,
                           CvRect rect )
{
    CvSubdiv2D* subdiv;

    subdiv = cvCreateSubdiv2D( CV_SEQ_KIND_SUBDIV2D, sizeof(*subdiv),
                               sizeof(CvSubdiv2DPoint),
                               sizeof(CvQuadEdge2D),
                               storage );
    cvInitSubdivDelaunay2D( subdiv, rect );

    return subdiv;
}
void draw_subdiv_point( IplImage* img, CvPoint2D32f fp, CvScalar color )
{
    cvCircle( img, cvPoint(cvRound(fp.x), cvRound(fp.y)), 3, color, CV_FILLED, 8, 0 );
}
void draw_subdiv_edge( IplImage* img, CvSubdiv2DEdge edge, CvScalar color )
{
    CvSubdiv2DPoint* org_pt;
    CvSubdiv2DPoint* dst_pt;
    CvPoint2D32f org;
    CvPoint2D32f dst;
    CvPoint iorg, idst;

    org_pt = cvSubdiv2DEdgeOrg(edge);
    dst_pt = cvSubdiv2DEdgeDst(edge);

    if( org_pt && dst_pt )
    {
        org = org_pt->pt;
        dst = dst_pt->pt;

        iorg = cvPoint( cvRound( org.x ), cvRound( org.y ));
        idst = cvPoint( cvRound( dst.x ), cvRound( dst.y ));

        cvLine( img, iorg, idst, color, 1, CV_AA, 0 );
    }
}
void draw_subdiv( IplImage* img, CvSubdiv2D* subdiv,
                  CvScalar delaunay_color, CvScalar voronoi_color )
{
    CvSeqReader  reader;
    int i, total = subdiv->edges->total;
    int elem_size = subdiv->edges->elem_size;

    cvStartReadSeq( (CvSeq*)(subdiv->edges), &reader, 0 );

    for( i = 0; i < total; i++ )
    {
        CvQuadEdge2D* edge = (CvQuadEdge2D*)(reader.ptr);

        if( CV_IS_SET_ELEM( edge ))
        {
            draw_subdiv_edge( img, (CvSubdiv2DEdge)edge + 1, voronoi_color );
            draw_subdiv_edge( img, (CvSubdiv2DEdge)edge, delaunay_color );
        }

        CV_NEXT_SEQ_ELEM( elem_size, reader );
    }
}
void locate_point( CvSubdiv2D* subdiv, CvPoint2D32f fp, IplImage* img,
                   CvScalar active_color )
{
    CvSubdiv2DEdge e;
    CvSubdiv2DEdge e0 = 0;
    CvSubdiv2DPoint* p = 0;

    cvSubdiv2DLocate( subdiv, fp, &e0, &p );

    if( e0 )
    {
        e = e0;
        do
        {
            draw_subdiv_edge( img, e, active_color );
            e = cvSubdiv2DGetEdge(e,CV_NEXT_AROUND_LEFT);
        }
        while( e != e0 );
    }

    draw_subdiv_point( img, fp, active_color );
}
void draw_subdiv_facet( IplImage* img, CvSubdiv2DEdge edge )
{
    CvSubdiv2DEdge t = edge;
    int i, count = 0;
    CvPoint* buf = 0;

    // count number of edges in facet
    do
    {
        count++;
        t = cvSubdiv2DGetEdge( t, CV_NEXT_AROUND_LEFT );
    } while (t != edge );

    buf = (CvPoint*)malloc( count * sizeof(buf[0]));

    // gather points
    t = edge;
    for( i = 0; i < count; i++ )
    {
        CvSubdiv2DPoint* pt = cvSubdiv2DEdgeOrg( t );
        if( !pt ) break;
        buf[i] = cvPoint( cvRound(pt->pt.x), cvRound(pt->pt.y));
        t = cvSubdiv2DGetEdge( t, CV_NEXT_AROUND_LEFT );
    }

    if( i == count )
    {
        CvSubdiv2DPoint* pt = cvSubdiv2DEdgeDst( cvSubdiv2DRotateEdge( edge, 1 ));
        cvFillConvexPoly( img, buf, count, CV_RGB(rand()&255,rand()&255,rand()&255), CV_AA, 0 );
        cvPolyLine( img, &buf, &count, 1, 1, CV_RGB(0,0,0), 1, CV_AA, 0);
        draw_subdiv_point( img, pt->pt, CV_RGB(0,0,0));
    }
    free( buf );
}
void paint_voronoi( CvSubdiv2D* subdiv, IplImage* img )
{
    CvSeqReader  reader;
    int i, total = subdiv->edges->total;
    int elem_size = subdiv->edges->elem_size;

    cvCalcSubdivVoronoi2D( subdiv );

    cvStartReadSeq( (CvSeq*)(subdiv->edges), &reader, 0 );

    for( i = 0; i < total; i++ )
    {
        CvQuadEdge2D* edge = (CvQuadEdge2D*)(reader.ptr);

        if( CV_IS_SET_ELEM( edge ))
        {
            CvSubdiv2DEdge e = (CvSubdiv2DEdge)edge;
            // left
            draw_subdiv_facet( img, cvSubdiv2DRotateEdge( e, 1 ));

            // right
            draw_subdiv_facet( img, cvSubdiv2DRotateEdge( e, 3 ));
        }

        CV_NEXT_SEQ_ELEM( elem_size, reader );
    }
}
int delaunay(void)
{
    int i;
    CvRect rect = { 0, 0, 600, 600 };
    CvMemStorage* storage;
    CvSubdiv2D* subdiv;
    IplImage* img;
    CvScalar active_facet_color, delaunay_color, voronoi_color, bkgnd_color;

    active_facet_color = CV_RGB( 255, 0, 0 );
    delaunay_color  = CV_RGB( 0,0,0);
    voronoi_color = CV_RGB(0, 180, 0);
    bkgnd_color = CV_RGB(255,255,255);

    img = cvCreateImage( cvSize(rect.width,rect.height), 8, 3 );
    cvSet( img, bkgnd_color, 0 );

    storage = cvCreateMemStorage(0);
    subdiv = init_delaunay( storage, rect );

    //printf("Delaunay triangulation...\n");

    for( i = 0; i < 3; i++ )
    {
        CvPoint2D32f fp = cvPoint2D32f( (float)(rand()%(rect.width-10)+5),
                                        (float)(rand()%(rect.height-10)+5));

        locate_point( subdiv, fp, img, active_facet_color );
        char* file = (char*)malloc(150*sizeof(char));
        sprintf(file,"/mnt/sdcard/OpenCVTests/results/delaunay/delaunay%d.png",i);
        int res1 = cvSaveImage(file, img);
        if(res1<=0) return -1;
        
        cvSubdivDelaunay2DInsert( subdiv, fp );
        cvCalcSubdivVoronoi2D( subdiv );
        cvSet( img, bkgnd_color, 0 );
        draw_subdiv( img, subdiv, delaunay_color, voronoi_color );
        
        sprintf(file,"/mnt/sdcard/OpenCVTests/results/delaunay/delaunay%d_s.png",i);
        res1 = cvSaveImage(file, img);
        if(res1<=0) return -1;
    }

    cvSet( img, bkgnd_color, 0 );
    paint_voronoi( subdiv, img );
     
    int res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/delaunay/voronoi.png", img);
    if(res1<=0) return -1;

    cvReleaseMemStorage( &storage );
    cvReleaseImage(&img);
    return 0;
}


int testFaceDetectHaarCascade(char* src, char* cascade_name, char* dst){

    IplImage* img = cvLoadImage(src, 1);
    
    CvMemStorage* storage = 0;

    CvHaarClassifierCascade* cascade = 0;

    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, storage, 0, 0);
    storage = cvCreateMemStorage(0);
    
    static CvScalar colors[] = 
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };
    double scale = 1.3;
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)),
                     8, 1 );
    int i;
    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );

    if( cascade )
    {
        //double t = (double)cvGetTickCount();
        CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
                                            cvSize(30, 30) );
        //t = (double)cvGetTickCount() - t;
        //printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
            CvPoint center;
            int radius;
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
            radius = cvRound((r->width + r->height)*0.25*scale);
            cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
        }
    }
    //cvShowImage( "result", img );
    int res = cvSaveImage(dst, img);
    if(res<=0) return -1;
    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
    
    cvReleaseImage(&img);
            
    return 0;
}

// kmeans.c
int testKmeans(char* dst){
    #define MAX_CLUSTERS 5
    CvScalar color_tab[MAX_CLUSTERS];
    IplImage* img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );
    CvRNG rng = cvRNG(-1);
    CvPoint ipt;

    color_tab[0] = CV_RGB(255,0,0);
    color_tab[1] = CV_RGB(0,255,0);
    color_tab[2] = CV_RGB(100,100,255);
    color_tab[3] = CV_RGB(255,0,255);
    color_tab[4] = CV_RGB(255,255,0);

    //cvNamedWindow( "clusters", 1 );
        
        char key;
        int k, cluster_count = cvRandInt(&rng)%MAX_CLUSTERS + 1;
        int i, sample_count = cvRandInt(&rng)%1000 + 1;
        CvMat* points = cvCreateMat( sample_count, 1, CV_32FC2 );
        CvMat* clusters = cvCreateMat( sample_count, 1, CV_32SC1 );
        
        /* generate random sample from multigaussian distribution */
        for( k = 0; k < cluster_count; k++ )
        {
            CvPoint center;
            CvMat point_chunk;
            center.x = cvRandInt(&rng)%img->width;
            center.y = cvRandInt(&rng)%img->height;
            cvGetRows( points, &point_chunk, k*sample_count/cluster_count,
                       k == cluster_count - 1 ? sample_count :
                       (k+1)*sample_count/cluster_count, 1 );
                        
            cvRandArr( &rng, &point_chunk, CV_RAND_NORMAL,
                       cvScalar(center.x,center.y,0,0),
                       cvScalar(img->width*0.1,img->height*0.1,0,0));
        }

        /* shuffle samples */
        for( i = 0; i < sample_count/2; i++ )
        {
            CvPoint2D32f* pt1 = (CvPoint2D32f*)points->data.fl + cvRandInt(&rng)%sample_count;
            CvPoint2D32f* pt2 = (CvPoint2D32f*)points->data.fl + cvRandInt(&rng)%sample_count;
            CvPoint2D32f temp;
            CV_SWAP( *pt1, *pt2, temp );
        }

        cvKMeans2( points, cluster_count, clusters,
                   cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0 ));

        cvZero( img );

        for( i = 0; i < sample_count; i++ )
        {
            int cluster_idx = clusters->data.i[i];
            ipt.x = (int)points->data.fl[i*2];
            ipt.y = (int)points->data.fl[i*2+1];
            cvCircle( img, ipt, 2, color_tab[cluster_idx], CV_FILLED, CV_AA, 0 );
        }

        cvReleaseMat( &points );
        cvReleaseMat( &clusters );

        int res = cvSaveImage(dst, img);        
        //cvShowImage( "clusters", img );

        //key = (char) cvWaitKey(0);
        //if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
        //    break;
    
    
    //cvDestroyWindow( "clusters" );
    return 0;
}

// contours.c
int testContours(char* pic, char* pic_contours){
    int levels = 3;
    CvSeq* contours = 0;
    
    int i, j;
    CvMemStorage* storage = cvCreateMemStorage(0);
    IplImage* img = cvCreateImage( cvSize(500,500), 8, 1 );

    cvZero( img );

    for( i=0; i < 6; i++ )
    {
        int dx = (i%2)*250 - 30;
        int dy = (i/2)*150;
        CvScalar white = cvRealScalar(255);
        CvScalar black = cvRealScalar(0);

        if( i == 0 )
        {
            for( j = 0; j <= 10; j++ )
            {
                double angle = (j+5)*CV_PI/21;
                cvLine(img, cvPoint(cvRound(dx+100+j*10-80*cos(angle)),
                    cvRound(dy+100-90*sin(angle))),
                    cvPoint(cvRound(dx+100+j*10-30*cos(angle)),
                    cvRound(dy+100-30*sin(angle))), white, 1, 8, 0);
            }
        }

        cvEllipse( img, cvPoint(dx+150, dy+100), cvSize(100,70), 0, 0, 360, white, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+115, dy+70), cvSize(30,20), 0, 0, 360, black, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+185, dy+70), cvSize(30,20), 0, 0, 360, black, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+115, dy+70), cvSize(15,15), 0, 0, 360, white, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+185, dy+70), cvSize(15,15), 0, 0, 360, white, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+115, dy+70), cvSize(5,5), 0, 0, 360, black, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+185, dy+70), cvSize(5,5), 0, 0, 360, black, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+150, dy+100), cvSize(10,5), 0, 0, 360, black, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+150, dy+150), cvSize(40,10), 0, 0, 360, black, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+27, dy+100), cvSize(20,35), 0, 0, 360, white, -1, 8, 0 );
        cvEllipse( img, cvPoint(dx+273, dy+100), cvSize(20,35), 0, 0, 360, white, -1, 8, 0 );
    }

    //cvNamedWindow( "image", 1 );
    //cvShowImage( "image", img );
    int res = cvSaveImage(pic, img); 

    cvFindContours( img, storage, &contours, sizeof(CvContour),
                    CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

    // comment this out if you do not want approximation
    contours = cvApproxPoly( contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 3, 1 );

    //cvNamedWindow( "contours", 1 );
    //cvCreateTrackbar( "levels+3", "contours", &levels, 7, on_trackbar );
    
    IplImage* cnt_img = cvCreateImage( cvSize(500,500), 8, 3 );
    CvSeq* _contours = contours;
    int _levels = levels - 3;
    if( _levels <= 0 ) // get to the nearest face to make it look more funny
        _contours = _contours->h_next->h_next->h_next;
    cvZero( cnt_img );
    cvDrawContours( cnt_img, _contours, CV_RGB(255,0,0), CV_RGB(0,255,0), _levels, 3, CV_AA, cvPoint(0,0) );
    
    //cvShowImage( "contours", cnt_img );
    res = cvSaveImage(pic_contours, cnt_img); 
    
    cvReleaseImage( &cnt_img );
    
    //cvWaitKey(0);
    cvReleaseMemStorage( &storage );
    cvReleaseImage( &img );
    return 0;
    
}

// demhist.c
int testDemHist(char* filename, char* result){

    int _brightness = 100;
    int _contrast = 100;

    int hist_size = 64;
    float range_0[] = {0, 256};
    float* ranges[] = {range_0};
    IplImage *src_image = 0, *dst_image = 0, *hist_image = 0;
    CvHistogram *hist;
    uchar lut[256];
    CvMat* lut_mat;
    // Load the source image. HighGUI use.
    src_image = cvLoadImage( filename, 0 );

    if( !src_image )
    {
        printf("Image was not loaded.\n");
        return -1;
    }

    dst_image = cvCloneImage(src_image);
    hist_image = cvCreateImage(cvSize(320,200), 8, 1);
    hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
    cvSetData( lut_mat, lut, 0 );
    
    
    //cvNamedWindow("image", 0);
    //cvNamedWindow("histogram", 0);

    //cvCreateTrackbar("brightness", "image", &_brightness, 200, update_brightcont);
    //cvCreateTrackbar("contrast", "image", &_contrast, 200, update_brightcont);

    
    int brightness = _brightness - 100;
    int contrast = _contrast - 100;
    int i, bin_w;
    float max_value = 0;

    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    if( contrast > 0 )
    {
        double delta = 127.*contrast/100;
        double a = 255./(255. - delta*2);
        double b = a*(brightness - delta);
        for( i = 0; i < 256; i++ )
        {
            int v = cvRound(a*i + b);
            if( v < 0 )
                v = 0;
            if( v > 255 )
                v = 255;
            lut[i] = (uchar)v;
        }
    }
    else
    {
        double delta = -128.*contrast/100;
        double a = (256.-delta*2)/255.;
        double b = a*brightness + delta;
        for( i = 0; i < 256; i++ )
        {
            int v = cvRound(a*i + b);
            if( v < 0 )
                v = 0;
            if( v > 255 )
                v = 255;
            lut[i] = (uchar)v;
        }
    }
    
    cvLUT( src_image, dst_image, lut_mat );
    //cvShowImage( "image", dst_image );

    cvCalcHist( &dst_image, hist, 0, NULL );
    cvZero( dst_image );
    cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
    cvScale( hist->bins, hist->bins, ((double)hist_image->height)/max_value, 0 );
    //cvNormalizeHist( hist, 1000 );
    
    
    cvSet( hist_image, cvScalarAll(255), 0 );
    bin_w = cvRound((double)hist_image->width/hist_size);

    for( i = 0; i < hist_size; i++ )
        cvRectangle( hist_image, cvPoint(i*bin_w, hist_image->height),
                     cvPoint((i+1)*bin_w, hist_image->height - cvRound(cvGetReal1D(hist->bins,i))),
                     cvScalarAll(0), -1, 8, 0 );

    //cvShowImage( "histogram", hist_image );
    int res = cvSaveImage(result, hist_image);

    //cvWaitKey(0);
    
    cvReleaseImage(&src_image);
    cvReleaseImage(&dst_image);

    cvReleaseHist(&hist);

    return 0;
}

//pyramid_segmentation.c
int testPyramidSegmentation(char* filename, char* result) {
    IplImage * image[2] = {0, 0}, *image0 = 0, *image1 = 0;
    CvSize size;

    int w0, h0, i;
    int threshold1, threshold2;
    int l, level = 4;
    int sthreshold1, sthreshold2;
    int l_comp;
    int block_size = 1000;
    float parameter;
    double threshold;
    double rezult, min_rezult;
    CvFilter filter = CV_GAUSSIAN_5x5;
    CvConnectedComp *cur_comp, min_comp;
    CvSeq *comp;
    CvMemStorage *storage;

    CvPoint pt1, pt2;
    
    if( (image[0] = cvLoadImage( filename, 1)) == 0 )
        return -1;

    //cvNamedWindow("Source", 0);
    //cvShowImage("Source", image[0]);

    //cvNamedWindow("Segmentation", 0);

    storage = cvCreateMemStorage ( block_size );

    image[0]->width &= -(1<<level);
    image[0]->height &= -(1<<level);

    image0 = cvCloneImage( image[0] );
    image1 = cvCloneImage( image[0] );
    // segmentation of the color image
    l = 1;
    threshold1 =255;
    threshold2 =30;

    cvPyrSegmentation(image0, image1, storage, &comp, 
                      level, threshold1+1, threshold2+1);
    //cvShowImage("Segmentation", image1);
    int res = cvSaveImage(result, image1);


    //cvDestroyWindow("Segmentation");
    //cvDestroyWindow("Source");

    cvReleaseMemStorage(&storage );

    cvReleaseImage(&image[0]);
    cvReleaseImage(&image0);
    cvReleaseImage(&image1);

    return 0;
}

//squares.c
// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2 
double angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 )
{
    double dx1 = pt1->x - pt0->x;
    double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
CvSeq* findSquares4( IplImage* img, CvMemStorage* storage )
{
    CvSeq* contours;
    int i, c, l, N = 11;
    CvSize sz = cvSize( img->width & -2, img->height & -2 );
    IplImage* timg = cvCloneImage( img ); // make a copy of input image
    IplImage* gray = cvCreateImage( sz, 8, 1 ); 
    IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 3 );
    IplImage* tgray;
    CvSeq* result;
    double s, t;
    int thresh = 50;
    // create empty sequence that will contain points -
    // 4 points per square (the square's vertices)
    CvSeq* squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage );
    
    // select the maximum ROI in the image
    // with the width and height divisible by 2
    cvSetImageROI( timg, cvRect( 0, 0, sz.width, sz.height ));
    
    // down-scale and upscale the image to filter out the noise
    cvPyrDown( timg, pyr, 7 );
    cvPyrUp( pyr, timg, 7 );
    tgray = cvCreateImage( sz, 8, 1 );
    
    // find squares in every color plane of the image
    for( c = 0; c < 3; c++ )
    {
        // extract the c-th color plane
        cvSetImageCOI( timg, c+1 );
        cvCopy( timg, tgray, 0 );
        
        // try several threshold levels
        for( l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading   
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging) 
                cvCanny( tgray, gray, 0, thresh, 5 );
                // dilate canny output to remove potential
                // holes between edge segments 
                cvDilate( gray, gray, 0, 1 );
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                cvThreshold( tgray, gray, (l+1)*255/N, 255, CV_THRESH_BINARY );
            }
            
            // find contours and store them all as a list
            cvFindContours( gray, storage, &contours, sizeof(CvContour),
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
            
            // test each contour
            while( contours )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                result = cvApproxPoly( contours, sizeof(CvContour), storage,
                    CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if( result->total == 4 &&
                    fabs(cvContourArea(result,CV_WHOLE_SEQ)) > 1000 &&
                    cvCheckContourConvexity(result) )
                {
                    s = 0;
                    
                    for( i = 0; i < 5; i++ )
                    {
                        // find minimum angle between joint
                        // edges (maximum of cosine)
                        if( i >= 2 )
                        {
                            t = fabs(angle(
                            (CvPoint*)cvGetSeqElem( result, i ),
                            (CvPoint*)cvGetSeqElem( result, i-2 ),
                            (CvPoint*)cvGetSeqElem( result, i-1 )));
                            s = s > t ? s : t;
                        }
                    }
                    
                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence 
                    if( s < 0.3 )
                        for( i = 0; i < 4; i++ )
                            cvSeqPush( squares,
                                (CvPoint*)cvGetSeqElem( result, i ));
                }
                
                // take the next contour
                contours = contours->h_next;
            }
        }
    }
    
    // release all the temporary images
    cvReleaseImage( &gray );
    cvReleaseImage( &pyr );
    cvReleaseImage( &tgray );
    cvReleaseImage( &timg );
    
    return squares;
}


// the function draws all the squares in the image
void drawSquares( IplImage* img, CvSeq* squares, int pos_img )
{
    char* resultsSquares[] = {"/mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic1.png", "/mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic2.png", "/mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic3.png",
        "/mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic4.png", "/mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic5.png", "/mnt/sdcard/OpenCVTests/results/squares/ResultSquarespic6.png", 0};
    CvSeqReader reader;
    IplImage* cpy = cvCloneImage( img );
    int i;
    
    // initialize reader of the sequence
    cvStartReadSeq( squares, &reader, 0 );
    
    // read 4 sequence elements at a time (all vertices of a square)
    for( i = 0; i < squares->total; i += 4 )
    {
        CvPoint pt[4], *rect = pt;
        int count = 4;
        
        // read 4 vertices
        CV_READ_SEQ_ELEM( pt[0], reader );
        CV_READ_SEQ_ELEM( pt[1], reader );
        CV_READ_SEQ_ELEM( pt[2], reader );
        CV_READ_SEQ_ELEM( pt[3], reader );
        
        // draw the square as a closed polyline 
        cvPolyLine( cpy, &rect, &count, 1, 1, CV_RGB(0,255,0), 3, CV_AA, 0 );
    }
    
    // show the resultant image
    //cvShowImage( wndname, cpy );
    int res = cvSaveImage(resultsSquares[pos_img], cpy);
    
    cvReleaseImage( &cpy );
}


int testSquareDetector(){
    char* namesSquares[] = {"/mnt/sdcard/OpenCVTests/data/pic1.png", "/mnt/sdcard/OpenCVTests/data/pic2.png", "/mnt/sdcard/OpenCVTests/data/pic3.png",
        "/mnt/sdcard/OpenCVTests/data/pic4.png", "/mnt/sdcard/OpenCVTests/data/pic5.png", "/mnt/sdcard/OpenCVTests/data/pic6.png", 0};

    
    
    int i, c;
    int thresh = 50;
    IplImage* img = 0;
    IplImage* img0 = 0;
    CvMemStorage* storage = 0;
    //const char* wndname = "Square Detection Demo";
    // create memory storage that will contain all the dynamic data
    storage = cvCreateMemStorage(0);

    for( i = 0; namesSquares[i] != 0; i++ )
    {
        // load i-th image
        img0 = cvLoadImage( namesSquares[i], 1 );
        if( !img0 )
        {
            printf("Couldn't load %s\n", namesSquares[i] );
            continue;
        }
        img = cvCloneImage( img0 );
        
        // create window and a trackbar (slider) with parent "image" and set callback
        // (the slider regulates upper threshold, passed to Canny edge detector) 
        //cvNamedWindow( wndname, 1 );
        
        // find and draw the squares
        drawSquares( img, findSquares4( img, storage ), i );
        
        // wait for key.
        // Also the function cvWaitKey takes care of event processing
        //c = cvWaitKey(0);
        // release both images
        cvReleaseImage( &img );
        cvReleaseImage( &img0 );
        // clear memory storage - reset free space position
        cvClearMemStorage( storage );
        //if( (char)c == 27 )
        //    break;
    }
    
    //cvDestroyWindow( wndname );
    
    return 0;
}


int testWatershed(char* filename, char* result){
    IplImage* marker_mask = 0;
    IplImage* markers = 0;
    IplImage* img0 = 0, *img = 0, *img_gray = 0, *wshed = 0;
    CvPoint prev_pt = {1,1};
    
    CvRNG rng = cvRNG(-1);

    if( (img0 = cvLoadImage(filename,1)) == 0 )
        return 0;

    img = cvCloneImage( img0 );
    img_gray = cvCloneImage( img0 );
    wshed = cvCloneImage( img0 );
    marker_mask = cvCreateImage( cvGetSize(img), 8, 1 );
    markers = cvCreateImage( cvGetSize(img), IPL_DEPTH_32S, 1 );
    cvCvtColor( img, marker_mask, CV_BGR2GRAY );
    cvCvtColor( marker_mask, img_gray, CV_GRAY2BGR );
    //cvZero( marker_mask );
    //cvZero( wshed );
    
    //cvShowImage( "image", img );
    //cvShowImage( "watershed transform", wshed );
    
    
            CvMemStorage* storage = cvCreateMemStorage(0);
            CvSeq* contours = 0;
            CvMat* color_tab;
            int i, j, comp_count = 0;
            //cvSaveImage( "wshed_mask.png", marker_mask );
            //marker_mask = cvLoadImage( "wshed_mask.png", 0 );
            cvFindContours( marker_mask, storage, &contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
            cvZero( markers );
            for( ; contours != 0; contours = contours->h_next, comp_count++ )
            {
                cvDrawContours( markers, contours, cvScalarAll(comp_count+1),
                                cvScalarAll(comp_count+1), -1, -1, 8, cvPoint(0,0) );
            }

            color_tab = cvCreateMat( 1, comp_count, CV_8UC3 );

            for( i = 0; i < comp_count; i++ )
            {
                uchar* ptr = color_tab->data.ptr + i*3;
                ptr[0] = (uchar)(cvRandInt(&rng)%180 + 50);
                ptr[1] = (uchar)(cvRandInt(&rng)%180 + 50);
                ptr[2] = (uchar)(cvRandInt(&rng)%180 + 50);
            }

            {
            //double t = (double)cvGetTickCount();
            cvWatershed( img0, markers );
            //t = (double)cvGetTickCount() - t;
            //printf( "exec time = %gms\n", t/(cvGetTickFrequency()*1000.) );
            }

            // paint the watershed image
            for( i = 0; i < markers->height; i++ )
                for( j = 0; j < markers->width; j++ )
                {
                    int idx = CV_IMAGE_ELEM( markers, int, i, j );
                    uchar* dst = &CV_IMAGE_ELEM( wshed, uchar, i, j*3 );
                    if( idx == -1 )
                        dst[0] = dst[1] = dst[2] = (uchar)255;
                    else if( idx <= 0 || idx > comp_count )
                        dst[0] = dst[1] = dst[2] = (uchar)0; // should not get here
                    else
                    {
                        uchar* ptr = color_tab->data.ptr + (idx-1)*3;
                        dst[0] = ptr[0]; dst[1] = ptr[1]; dst[2] = ptr[2];
                    }
                }

            cvAddWeighted( wshed, 0.5, img_gray, 0.5, 0, wshed );
            //cvShowImage( "watershed transform", wshed );
            int res = cvSaveImage(result, wshed);
            cvReleaseMemStorage( &storage );
            cvReleaseMat( &color_tab );
        //}
    

    return 0;
}


// morphology.c
void OpenClose(int pos, IplImage* src, IplImage* dst,int element_shape)   
{
    //the address of variable which receives trackbar position update 
    int max_iters = 10;
    int open_close_pos = 0;
    int erode_dilate_pos = 0;
    open_close_pos = erode_dilate_pos = max_iters/2.0;
    int n = open_close_pos - max_iters;
    int an = n > 0 ? n : -n;
    IplConvKernel* element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    if( n < 0 )
    {
        cvErode(src,dst,element,1);
        cvDilate(dst,dst,element,1);
    }
    else
    {
        cvDilate(src,dst,element,1);
        cvErode(dst,dst,element,1);
    }
    cvReleaseStructuringElement(&element);
}   
void ErodeDilate(int pos, IplImage* src, IplImage* dst, int element_shape)   
{
    //the address of variable which receives trackbar position update 
    int max_iters = 10;
    int open_close_pos = 0;
    int erode_dilate_pos = 0;
    open_close_pos = erode_dilate_pos = max_iters/2.0;
    int n = erode_dilate_pos - max_iters;
    int an = n > 0 ? n : -n;
    IplConvKernel* element = cvCreateStructuringElementEx( an*2+1, an*2+1, an, an, element_shape, 0 );
    if( n < 0 )
    {
        cvErode(src,dst,element,1);
    }
    else
    {
        cvDilate(src,dst,element,1);
    }
    cvReleaseStructuringElement(&element);
}   
int morphology( )
{
    char* filename = (char*)"/mnt/sdcard/OpenCVTests/data/baboon.png";
    IplImage* src = 0;
    if( (src = cvLoadImage(filename,1)) == 0 )
        return -1;

    IplImage* dst = cvCloneImage(src);
    //the address of variable which receives trackbar position update 
    int max_iters = 10;
    int open_close_pos = 0;
    int erode_dilate_pos = 0;
    open_close_pos = erode_dilate_pos = max_iters/2.0;
    
    int element_shape = CV_SHAPE_ELLIPSE;
    OpenClose(open_close_pos,src,dst,element_shape);
    int res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/morphology/baboon_E_OC.png", dst);
    if(res1<=0) return -1;
    ErodeDilate(erode_dilate_pos,src,dst,element_shape);
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/morphology/baboon_E_ED.png", dst);
    if(res1<=0) return -1;
    
    element_shape = CV_SHAPE_RECT;
    OpenClose(open_close_pos,src,dst,element_shape);
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/morphology/baboon_R_OC.png", dst);
    if(res1<=0) return -1;
    ErodeDilate(erode_dilate_pos,src,dst,element_shape);
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/morphology/baboon_R_ED.png", dst);
    if(res1<=0) return -1;
    
    element_shape = CV_SHAPE_CROSS;
    OpenClose(open_close_pos,src,dst,element_shape);
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/morphology/baboon_C_OC.png", dst);
    if(res1<=0) return -1;
    ErodeDilate(erode_dilate_pos,src,dst,element_shape);
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/morphology/baboon_C_ED.png", dst);
    if(res1<=0) return -1;

    //release images
    cvReleaseImage(&src);
    cvReleaseImage(&dst);

    return 0;
}




// dft.c
// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
// src & dst arrays of equal size & type
void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr )
{
    CvMat * tmp;
    CvMat q1stub, q2stub;
    CvMat q3stub, q4stub;
    CvMat d1stub, d2stub;
    CvMat d3stub, d4stub;
    CvMat * q1, * q2, * q3, * q4;
    CvMat * d1, * d2, * d3, * d4;

    CvSize size = cvGetSize(src_arr);
    CvSize dst_size = cvGetSize(dst_arr);
    int cx, cy;

    if(dst_size.width != size.width || 
       dst_size.height != size.height){
        cvError( CV_StsUnmatchedSizes, "cvShiftDFT", "Source and Destination arrays must have equal sizes", __FILE__, __LINE__ );   
    }

    if(src_arr==dst_arr){
        tmp = cvCreateMat(size.height/2, size.width/2, cvGetElemType(src_arr));
    }
    
    cx = size.width/2;
    cy = size.height/2; // image center

    q1 = cvGetSubRect( src_arr, &q1stub, cvRect(0,0,cx, cy) );
    q2 = cvGetSubRect( src_arr, &q2stub, cvRect(cx,0,cx,cy) );
    q3 = cvGetSubRect( src_arr, &q3stub, cvRect(cx,cy,cx,cy) );
    q4 = cvGetSubRect( src_arr, &q4stub, cvRect(0,cy,cx,cy) );
    d1 = cvGetSubRect( src_arr, &d1stub, cvRect(0,0,cx,cy) );
    d2 = cvGetSubRect( src_arr, &d2stub, cvRect(cx,0,cx,cy) );
    d3 = cvGetSubRect( src_arr, &d3stub, cvRect(cx,cy,cx,cy) );
    d4 = cvGetSubRect( src_arr, &d4stub, cvRect(0,cy,cx,cy) );

    if(src_arr!=dst_arr){
        if( !CV_ARE_TYPES_EQ( q1, d1 )){
            cvError( CV_StsUnmatchedFormats, "cvShiftDFT", "Source and Destination arrays must have the same format", __FILE__, __LINE__ ); 
        }
        cvCopy(q3, d1, 0);
        cvCopy(q4, d2, 0);
        cvCopy(q1, d3, 0);
        cvCopy(q2, d4, 0);
    }
    else{
        cvCopy(q3, tmp, 0);
        cvCopy(q1, q3, 0);
        cvCopy(tmp, q1, 0);
        cvCopy(q4, tmp, 0);
        cvCopy(q2, q4, 0);
        cvCopy(tmp, q2, 0);
    }
}

int dft()
{
    const char* filename = "/mnt/sdcard/OpenCVTests/data/suit.png";
    IplImage * im;

    IplImage * realInput;
    IplImage * imaginaryInput;
    IplImage * complexInput;
    int dft_M, dft_N;
    CvMat* dft_A, tmp;
    IplImage * image_Re;
    IplImage * image_Im;
    double m, M;

    im = cvLoadImage( filename, CV_LOAD_IMAGE_GRAYSCALE );
    if( !im )
        return -1;

    realInput = cvCreateImage( cvGetSize(im), IPL_DEPTH_32F, 1);
    imaginaryInput = cvCreateImage( cvGetSize(im), IPL_DEPTH_32F, 1);
    complexInput = cvCreateImage( cvGetSize(im), IPL_DEPTH_32F, 2);
    
    cvScale(im, realInput, 1.0, 0.0);
    cvZero(imaginaryInput);
    cvMerge(realInput, imaginaryInput, NULL, NULL, complexInput);
            
    dft_M = cvGetOptimalDFTSize( im->height - 1 );
    dft_N = cvGetOptimalDFTSize( im->width - 1 );
            
    dft_A = cvCreateMat( dft_M, dft_N, CV_32FC2 );
    image_Re = cvCreateImage( cvSize(dft_N, dft_M), IPL_DEPTH_32F, 1);
    image_Im = cvCreateImage( cvSize(dft_N, dft_M), IPL_DEPTH_32F, 1);
            
    // copy A to dft_A and pad dft_A with zeros
    cvGetSubRect( dft_A, &tmp, cvRect(0,0, im->width, im->height));
    cvCopy( complexInput, &tmp, NULL );
    if( dft_A->cols > im->width )
    {
        cvGetSubRect( dft_A, &tmp, cvRect(im->width,0, dft_A->cols - im->width, im->height));
        cvZero( &tmp );
    }
            
    // no need to pad bottom part of dft_A with zeros because of
    // use nonzero_rows parameter in cvDFT() call below
      
    cvDFT( dft_A, dft_A, CV_DXT_FORWARD, complexInput->height );

    // Split Fourier in real and imaginary parts
    cvSplit( dft_A, image_Re, image_Im, 0, 0 );

    // Compute the magnitude of the spectrum Mag = sqrt(Re^2 + Im^2)
    cvPow( image_Re, image_Re, 2.0);
    cvPow( image_Im, image_Im, 2.0);
    cvAdd( image_Re, image_Im, image_Re, NULL);
    cvPow( image_Re, image_Re, 0.5 );

    // Rearrange the quadrants of Fourier image so that the origin is at
    // the image center
    cvShiftDFT( image_Re, image_Re );

    cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL);
    cvScale(image_Re, image_Re, 1.0/(M-m), 1.0*(-m)/(M-m));
    cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL);
    
    cvAbs(image_Re, image_Re);    
    
    IplImage * re = cvCreateImage( cvGetSize(image_Re), IPL_DEPTH_8U, 1);
    int i;
    for(i=0;i<cvGetSize(image_Re).height*cvGetSize(image_Re).width;i++){
        re->imageData[i]=image_Re->imageData[i];
    }
        
    int res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/dftres2.png", re);
    if(res1<=0) return -1;

    return 0;
}


// inpaint.cpp
int inpaint( )
{
    char* filename = "/mnt/sdcard/OpenCVTests/data/lena.png";

    IplImage* img0 = cvLoadImage(filename,-1);
    if( !img0 )
        return -1;

    IplImage* img = cvCloneImage( img0 );
    IplImage* inpainted = cvCloneImage( img0 );
    IplImage* inpaint_mask = cvCreateImage( cvGetSize(img), 8, 1 );

    cvZero( inpaint_mask );
    cvZero( inpainted );

    CvPoint prev_pt = {0,0};
    CvPoint pt = {cvGetSize(img).height,cvGetSize(img).width};
    cvLine( inpaint_mask, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    cvLine( img, prev_pt, pt, cvScalarAll(255), 5, 8, 0 );
    
    cvInpaint( img, inpaint_mask, inpainted, 3, CV_INPAINT_TELEA );
    int res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/inpainted.png", inpainted);
    if(res1<=0) return -1;

    return 0;
}

// minarea.c
int minarea(  )
{
    IplImage* img = cvCreateImage( cvSize( 500, 500 ), 8, 3 );

        char key;
        int i, count = rand()%100 + 1;
        CvPoint pt0, pt;
        CvBox2D box;
        CvPoint2D32f box_vtx[4];
        CvPoint2D32f center;
        CvPoint icenter;
        float radius;

        CvPoint* points = (CvPoint*)malloc( count * sizeof(points[0]));
        CvMat pointMat = cvMat( 1, count, CV_32SC2, points );

        for( i = 0; i < count; i++ )
        {
            pt0.x = rand() % (img->width/2) + img->width/4;
            pt0.y = rand() % (img->height/2) + img->height/4;
            points[i] = pt0;
        }
        box = cvMinAreaRect2( &pointMat, 0 );
        cvMinEnclosingCircle( &pointMat, &center, &radius );
        cvBoxPoints( box, box_vtx );
        cvZero( img );
        for( i = 0; i < count; i++ )
        {
            pt0 = points[i];
            cvCircle( img, pt0, 2, CV_RGB( 255, 0, 0 ), CV_FILLED, CV_AA, 0 );
        }

        pt0.x = cvRound(box_vtx[3].x);
        pt0.y = cvRound(box_vtx[3].y);
        for( i = 0; i < 4; i++ )
        {
            pt.x = cvRound(box_vtx[i].x);
            pt.y = cvRound(box_vtx[i].y);
            cvLine(img, pt0, pt, CV_RGB(0, 255, 0), 1, CV_AA, 0);
            pt0 = pt;
        }
        icenter.x = cvRound(center.x);
        icenter.y = cvRound(center.y);
        cvCircle( img, icenter, cvRound(radius), CV_RGB(255, 255, 0), 1, CV_AA, 0 );

        int res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/rectCircle.png", img);
        if(res1<=0) return -1;

        free( points );

    
    return 0;
}


//lkdemo.c
int testLKdemo(char* filename) {
    IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

    int win_size = 10;
    const int MAX_COUNT = 500;
    CvPoint2D32f * points[2] = {0, 0}, *swap_points;
    char* status = 0;
    int count = 0;
    int need_to_init = 1;
    int night_mode = 0;
    int flags = 0;
    int add_remove_pt = 0;
    CvPoint pt;

    int run_algorithm = 1;

    while (run_algorithm) {

        IplImage* frame = 0;
        int i, k, c;

        frame = cvLoadImage(filename, 1);
        //if (!frame)
        //    break;

        if (!image) {
            //printf("1\n");
            /* allocate all the buffers */
            image = cvCreateImage(cvGetSize(frame), 8, 3);
            image->origin = frame->origin;
            grey = cvCreateImage(cvGetSize(frame), 8, 1);
            prev_grey = cvCreateImage(cvGetSize(frame), 8, 1);
            pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
            prev_pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
            points[0] = (CvPoint2D32f*) cvAlloc(MAX_COUNT * sizeof (points[0][0]));
            points[1] = (CvPoint2D32f*) cvAlloc(MAX_COUNT * sizeof (points[0][0]));
            status = (char*) cvAlloc(MAX_COUNT);
            flags = 0;
        }

        cvCopy(frame, image, 0);
        cvCvtColor(image, grey, CV_BGR2GRAY);

        if (night_mode)
            cvZero(image);

        if (need_to_init) {
            //printf("2\n");
            /* automatic initialization */
            IplImage* eig = cvCreateImage(cvGetSize(grey), 32, 1);
            IplImage* temp = cvCreateImage(cvGetSize(grey), 32, 1);
            double quality = 0.01;
            double min_distance = 10;

            count = MAX_COUNT;
            cvGoodFeaturesToTrack(grey, eig, temp, points[1], &count,
                    quality, min_distance, 0, 3, 0, 0.04);
            cvFindCornerSubPix(grey, points[1], count,
                    cvSize(win_size, win_size), cvSize(-1, -1),
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
            cvReleaseImage(&eig);
            cvReleaseImage(&temp);

            add_remove_pt = 0;
        } else if (count > 0) {
            //printf("3\n");
            cvCalcOpticalFlowPyrLK(prev_grey, grey, prev_pyramid, pyramid,
                    points[0], points[1], count, cvSize(win_size, win_size), 3, status, 0,
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03), flags);
            flags |= CV_LKFLOW_PYR_A_READY;
            for (i = k = 0; i < count; i++) {
                if (add_remove_pt) {
                    double dx = pt.x - points[1][i].x;
                    double dy = pt.y - points[1][i].y;

                    if (dx * dx + dy * dy <= 25) {
                        add_remove_pt = 0;
                        continue;
                    }
                }

                if (!status[i])
                    continue;

                points[1][k++] = points[1][i];
                cvCircle(image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0, 255, 0), -1, 8, 0);
            }
            count = k;
            run_algorithm=0;
        }

        if (add_remove_pt && count < MAX_COUNT) {
            points[1][count++] = cvPointTo32f(pt);
            cvFindCornerSubPix(grey, points[1] + count - 1, 1,
                    cvSize(win_size, win_size), cvSize(-1, -1),
                    cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
            add_remove_pt = 0;
        }

        CV_SWAP(prev_grey, grey, swap_temp);
        CV_SWAP(prev_pyramid, pyramid, swap_temp);
        CV_SWAP(points[0], points[1], swap_points);
        need_to_init = 0;

    }
    
    //cvShowImage("LkDemo", image);
    int res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/LKDemo.png", image);
    

    return 0;
}




void dogCam(IplImage* src, IplImage* dst) {

  // Difference-Of-Gaussians (DOG) works by performing two different Gaussian blurs on the image, 

  // with a different blurring radius for each, and subtracting them to yield the result.   

  //   http://en.wikipedia.org/wiki/Difference_of_Gaussians 

  //   http://docs.gimp.org/en/plug-in-dog.html 
    

  IplImage *dog_1 = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

  IplImage *dog_2 = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

  cvSmooth(src, dog_2, CV_GAUSSIAN, 3, 0, 0, 0); // Gaussian blur

  cvSmooth(src, dog_1, CV_GAUSSIAN, 7, 0, 0, 0);

  cvSub(dog_2, dog_1, dst, 0);
  
  

} // dogCam()


void dilateBenchmark(IplImage* src, IplImage* dst) {

  cvDilate(src,dst,NULL,5);  

} 


void cornerHarrisBenchmark(IplImage* src, IplImage* dst) {

    IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
    cvCvtColor(src, gray, CV_RGB2GRAY);

    dst = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
    cvCornerHarris(gray, dst, 3, 5, 0.07);

} 


void medianFilterBenchmark(IplImage* src, IplImage* dst) {

    dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

    cvSmooth(src, dst, CV_MEDIAN, 3, 0, 0, 0); // 

} 








void *mainFunction(void *arg)
{

    printf("Starting run\n");
    

  
    int result=SDCardMount();
    printf("SD card mounted? %d\n",result);
    int res;
    
    
    res = testCanny("/mnt/sdcard/OpenCVTests/data/nature.png","/mnt/sdcard/OpenCVTests/results/resultEdge.png");
    assert(res==0);
    printf("+ Test 1 passed. OK\n");
    
    res = testFaceDetectHaarCascade("/mnt/sdcard/OpenCVTests/data/lena.png","/mnt/sdcard/OpenCVTests/data/haarface.xml","/mnt/sdcard/OpenCVTests/results/resultFaceDetection.png");
    assert(res==0);
    printf("+ Test 2 passed. OK\n");
    
    res = testKmeans("/mnt/sdcard/OpenCVTests/results/resultTestKmeans.png");
    assert(res==0);
    printf("+ Test 3 passed. OK\n");
    
    res = testContours("/mnt/sdcard/OpenCVTests/results/resultTestContours_1.png","/mnt/sdcard/OpenCVTests/results/resultTestContours_2.png");
    assert(res==0);
    printf("+ Test 4 passed. OK\n");
    
    res = testDemHist("/mnt/sdcard/OpenCVTests/data/baboon.png","/mnt/sdcard/OpenCVTests/results/resultDemHist.png");
    assert(res==0);
    printf("+ Test 5 passed. OK\n");
    
    res = delaunay();
    assert(res==0);
    printf("+ Test 6 passed. OK\n"); 
    
    res=testPyramidSegmentation("/mnt/sdcard/OpenCVTests/data/fruits.png", "/mnt/sdcard/OpenCVTests/results/resultPyramidSegmentation.png");
    assert(res==0);
    printf("+ Test 7 passed. OK\n");
    
    res=testSquareDetector();
    assert(res==0);
    printf("+ Test 8 passed. OK\n");
    
    res=testWatershed("/mnt/sdcard/OpenCVTests/data/fruits.png", "/mnt/sdcard/OpenCVTests/results/resultWatershed.png");
    assert(res==0);
    printf("+ Test 9 passed. OK\n");
    
    res = morphology();
    assert(res==0);
    printf("+ Test 10 passed. OK\n");
    
    res = dft();
    assert(res==0);
    printf("+ Test 11 passed. OK\n");
    
    res = inpaint();
    assert(res==0);
    printf("+ Test 12 passed. OK\n"); 
    
    res = minarea();
    assert(res==0);
    printf("+ Test 13 passed. OK\n"); 
        
    res = testLKdemo("/mnt/sdcard/OpenCVTests/data/suit.png");
    assert(res==0);
    printf("+ Test 14 passed. OK\n");
    
    //BENCHMARKS//
    
    struct timeval currentTime;
    struct timeval previousTime;
    int previousUsec = 0;
    int currentUsec = 0;
    IplImage* src;
    IplImage* dst;
    int res1;
       
    //TEST 15
    src = cvLoadImage("/mnt/sdcard/OpenCVTests/data/DunLoghaire_320x240.png", 1);
    dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

    if( !src )
    {
        printf("Image was not loaded.\n");
        return -1;
    }

    gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

    dogCam(src,dst);   

    gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 15 Usec CPU time: %d \n",currentUsec - previousUsec);
    
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/DOG.png", dst);
    
    //TEST 16
    src = cvLoadImage("/mnt/sdcard/OpenCVTests/data/lena_512x512_luma.png", 1);
    dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

    if( !src )
    {
        printf("Image was not loaded.\n");
        return -1;
    }

    gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

    dilateBenchmark(src,dst);   

    gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 16 Usec CPU time: %d \n",currentUsec - previousUsec);
    
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/dilate2.png", dst);
    
    //TEST 17
    src = cvLoadImage("/mnt/sdcard/OpenCVTests/data/lena_512x512_luma.png", 1);
    dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

    if( !src )
    {
        printf("Image was not loaded.\n");
        return -1;
    }

    gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

    cornerHarrisBenchmark(src,dst);   

    gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 17 Usec CPU time: %d \n",currentUsec - previousUsec);
    
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/cornerharris.png", dst);
            
    //TEST 18
    
    src = cvLoadImage("/mnt/sdcard/OpenCVTests/data/ref_chroma_median_out_512x512_P444_8bpp.png", 1);
    dst = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);

    if( !src )
    {
        printf("Image was not loaded.\n");
        return -1;
    }

    gettimeofday(&previousTime, NULL);
    previousUsec = previousTime.tv_sec * 1000000 + previousTime.tv_usec;

    medianFilterBenchmark(src,dst);   

    gettimeofday(&currentTime,NULL);
    currentUsec = currentTime.tv_sec*1000000+currentTime.tv_usec;
    
    printf("TEST 18 Usec CPU time: %d \n",currentUsec - previousUsec);
    
    res1 = cvSaveImage("/mnt/sdcard/OpenCVTests/results/medianfilter.png", dst);
    
    
    
    
    
        
    printf("Unmounting SD card...\n");
    SDCardUnmount();
    
    printf("Tests ended \n");

    pthread_exit(0);
}


static void Fatal_extension(
  Internal_errors_Source  the_source,
  bool                    is_internal,
  uint32_t                the_error
)
{ 
    if (the_source != RTEMS_FATAL_SOURCE_EXIT)
        printk ("\nSource %d Internal %d Error %d\n", the_source, is_internal, the_error);
    if (the_source == RTEMS_FATAL_SOURCE_EXCEPTION)
        rtems_exception_frame_print((void *) the_error);
}




