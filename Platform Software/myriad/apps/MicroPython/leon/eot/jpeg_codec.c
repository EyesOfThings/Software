/***************************************************************************/
/*                                                                         */
/*  File: savejpg.h                                                        */
/*  Author: bkenwright@xbdev.net                                           */
/*  URL: www.xbdev.net                                                     */
/*  Date: 19-01-06                                                         */
/*                                                                         */
/***************************************************************************/
/*
    Tiny Simplified C source of a JPEG encoder.
    A BMP truecolor to JPEG encoder

 *.bmp -> *.jpg
 */
/***************************************************************************/


#include "jpeg_codec.h"
#include <stdlib.h>
#include <string.h>

/***************************************************************************/

extern volatile unsigned char *image_to_show;
extern int image_size_in_bytes_to_show;

static BYTE bytenew = 0; // The byte that will be written in the JPG file
static SBYTE bytepos = 7; //bit position in the byte we write (bytenew)
//should be<=7 and >=0
static WORD mask[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

// The Huffman tables we'll use:
static bitstring YDC_HT[12];
static bitstring CbDC_HT[12];
static bitstring YAC_HT[256];
static bitstring CbAC_HT[256];

static BYTE *category_alloc;
static BYTE category[65535]; //Here we'll keep the category of the numbers in range: -32767..32767
static bitstring *bitcode_alloc;
static bitstring bitcode[65535]; // their bitcoded representation

//Precalculated tables for a faster YCbCr->RGB transformation
// We use a SDWORD table because we'll scale values by 2^16 and work with integers
static SDWORD YRtab[256], YGtab[256], YBtab[256];
static SDWORD CbRtab[256], CbGtab[256], CbBtab[256];
static SDWORD CrRtab[256], CrGtab[256], CrBtab[256];
static float fdtbl_Y[64];
static float fdtbl_Cb[64]; //the same with the fdtbl_Cr[64]

static WORD Ximage, Yimage; // image dimensions divisible by 8
static SBYTE YDU[64]; // This is the Data Unit of Y after YCbCr->RGB transformation
static SBYTE CbDU[64];
static SBYTE CrDU[64];
static SWORD DU_DCT[64]; // Current DU (after DCT and quantization) which we'll zigzag
static SWORD DU[64]; //zigzag reordered DU which will be Huffman coded

colorYCbCr *last_frame_buffer3;

/***************************************************************************/


void write_APP0info()
//Nothing to overwrite for APP0info
{
    writeword(APP0info.marker);
    writeword(APP0info.length);
    writebyte('J');
    writebyte('F');
    writebyte('I');
    writebyte('F');
    writebyte(0);
    writebyte(APP0info.versionhi);
    writebyte(APP0info.versionlo);
    writebyte(APP0info.xyunits);
    writeword(APP0info.xdensity);
    writeword(APP0info.ydensity);
    writebyte(APP0info.thumbnwidth);
    writebyte(APP0info.thumbnheight);
}

void write_SOF0info()
// We should overwrite width and height
{
    writeword(SOF0info.marker);
    writeword(SOF0info.length);
    writebyte(SOF0info.precision);
    writeword(SOF0info.height);
    writeword(SOF0info.width);
    writebyte(SOF0info.nrofcomponents);
    writebyte(SOF0info.IdY);
    writebyte(SOF0info.HVY);
    writebyte(SOF0info.QTY);
    writebyte(SOF0info.IdCb);
    writebyte(SOF0info.HVCb);
    writebyte(SOF0info.QTCb);
    writebyte(SOF0info.IdCr);
    writebyte(SOF0info.HVCr);
    writebyte(SOF0info.QTCr);
}

void write_DQTinfo() {
    BYTE i;
    writeword(DQTinfo.marker);
    writeword(DQTinfo.length);
    writebyte(DQTinfo.QTYinfo);
    for (i = 0; i < 64; i++) writebyte(DQTinfo.Ytable[i]);
    writebyte(DQTinfo.QTCbinfo);
    for (i = 0; i < 64; i++) writebyte(DQTinfo.Cbtable[i]);
}

void set_quant_table(BYTE *basic_table, BYTE scale_factor, BYTE *newtable)
// Set quantization table and zigzag reorder it
{
    BYTE i;
    long temp;
    for (i = 0; i < 64; i++) {
        temp = ((long) basic_table[i] * scale_factor + 50L) / 100L;
        //limit the values to the valid range
        if (temp <= 0L) temp = 1L;
        if (temp > 255L) temp = 255L; //limit to baseline range if requested
        newtable[zigzag[i]] = (BYTE) temp;
    }
}

void set_DQTinfo() {
    BYTE scalefactor = 50; // scalefactor controls the visual quality of the image
    // the smaller is, the better image we'll get, and the smaller
    // compression we'll achieve
    DQTinfo.marker = 0xFFDB;
    DQTinfo.length = 132;
    DQTinfo.QTYinfo = 0;
    DQTinfo.QTCbinfo = 1;
    set_quant_table(std_luminance_qt, scalefactor, DQTinfo.Ytable);
    set_quant_table(std_chrominance_qt, scalefactor, DQTinfo.Cbtable);
}

void write_DHTinfo() {
    BYTE i;
    writeword(DHTinfo.marker);
    writeword(DHTinfo.length);
    writebyte(DHTinfo.HTYDCinfo);
    for (i = 0; i < 16; i++) writebyte(DHTinfo.YDC_nrcodes[i]);
    for (i = 0; i <= 11; i++) writebyte(DHTinfo.YDC_values[i]);
    writebyte(DHTinfo.HTYACinfo);
    for (i = 0; i < 16; i++) writebyte(DHTinfo.YAC_nrcodes[i]);
    for (i = 0; i <= 161; i++) writebyte(DHTinfo.YAC_values[i]);
    writebyte(DHTinfo.HTCbDCinfo);
    for (i = 0; i < 16; i++) writebyte(DHTinfo.CbDC_nrcodes[i]);
    for (i = 0; i <= 11; i++) writebyte(DHTinfo.CbDC_values[i]);
    writebyte(DHTinfo.HTCbACinfo);
    for (i = 0; i < 16; i++) writebyte(DHTinfo.CbAC_nrcodes[i]);
    for (i = 0; i <= 161; i++) writebyte(DHTinfo.CbAC_values[i]);
}

void set_DHTinfo() {
    BYTE i;
    DHTinfo.marker = 0xFFC4;
    DHTinfo.length = 0x01A2;
    DHTinfo.HTYDCinfo = 0;
    for (i = 0; i < 16; i++) DHTinfo.YDC_nrcodes[i] = std_dc_luminance_nrcodes[i + 1];
    for (i = 0; i <= 11; i++) DHTinfo.YDC_values[i] = std_dc_luminance_values[i];
    DHTinfo.HTYACinfo = 0x10;
    for (i = 0; i < 16; i++) DHTinfo.YAC_nrcodes[i] = std_ac_luminance_nrcodes[i + 1];
    for (i = 0; i <= 161; i++) DHTinfo.YAC_values[i] = std_ac_luminance_values[i];
    DHTinfo.HTCbDCinfo = 1;
    for (i = 0; i < 16; i++) DHTinfo.CbDC_nrcodes[i] = std_dc_chrominance_nrcodes[i + 1];
    for (i = 0; i <= 11; i++) DHTinfo.CbDC_values[i] = std_dc_chrominance_values[i];
    DHTinfo.HTCbACinfo = 0x11;
    for (i = 0; i < 16; i++) DHTinfo.CbAC_nrcodes[i] = std_ac_chrominance_nrcodes[i + 1];
    for (i = 0; i <= 161; i++) DHTinfo.CbAC_values[i] = std_ac_chrominance_values[i];
}

void write_SOSinfo()
//Nothing to overwrite for SOSinfo
{
    writeword(SOSinfo.marker);
    writeword(SOSinfo.length);
    writebyte(SOSinfo.nrofcomponents);
    writebyte(SOSinfo.IdY);
    writebyte(SOSinfo.HTY);
    writebyte(SOSinfo.IdCb);
    writebyte(SOSinfo.HTCb);
    writebyte(SOSinfo.IdCr);
    writebyte(SOSinfo.HTCr);
    writebyte(SOSinfo.Ss);
    writebyte(SOSinfo.Se);
    writebyte(SOSinfo.Bf);
}

void write_comment(BYTE *comment) {
    WORD i, length;
    writeword(0xFFFE); //The COM marker
    length = (WORD) strlen((const char *) comment);
    writeword(length + 2);
    for (i = 0; i < length; i++) writebyte(comment[i]);
}

void writebits(bitstring bs)
// A portable version; it should be done in assembler
{
    WORD value;
    SBYTE posval; //bit position in the bitstring we read, should be<=15 and >=0
    value = bs.value;
    posval = bs.length - 1;
    while (posval >= 0) {
        if (value & mask[posval]) bytenew |= mask[bytepos];
        posval--;
        bytepos--;
        if (bytepos < 0) {
            if (bytenew == 0xFF) {
                writebyte(0xFF);
                writebyte(0);
            } else {
                writebyte(bytenew);
            }

            bytepos = 7;
            bytenew = 0;
        }
    }
}

void compute_Huffman_table(BYTE *nrcodes, BYTE *std_table, bitstring *HT) {
    BYTE k, j;
    BYTE pos_in_table;
    WORD codevalue;
    codevalue = 0;
    pos_in_table = 0;
    for (k = 1; k <= 16; k++) {
        for (j = 1; j <= nrcodes[k]; j++) {
            HT[std_table[pos_in_table]].value = codevalue;
            HT[std_table[pos_in_table]].length = k;
            pos_in_table++;
            codevalue++;
        }
        codevalue *= 2;
    }
}

void init_Huffman_tables() {
    compute_Huffman_table(std_dc_luminance_nrcodes, std_dc_luminance_values, YDC_HT);
    compute_Huffman_table(std_dc_chrominance_nrcodes, std_dc_chrominance_values, CbDC_HT);
    compute_Huffman_table(std_ac_luminance_nrcodes, std_ac_luminance_values, YAC_HT);
    compute_Huffman_table(std_ac_chrominance_nrcodes, std_ac_chrominance_values, CbAC_HT);
}

void set_numbers_category_and_bitcode() {
    SDWORD nr;
    SDWORD nrlower, nrupper;
    BYTE cat;


    /*category_alloc=NULL;
    while(category_alloc==NULL)
       category_alloc=(BYTE *)malloc(65535*sizeof(BYTE));
    bitcode_alloc=NULL;
    while(bitcode_alloc==NULL)
           bitcode_alloc=(bitstring *)malloc(65535*sizeof(bitstring));*/


    /*BYTE ca[65535];
    bitstring bca[65535];
    
    category_alloc=(BYTE *)ca;
    bitcode_alloc=(bitstring *)bca;
    
    
    category=category_alloc+32767; //allow negative subscripts
    bitcode=bitcode_alloc+32767;*/



    nrlower = 1;
    nrupper = 2;
    for (cat = 1; cat <= 15; cat++) {
        //Positive numbers
        for (nr = nrlower; nr < nrupper; nr++) {
            category[nr + 32767] = cat;
            bitcode[nr + 32767].length = cat;
            bitcode[nr + 32767].value = (WORD) nr;
        }
        //Negative numbers
        for (nr = -(nrupper - 1); nr <= -nrlower; nr++) {
            category[nr + 32767] = cat;
            bitcode[nr + 32767].length = cat;
            bitcode[nr + 32767].value = (WORD) (nrupper - 1 + nr);
        }
        nrlower <<= 1;
        nrupper <<= 1;
    }
}

void precalculate_YCbCr_tables() {
    WORD R, G, B;
    for (R = 0; R <= 255; R++) {
        YRtab[R] = (SDWORD) (65536 * 0.299 + 0.5) * R;
        CbRtab[R] = (SDWORD) (65536 * -0.16874 + 0.5) * R;
        CrRtab[R] = (SDWORD) (32768) * R;
    }
    for (G = 0; G <= 255; G++) {
        YGtab[G] = (SDWORD) (65536 * 0.587 + 0.5) * G;
        CbGtab[G] = (SDWORD) (65536 * -0.33126 + 0.5) * G;
        CrGtab[G] = (SDWORD) (65536 * -0.41869 + 0.5) * G;
    }
    for (B = 0; B <= 255; B++) {
        YBtab[B] = (SDWORD) (65536 * 0.114 + 0.5) * B;
        CbBtab[B] = (SDWORD) (32768) * B;
        CrBtab[B] = (SDWORD) (65536 * -0.08131 + 0.5) * B;
    }
}

// Using a bit modified form of the FDCT routine from IJG's C source:
// Forward DCT routine idea taken from Independent JPEG Group's C source for
// JPEG encoders/decoders

// For float AA&N IDCT method, divisors are equal to quantization
//   coefficients scaled by scalefactor[row]*scalefactor[col], where
//   scalefactor[0] = 1
//   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
//   We apply a further scale factor of 8.
//   What's actually stored is 1/divisor so that the inner loop can
//   use a multiplication rather than a division.

void prepare_quant_tables() {
    double aanscalefactor[8] = {1.0, 1.387039845, 1.306562965, 1.175875602,
        1.0, 0.785694958, 0.541196100, 0.275899379};
    BYTE row, col;
    BYTE i = 0;
    for (row = 0; row < 8; row++) {
        for (col = 0; col < 8; col++) {
            fdtbl_Y[i] = (float) (1.0 / ((double) DQTinfo.Ytable[zigzag[i]] *
                    aanscalefactor[row] * aanscalefactor[col] * 8.0));
            fdtbl_Cb[i] = (float) (1.0 / ((double) DQTinfo.Cbtable[zigzag[i]] *
                    aanscalefactor[row] * aanscalefactor[col] * 8.0));
            i++;
        }
    }
}

void fdct_and_quantization(SBYTE *data, float *fdtbl, SWORD *outdata) {
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z1, z2, z3, z4, z5, z11, z13;
    float *dataptr;
    float datafloat[64];
    float temp;
    SBYTE ctr;
    BYTE i;
    for (i = 0; i < 64; i++) datafloat[i] = data[i];

    // Pass 1: process rows.
    dataptr = datafloat;
    for (ctr = 7; ctr >= 0; ctr--) {
        tmp0 = dataptr[0] + dataptr[7];
        tmp7 = dataptr[0] - dataptr[7];
        tmp1 = dataptr[1] + dataptr[6];
        tmp6 = dataptr[1] - dataptr[6];
        tmp2 = dataptr[2] + dataptr[5];
        tmp5 = dataptr[2] - dataptr[5];
        tmp3 = dataptr[3] + dataptr[4];
        tmp4 = dataptr[3] - dataptr[4];

        // Even part

        tmp10 = tmp0 + tmp3; // phase 2
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[0] = tmp10 + tmp11; // phase 3
        dataptr[4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * ((float) 0.707106781); // c4
        dataptr[2] = tmp13 + z1; // phase 5
        dataptr[6] = tmp13 - z1;

        // Odd part

        tmp10 = tmp4 + tmp5; // phase 2
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        // The rotator is modified from fig 4-8 to avoid extra negations
        z5 = (tmp10 - tmp12) * ((float) 0.382683433); // c6
        z2 = ((float) 0.541196100) * tmp10 + z5; // c2-c6
        z4 = ((float) 1.306562965) * tmp12 + z5; // c2+c6
        z3 = tmp11 * ((float) 0.707106781); // c4

        z11 = tmp7 + z3; // phase 5
        z13 = tmp7 - z3;

        dataptr[5] = z13 + z2; // phase 6
        dataptr[3] = z13 - z2;
        dataptr[1] = z11 + z4;
        dataptr[7] = z11 - z4;

        dataptr += 8; //advance pointer to next row
    }

    // Pass 2: process columns

    dataptr = datafloat;
    for (ctr = 7; ctr >= 0; ctr--) {
        tmp0 = dataptr[0] + dataptr[56];
        tmp7 = dataptr[0] - dataptr[56];
        tmp1 = dataptr[8] + dataptr[48];
        tmp6 = dataptr[8] - dataptr[48];
        tmp2 = dataptr[16] + dataptr[40];
        tmp5 = dataptr[16] - dataptr[40];
        tmp3 = dataptr[24] + dataptr[32];
        tmp4 = dataptr[24] - dataptr[32];

        //Even part/

        tmp10 = tmp0 + tmp3; //phase 2
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[0] = tmp10 + tmp11; // phase 3
        dataptr[32] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * ((float) 0.707106781); // c4
        dataptr[16] = tmp13 + z1; // phase 5
        dataptr[48] = tmp13 - z1;

        // Odd part

        tmp10 = tmp4 + tmp5; // phase 2
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        // The rotator is modified from fig 4-8 to avoid extra negations.
        z5 = (tmp10 - tmp12) * ((float) 0.382683433); // c6
        z2 = ((float) 0.541196100) * tmp10 + z5; // c2-c6
        z4 = ((float) 1.306562965) * tmp12 + z5; // c2+c6
        z3 = tmp11 * ((float) 0.707106781); // c4

        z11 = tmp7 + z3; // phase 5
        z13 = tmp7 - z3;
        dataptr[40] = z13 + z2; // phase 6
        dataptr[24] = z13 - z2;
        dataptr[8] = z11 + z4;
        dataptr[56] = z11 - z4;

        dataptr++; // advance pointer to next column
    }

    // Quantize/descale the coefficients, and store into output array
    for (i = 0; i < 64; i++) {
        // Apply the quantization and scaling factor
        temp = datafloat[i] * fdtbl[i];

        //Round to nearest integer.
        //Since C does not specify the direction of rounding for negative
        //quotients, we have to force the dividend positive for portability.
        //The maximum coefficient size is +-16K (for 12-bit data), so this
        //code should work for either 16-bit or 32-bit ints.

        outdata[i] = (SWORD) ((SWORD) (temp + 16384.5) - 16384);
    }
}

void process_DU(SBYTE *ComponentDU, float *fdtbl, SWORD *DC,
        bitstring *HTDC, bitstring *HTAC) {
    bitstring EOB = HTAC[0x00];
    bitstring M16zeroes = HTAC[0xF0];
    BYTE i;
    BYTE startpos;
    BYTE end0pos;
    BYTE nrzeroes;
    BYTE nrmarker;
    SWORD Diff;



    fdct_and_quantization(ComponentDU, fdtbl, DU_DCT);
    //zigzag reorder
    for (i = 0; i <= 63; i++) DU[zigzag[i]] = DU_DCT[i];
    Diff = DU[0]-*DC;
    *DC = DU[0];
    //Encode DC
    if (Diff == 0) {
        writebits(HTDC[0]); //Diff might be 0
    } else {
        writebits(HTDC[category[Diff + 32767]]);
        writebits(bitcode[Diff + 32767]);
    }
    //Encode ACs
    for (end0pos = 63; (end0pos > 0)&&(DU[end0pos] == 0); end0pos--);
    //end0pos = first element in reverse order !=0
    if (end0pos == 0) {
        writebits(EOB);
        return;
    }

    i = 1;
    while (i <= end0pos) {
        startpos = i;
        for (; (DU[i] == 0)&&(i <= end0pos); i++);
        nrzeroes = i - startpos;
        if (nrzeroes >= 16) {
            for (nrmarker = 1; nrmarker <= nrzeroes / 16; nrmarker++) writebits(M16zeroes);
            nrzeroes = nrzeroes % 16;
        }
        writebits(HTAC[nrzeroes * 16 + category[DU[i] + 32767]]);
        writebits(bitcode[DU[i] + 32767]);
        i++;
    }

    if (end0pos != 63) writebits(EOB);
}

void load_data_units_from_YCbCr_buffer(colorYCbCr *imageBuffer, WORD xpos, WORD ypos) {
    BYTE x, y;
    BYTE pos = 0;
    DWORD location;
    location = ypos * Ximage + xpos;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            YDU[pos] = imageBuffer[location].Y;
            CbDU[pos] = imageBuffer[location].Cb;
            CrDU[pos] = imageBuffer[location].Cr;
            location++;
            pos++;
        }
        location += Ximage - 8;
    }
}

