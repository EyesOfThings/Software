#define CCV_BBF_POINT_MAX (8)
#define CCV_BBF_POINT_MIN (3)

#define ccv_max(a, b) (((a) > (b)) ? (a) : (b))
#define ccv_array_get(a, i) (((char*)((a)->data)) + (size_t)(a)->rsize * (size_t)(i))

#define ccrealloc realloc

typedef struct {
	int width;
	int height;
} ccv_size_t;

typedef struct {
	int interval; /**< Interval images between the full size image and the half size one. e.g. 2 will generate 2 images in between full size image and half size one: image with full size, image with 5/6 size, image with 2/3 size, image with 1/2 size. */
	int min_neighbors; /**< 0: no grouping afterwards. 1: group objects that intersects each other. > 1: group objects that intersects each other, and only passes these that have at least **min_neighbors** intersected objects. */
	int flags; /**< CCV_BBF_NO_NESTED, if one class of object is inside another class of object, this flag will reject the first object. */
	int accurate; /**< BBF will generates 4 spatial scale variations for better accuracy. Set this parameter to 0 will reduce to 1 scale variation, and thus 3 times faster but lower the general accuracy of the detector. */
	ccv_size_t size; /**< The smallest object size that will be interesting to us. */
} ccv_bbf_param_t;

typedef union {
	unsigned char* u8;
	int* i32;
	float* f32;
	int64_t* i64;
	double* f64;
} ccv_matrix_cell_t;

typedef struct {
	int type;
	uint64_t sig;
	int refcount;
	int rows;
	int cols;
	int step;
	union {
		unsigned char u8;
		int i32;
		float f32;
		int64_t i64;
		double f64;
		void* p;
	} tag;
	ccv_matrix_cell_t data;
} ccv_dense_matrix_t;


typedef struct {
	int type;
	uint64_t sig;
	int refcount;
	int rnum;
	int size;
	int rsize;
	void* data;
} ccv_array_t;

typedef struct {
	int x;
	int y;
	int width;
	int height;
} ccv_rect_t;

typedef struct {
	int id;
	float confidence;
} ccv_classification_t;

typedef struct {
	ccv_rect_t rect;
	int neighbors;
	ccv_classification_t classification;
} ccv_comp_t;


ccv_bbf_param_t params = {
	.interval = 3,
	.min_neighbors = 2,
	.accurate = 1,
	.flags = 0,
	.size = {
		50,
		50,
	},
};

typedef struct {
	int size;
	int px[CCV_BBF_POINT_MAX];
	int py[CCV_BBF_POINT_MAX];
	int pz[CCV_BBF_POINT_MAX];
	int nx[CCV_BBF_POINT_MAX];
	int ny[CCV_BBF_POINT_MAX];
	int nz[CCV_BBF_POINT_MAX];
} ccv_bbf_feature_t;

typedef struct {
	int count;
	float threshold;
	ccv_bbf_feature_t* feature;
	float* alpha;
} ccv_bbf_stage_classifier_t;



typedef struct {
	int count;
	ccv_size_t size;
	ccv_bbf_stage_classifier_t* stage_classifier;
} ccv_bbf_classifier_cascade_t;

unsigned char pf_at (int i, ccv_bbf_feature_t* feature, int* step, unsigned char** u8) {
	unsigned char v = *(u8[feature->pz[i]] + feature->px[i] + feature->py[i] * step[feature->pz[i]]);
	return v;
}

unsigned char nf_at(int i, ccv_bbf_feature_t* feature, int* step, unsigned char** u8) {
	unsigned char v2 =  *(u8[feature->nz[i]] + feature->nx[i] + feature->ny[i] * step[feature->nz[i]]);
	return v2;
}

ccv_rect_t ccv_rect(int x, int y, int width, int height)
{
	ccv_rect_t rect;
	rect.x = x;
	rect.y = y;
	rect.width = width;
	rect.height = height;
	return rect;
}

void ccv_array_push(ccv_array_t* array, void* r)
{
	array->rnum++;
	if (array->rnum > array->size)
	{
		array->size = ccv_max(array->size * 3 / 2, array->size + 1);
		array->data = ccrealloc(array->data, (size_t)array->size * (size_t)array->rsize);
	}
	memcpy(ccv_array_get(array, array->rnum - 1), r, array->rsize);
}

inline static int _SHAVE_ccv_run_bbf_feature(ccv_bbf_feature_t* feature, int* step, unsigned char** u8) {
	unsigned char pmin = pf_at(0, feature, step, u8);
	unsigned char nmax = nf_at(0, feature, step, u8);
	if (pmin <= nmax) {
		return 0;
	}
	int i;
	for (i = 1; i < feature->size; i++)
	{
		if (feature->pz[i] >= 0)
		{
			int p = pf_at(i, feature, step, u8);
			if (p < pmin)
			{

				if (p <= nmax) {
					return 0;
				}
				pmin = p;
			}
		}
		if (feature->nz[i] >= 0)
		{
			int n = nf_at(i, feature, step, u8);
			if (n > nmax)
			{
				if (pmin <= n) {
					return 0;
				} 
				nmax = n;
			}
		}
	}
	return 1;
}