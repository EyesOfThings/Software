#define N_SHAVES 9

ccv_dense_matrix_t** bbfParallel0_pyr;
ccv_dense_matrix_t** bbfParallel1_pyr;
ccv_dense_matrix_t** bbfParallel2_pyr;
ccv_dense_matrix_t** bbfParallel3_pyr;
ccv_dense_matrix_t** bbfParallel4_pyr;
ccv_dense_matrix_t** bbfParallel5_pyr;
ccv_dense_matrix_t** bbfParallel6_pyr;
ccv_dense_matrix_t** bbfParallel7_pyr;
ccv_dense_matrix_t** bbfParallel8_pyr;

ccv_bbf_classifier_cascade_t* bbfParallel0_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel1_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel2_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel3_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel4_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel5_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel6_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel7_cascade;
ccv_bbf_classifier_cascade_t* bbfParallel8_cascade;

extern u32 bbfParallel0_bbfParallel;
extern u32 bbfParallel1_bbfParallel;
extern u32 bbfParallel2_bbfParallel;
extern u32 bbfParallel3_bbfParallel;
extern u32 bbfParallel4_bbfParallel;
extern u32 bbfParallel5_bbfParallel;
extern u32 bbfParallel6_bbfParallel;
extern u32 bbfParallel7_bbfParallel;
extern u32 bbfParallel8_bbfParallel;

u32 entryPoints [N_SHAVES] = {
	(u32)&bbfParallel0_bbfParallel,
	(u32)&bbfParallel1_bbfParallel,
	(u32)&bbfParallel2_bbfParallel,
	(u32)&bbfParallel3_bbfParallel,
	(u32)&bbfParallel4_bbfParallel,
	(u32)&bbfParallel5_bbfParallel,
	(u32)&bbfParallel6_bbfParallel,
	(u32)&bbfParallel7_bbfParallel,
	(u32)&bbfParallel8_bbfParallel,
};

static swcShaveUnit_t SHAVE[N_SHAVES] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