void load_data_units_from_YCbCr_buffer2(colorYCbCr *imageBuffer, WORD xpos, WORD ypos) {
    BYTE x, y;
    BYTE pos = 0;
    DWORD location;
    location = ypos * Ximage + xpos;
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (x % 2 == 0 && y % 2 == 0) {
                YDU[pos] = imageBuffer[location].Y;
                CbDU[pos] = imageBuffer[location].Cb;
                CrDU[pos] = imageBuffer[location].Cr;
                pos++;
            }
            location++;
        }
        location += Ximage - 16;
    }
}

void main_encoder(colorYCbCr *imageBuffer) {
    SWORD DCY = 0, DCCb = 0, DCCr = 0; //DC coefficients used for differential encoding
    WORD xpos, ypos;
    for (ypos = 0; ypos < Yimage - 15; ypos += 16)
        for (xpos = 0; xpos < Ximage - 15; xpos += 16) {

            load_data_units_from_YCbCr_buffer(imageBuffer, xpos, ypos);
            process_DU(YDU, fdtbl_Y, &DCY, YDC_HT, YAC_HT);
            load_data_units_from_YCbCr_buffer(imageBuffer, xpos + 8, ypos);
            process_DU(YDU, fdtbl_Y, &DCY, YDC_HT, YAC_HT);
            load_data_units_from_YCbCr_buffer(imageBuffer, xpos, ypos + 8);
            process_DU(YDU, fdtbl_Y, &DCY, YDC_HT, YAC_HT);
            load_data_units_from_YCbCr_buffer(imageBuffer, xpos + 8, ypos + 8);
            process_DU(YDU, fdtbl_Y, &DCY, YDC_HT, YAC_HT);

            load_data_units_from_YCbCr_buffer2(imageBuffer, xpos, ypos);
            process_DU(CbDU, fdtbl_Cb, &DCCb, CbDC_HT, CbAC_HT);
            process_DU(CrDU, fdtbl_Cb, &DCCr, CbDC_HT, CbAC_HT);
        }
}

