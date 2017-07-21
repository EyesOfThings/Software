#include <svuCommonShave.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bbfCCV.h"

int flag;

extern ccv_dense_matrix_t** pyr;
extern ccv_bbf_classifier_cascade_t* cascade;
void __attribute__((section(".text.entrypoint\n.salign 16")))

bbfParallel (ccv_array_t* seq, int t, int i)
{	
	int next = params.interval + 1;
	double scale = pow(2., 1. / (params.interval + 1.));
	int k=0;
	int j=0;
	int x=0;
	int y=0;
	int q=0;
	int idx =0;
	float scale_x = (float) params.size.width / (float) cascade->size.width;
	float scale_y = (float) params.size.height / (float) cascade->size.height;

	for (idx = 0; idx<i; idx++) {
		scale_x*=scale;
		scale_y*=scale;
	}

	int dx[] = {0, 1, 0, 1};
	int dy[] = {0, 0, 1, 1};
	int i_rows = pyr[i * 4 + next * 8]->rows - (cascade->size.height >> 2);
	int steps[] = { pyr[i * 4]->step, pyr[i * 4 + next * 4]->step, pyr[i * 4 + next * 8]->step };
	int i_cols = pyr[i * 4 + next * 8]->cols - (cascade->size.width >> 2);
	int paddings[] = { pyr[i * 4]->step * 4 - i_cols * 4,
							   pyr[i * 4 + next * 4]->step * 2 - i_cols * 2,
							   pyr[i * 4 + next * 8]->step - i_cols };
	
	for (q = 0; q < (params.accurate ? 4 : 1); q++)
	{
		unsigned char* u8[] = { pyr[i * 4]->data.u8 + dx[q] * 2 + dy[q] * pyr[i * 4]->step * 2, pyr[i * 4 + next * 4]->data.u8 + dx[q] + dy[q] * pyr[i * 4 + next * 4]->step, pyr[i * 4 + next * 8 + q]->data.u8 };
		for (y = 0; y< i_rows; y++) 
		{
			for (x = 0; x < i_cols; x++)
			{
				float sum;
				ccv_bbf_stage_classifier_t* classifier = cascade->stage_classifier;
				flag = 1;
				for (j=0; j < cascade->count; ++j, ++classifier) {
					sum = 0;
					float* alpha = classifier->alpha;
					ccv_bbf_feature_t* feature = classifier->feature;
					
					for (k=0; k < classifier->count; ++k, alpha += 2, ++feature){
						sum+= alpha[_SHAVE_ccv_run_bbf_feature(feature, steps, u8)];
					}
					if (sum < classifier->threshold)
					{
						flag = 0;
						break;
					}
				}
				if (flag)
				{	
					ccv_comp_t comp;
					comp.rect = ccv_rect((int)((x * 4 + dx[q] * 2) * scale_x + 0.5), (int)((y * 4 + dy[q] * 2) * scale_y + 0.5), (int)(cascade->size.width * scale_x + 0.5), (int)(cascade->size.height * scale_y + 0.5));
					comp.neighbors = 1;
					comp.classification.id = t;
					comp.classification.confidence = sum;
					ccv_array_push(seq, &comp);

				}
				u8[0] += 4;
				u8[1] += 2;
				u8[2] += 1;
			}
			u8[0] += paddings[0];
			u8[1] += paddings[1];
			u8[2] += paddings[2];
		}
	}
	
	SHAVE_HALT;
	return;
}