#include "shaveConfiguration.h"

#define marginX 25
#define marginY 25
#define widthAdd 50
#define heightAdd 50

ccv_bbf_param_t ccv_bbf_custom = {
    .interval = 3,     /*!< Interval images between the full size image and the half size one. e.g. 2 
    will generate 2 images in between full size image and half size one: image with 
    full size, image with 5/6 size, image with 2/3 size, image with 1/2 size. */

    .min_neighbors = 2,     /*!< 0: no grouping afterwards. 1: group objects that intersects each other. > 
    1: group objects that intersects each other, and only passes these that have 
    at least **min_neighbors** intersected objects. */

    .accurate = 1,    /*!< BBF will generates 4 spatial scale variations for better accuracy. 
    Set this parameter to 0 will reduce to 1 scale variation, and thus 3 times 
    faster but lower the general accuracy of the detector. */

    .flags = 0,     /*!< CCV_BBF_NO_NESTED, if one class of object is inside another class of 
    object, this flag will reject the first object. */

    .size = {
        50,
        50,
    },     /*!< The smallest object size that will be interesting to us. */
}; 

static int _ccv_is_equal_same_class2(const void* _r1, const void* _r2, void* data)
{
    const ccv_comp_t* r1 = (const ccv_comp_t*)_r1;
    const ccv_comp_t* r2 = (const ccv_comp_t*)_r2;
    int distance = (int)(r1->rect.width * 0.25 + 0.5);

    return r2->classification.id == r1->classification.id &&
           r2->rect.x <= r1->rect.x + distance &&
           r2->rect.x >= r1->rect.x - distance &&
           r2->rect.y <= r1->rect.y + distance &&
           r2->rect.y >= r1->rect.y - distance &&
           r2->rect.width <= (int)(r1->rect.width * 1.5 + 0.5) &&
           (int)(r2->rect.width * 1.5 + 0.5) >= r1->rect.width;
}

static int _ccv_is_equal2(const void* _r1, const void* _r2, void* data)
{
    const ccv_comp_t* r1 = (const ccv_comp_t*)_r1;
    const ccv_comp_t* r2 = (const ccv_comp_t*)_r2;
    int distance = (int)(r1->rect.width * 0.25 + 0.5);

    return r2->rect.x <= r1->rect.x + distance &&
           r2->rect.x >= r1->rect.x - distance &&
           r2->rect.y <= r1->rect.y + distance &&
           r2->rect.y >= r1->rect.y - distance &&
           r2->rect.width <= (int)(r1->rect.width * 1.5 + 0.5) &&
           (int)(r2->rect.width * 1.5 + 0.5) >= r1->rect.width;
}

void drawSquare (ccv_dense_matrix_t* img, int SquareX, int SquareY, int w, u8 color) 
{
    int i=0;
    int cols = img->cols;
    int rows = img->rows;

    for (i=0; i<w; i++) {

        img->data.u8[(SquareY*cols+SquareX)+i] = color;
        img->data.u8[((SquareY-1)*cols+(SquareX))+i] = color;
        img->data.u8[((1+SquareY)*cols+(SquareX))+i] = color;

        img->data.u8[(SquareY*cols+SquareX)+(cols*i)] = color;
        img->data.u8[(SquareY*cols+SquareX-1)+(cols*i)] = color;
        img->data.u8[(SquareY*cols+SquareX+1)+(cols*i)] = color;

        img->data.u8[(SquareY*cols+SquareX+(cols*w)+i)] = color;
        img->data.u8[((SquareY-1)*cols+SquareX+(cols*w)+i)] = color;
        img->data.u8[((1+SquareY)*cols+SquareX+(cols*w)+i)] = color;

        img->data.u8[(SquareY*cols+SquareX+w)+(cols*i)] = color;
        img->data.u8[(SquareY*cols+SquareX-1+w)+(cols*i)] = color;
        img->data.u8[(SquareY*cols+SquareX+1+w)+(cols*i)] = color;

    }
}

int select_face(ccv_array_t* seq, int xCenter, int yCenter) {

    int i, index_Face;
    index_Face = 0;
    ccv_comp_t* comp;
    int currentDistance=0;
    int xCurrent, yCurrent;
    int shorterDistance = 200000;
    for (i=0; i<seq->rnum; i++) {
        comp = (ccv_comp_t*) ccv_array_get(seq, i);
        xCurrent = comp->rect.x;
        yCurrent = comp->rect.y;

        xCurrent += comp->rect.width/2;
        yCurrent += comp->rect.height/2;

        currentDistance = sqrt (pow((xCenter - xCurrent), 2) + pow((yCenter - yCurrent), 2));
        if (currentDistance < shorterDistance) {
            shorterDistance = currentDistance;
            index_Face = i;
        }
    }
    return index_Face;
}