void init_all() {
    set_DQTinfo();
    set_DHTinfo();
    init_Huffman_tables();
    set_numbers_category_and_bitcode();
    precalculate_YCbCr_tables();
    prepare_quant_tables();
}

void SaveJpgFile(colorYCbCr *imageBuffer, WORD Ximage_original, WORD Yimage_original) {
    bitstring fillbits; //filling bitstring for the bit alignment of the EOI marker
    init_all();
    SOF0info.width = Ximage_original;
    SOF0info.height = Yimage_original;
    Ximage = Ximage_original;
    Yimage = Yimage_original;

    bytenew = 0;
    bytepos = 7;
    main_encoder(imageBuffer);
    //Do the bit alignment of the EOI marker
    if (bytepos >= 0) {
        fillbits.length = bytepos + 1;
        fillbits.value = (1 << (bytepos + 1)) - 1;
        writebits(fillbits);
    }
    writeword(0xFFD9); //EOI
    free(category_alloc);
    free(bitcode_alloc);
}

void convert2Jpeg(colorYCbCr *imageBuffer, WORD Ximage_original, WORD Yimage_original) {

    image_size_in_bytes_to_show = 0;
    bitstring fillbits; //filling bitstring for the bit alignment of the EOI marker
    init_all();
    SOF0info.width = Ximage_original;
    SOF0info.height = Yimage_original;
    Ximage = Ximage_original;
    Yimage = Yimage_original;

    writeword(0xFFD8); //SOI
    write_APP0info();

    write_DQTinfo();
    write_SOF0info();
    write_DHTinfo();
    write_SOSinfo();

    bytenew = 0;
    bytepos = 7;
    main_encoder(imageBuffer);
    //Do the bit alignment of the EOI marker
    if (bytepos >= 0) {
        fillbits.length = bytepos + 1;
        fillbits.value = (1 << (bytepos + 1)) - 1;
        writebits(fillbits);
    }
    writeword(0xFFD9); //EOI
    free(category_alloc);
    free(bitcode_alloc);
}

void convertDenseMatrix2Jpeg(ccv_dense_matrix_t* img_src) {
    image_size_in_bytes_to_show = 0;
    if(image_to_show==0) image_to_show = (unsigned char *) malloc(((img_src)->rows)*((img_src)->cols) * sizeof(unsigned char));
    else image_to_show = (unsigned char *) realloc(image_to_show, ((img_src)->rows)*((img_src)->cols) * sizeof(unsigned char));
    bitstring fillbits; //filling bitstring for the bit alignment of the EOI marker
    init_all();
    SOF0info.width = img_src->cols;
    SOF0info.height = img_src->rows;
    Ximage = img_src->cols;
    Yimage = img_src->rows;

    writeword(0xFFD8); //SOI
    write_APP0info();

    write_DQTinfo();
    write_SOF0info();
    write_DHTinfo();
    write_SOSinfo();

    bytenew = 0;
    bytepos = 7;
    int i = 0;
    if (CCV_GET_CHANNEL((img_src)->type) == CCV_C1) {
        ccv_dense_matrix_t *img = 0;
        ccv_visualize(img_src, &img, 0); // third param is not used
        int len = ((img)->rows)*((img)->cols);
        last_frame_buffer3 = (colorYCbCr *) malloc(len * sizeof (colorYCbCr));
        for (i = 0; i < len; i++) {
            last_frame_buffer3[i].Y = (img)->data.u8[i] - 0x80;
            last_frame_buffer3[i].Cb = 0x00;
            last_frame_buffer3[i].Cr = 0x00;
        }
    } else {
        int ptr = 0;
        int r, g, b;
        int len = ((img_src)->rows)*((img_src)->cols);
        last_frame_buffer3 = (colorYCbCr *) malloc(len * sizeof (colorYCbCr));
        for (i = 0; i < len; i++) {
            r = (img_src)->data.u8[ptr++];
            g = (img_src)->data.u8[ptr++];
            b = (img_src)->data.u8[ptr++];
            last_frame_buffer3[i].Y = Y(r, g, b);
            last_frame_buffer3[i].Cb = Cb(r, g, b);
            last_frame_buffer3[i].Cr = Cr(r, g, b);
        }
    }
    main_encoder(last_frame_buffer3);
    //Do the bit alignment of the EOI marker
    if (bytepos >= 0) {
        fillbits.length = bytepos + 1;
        fillbits.value = (1 << (bytepos + 1)) - 1;
        writebits(fillbits);
    }
    writeword(0xFFD9); //EOI
    free(last_frame_buffer3);
    free(category_alloc);
    free(bitcode_alloc);
}