ccv_array_t* _ccv_bbf_detect_objects_Parallel(ccv_dense_matrix_t* a, ccv_bbf_classifier_cascade_t** _cascade, int count, ccv_bbf_param_t params)
{
    int hr = a->rows / params.size.height;
    int wr = a->cols / params.size.width;
    double scale = pow(2., 1. / (params.interval + 1.));

    int next = params.interval + 1;
    int scale_upto = (int)(log((double)ccv_min(hr, wr)) / log(scale));
    
    ccv_dense_matrix_t** pyr = (ccv_dense_matrix_t**)alloca((scale_upto + next * 2) * 4 * sizeof(ccv_dense_matrix_t*));
    memset(pyr, 0, (scale_upto + next * 2) * 4 * sizeof(ccv_dense_matrix_t*));
    if (params.size.height != _cascade[0]->size.height || params.size.width != _cascade[0]->size.width)
        ccv_resample(a, &pyr[0], 0, a->rows * _cascade[0]->size.height / params.size.height, a->cols * _cascade[0]->size.width / params.size.width, CCV_INTER_AREA);
    else
        pyr[0] = a;

    int i, j, k, t, x, y, q;
    for (i = 1; i < ccv_min(params.interval + 1, scale_upto + next * 2); i++)
        ccv_resample(pyr[0], &pyr[i * 4], 0, (int)(pyr[0]->rows / pow(scale, i)), (int)(pyr[0]->cols / pow(scale, i)), CCV_INTER_AREA);
    for (i = next; i < scale_upto + next * 2; i++)
        ccv_sample_down(pyr[i * 4 - next * 4], &pyr[i * 4], 0, 0, 0);
    if (params.accurate)
        for (i = next * 2; i < scale_upto + next * 2; i++)
        {
            ccv_sample_down(pyr[i * 4 - next * 4], &pyr[i * 4 + 1], 0, 1, 0);
            ccv_sample_down(pyr[i * 4 - next * 4], &pyr[i * 4 + 2], 0, 0, 1);
            ccv_sample_down(pyr[i * 4 - next * 4], &pyr[i * 4 + 3], 0, 1, 1);
        }
    ccv_array_t* idx_seq;
    ccv_array_t* seq = ccv_array_new(sizeof(ccv_comp_t), 64, 0);
    ccv_array_t* seq2 = ccv_array_new(sizeof(ccv_comp_t), 64, 0);
    ccv_array_t* result_seq = ccv_array_new(sizeof(ccv_comp_t), 64, 0);

    /* detect in multi scale */
        
    bbfParallel0_pyr = pyr;
    bbfParallel1_pyr = pyr;
    bbfParallel2_pyr = pyr;
    bbfParallel3_pyr = pyr;
    bbfParallel4_pyr = pyr;
    bbfParallel5_pyr = pyr;
    bbfParallel6_pyr = pyr;
    bbfParallel7_pyr = pyr;
    bbfParallel8_pyr = pyr;

    for (t = 0; t < count; t++)
    {
        bbfParallel0_cascade = _cascade[t];
        bbfParallel1_cascade = _cascade[t];
        bbfParallel2_cascade = _cascade[t];
        bbfParallel3_cascade = _cascade[t];
        bbfParallel4_cascade = _cascade[t];
        bbfParallel5_cascade = _cascade[t];
        bbfParallel6_cascade = _cascade[t];
        bbfParallel7_cascade = _cascade[t];
        bbfParallel8_cascade = _cascade[t];

        ccv_array_clear(seq);

        for (i = 0; i< scale_upto; i++) {
            swcResetShave(i);
            swcSetAbsoluteDefaultStack(i);
            swcStartShaveCC (i, entryPoints[i], "iii", seq, t, i);
        }

        for (i = scale_upto-1; i>=0; i--) {
            swcWaitShave(i);
        }

        for (i = 0; i< N_SHAVES; i++) 
            DrvShaveL2CachePartitionFlushAndInvalidate(i);
        swcLeonDataCacheFlush();

        if(params.min_neighbors == 0)
        {   
            for (i = 0; i < seq->rnum; i++)
            {
                ccv_comp_t* comp = (ccv_comp_t*)ccv_array_get(seq, i);
                ccv_array_push(result_seq, comp);
            }
        } else {
            idx_seq = 0;
            ccv_array_clear(seq2);
            // group retrieved rectangles in order to filter out noise
            int ncomp = ccv_array_group(seq, &idx_seq, _ccv_is_equal_same_class2, 0);
            ccv_comp_t* comps = (ccv_comp_t*)ccmalloc((ncomp + 1) * sizeof(ccv_comp_t));
            memset(comps, 0, (ncomp + 1) * sizeof(ccv_comp_t));

            // count number of neighbors
            for(i = 0; i < seq->rnum; i++)
            {
                ccv_comp_t r1 = *(ccv_comp_t*)ccv_array_get(seq, i);
                int idx = *(int*)ccv_array_get(idx_seq, i);

                if (comps[idx].neighbors == 0)
                    comps[idx].classification.confidence = r1.classification.confidence;

                ++comps[idx].neighbors;

                comps[idx].rect.x += r1.rect.x;
                comps[idx].rect.y += r1.rect.y;
                comps[idx].rect.width += r1.rect.width;
                comps[idx].rect.height += r1.rect.height;
                comps[idx].classification.id = r1.classification.id;
                comps[idx].classification.confidence = ccv_max(comps[idx].classification.confidence, r1.classification.confidence);
            }

            // calculate average bounding box
            for(i = 0; i < ncomp; i++)
            {
                int n = comps[i].neighbors;
                if(n >= params.min_neighbors)
                {
                    ccv_comp_t comp;
                    comp.rect.x = (comps[i].rect.x * 2 + n) / (2 * n);
                    comp.rect.y = (comps[i].rect.y * 2 + n) / (2 * n);
                    comp.rect.width = (comps[i].rect.width * 2 + n) / (2 * n);
                    comp.rect.height = (comps[i].rect.height * 2 + n) / (2 * n);
                    comp.neighbors = comps[i].neighbors;
                    comp.classification.id = comps[i].classification.id;
                    comp.classification.confidence = comps[i].classification.confidence;
                    ccv_array_push(seq2, &comp);
                }
            }

            // filter out small face rectangles inside large face rectangles
            for(i = 0; i < seq2->rnum; i++)
            {
                ccv_comp_t r1 = *(ccv_comp_t*)ccv_array_get(seq2, i);
                int flag = 1;

                for(j = 0; j < seq2->rnum; j++)
                {
                    ccv_comp_t r2 = *(ccv_comp_t*)ccv_array_get(seq2, j);
                    int distance = (int)(r2.rect.width * 0.25 + 0.5);

                    if(i != j &&
                       r1.classification.id == r2.classification.id &&
                       r1.rect.x >= r2.rect.x - distance &&
                       r1.rect.y >= r2.rect.y - distance &&
                       r1.rect.x + r1.rect.width <= r2.rect.x + r2.rect.width + distance &&
                       r1.rect.y + r1.rect.height <= r2.rect.y + r2.rect.height + distance &&
                       (r2.neighbors > ccv_max(3, r1.neighbors) || r1.neighbors < 3))
                    {
                        flag = 0;
                        break;
                    }
                }

                if(flag)
                    ccv_array_push(result_seq, &r1);
            }
            ccv_array_free(idx_seq);
            ccfree(comps);
        }
    }

    ccv_array_free(seq);
    ccv_array_free(seq2);

    ccv_array_t* result_seq2;
    /* the following code from OpenCV's haar feature implementation */
    if (params.flags & CCV_BBF_NO_NESTED)
    {
        result_seq2 = ccv_array_new(sizeof(ccv_comp_t), 64, 0);
        idx_seq = 0;
        // group retrieved rectangles in order to filter out noise
        int ncomp = ccv_array_group(result_seq, &idx_seq, _ccv_is_equal2, 0);
        ccv_comp_t* comps = (ccv_comp_t*)ccmalloc((ncomp + 1) * sizeof(ccv_comp_t));
        memset(comps, 0, (ncomp + 1) * sizeof(ccv_comp_t));

        // count number of neighbors
        for(i = 0; i < result_seq->rnum; i++)
        {
            ccv_comp_t r1 = *(ccv_comp_t*)ccv_array_get(result_seq, i);
            int idx = *(int*)ccv_array_get(idx_seq, i);

            if (comps[idx].neighbors == 0 || comps[idx].classification.confidence < r1.classification.confidence)
            {
                comps[idx].classification.confidence = r1.classification.confidence;
                comps[idx].neighbors = 1;
                comps[idx].rect = r1.rect;
                comps[idx].classification.id = r1.classification.id;
            }
        }

        // calculate average bounding box
        for(i = 0; i < ncomp; i++)
            if(comps[i].neighbors)
                ccv_array_push(result_seq2, &comps[i]);

        ccv_array_free(result_seq);
        ccfree(comps);
    } else {
        result_seq2 = result_seq;
    }

    for (i = 1; i < scale_upto + next * 2; i++)
        ccv_matrix_free(pyr[i * 4]);
    if (params.accurate)
        for (i = next * 2; i < scale_upto + next * 2; i++)
        {
            ccv_matrix_free(pyr[i * 4 + 1]);
            ccv_matrix_free(pyr[i * 4 + 2]);
            ccv_matrix_free(pyr[i * 4 + 3]);
        }
    if (params.size.height != _cascade[0]->size.height || params.size.width != _cascade[0]->size.width)
        ccv_matrix_free(pyr[0]);

    return result_seq2;
}