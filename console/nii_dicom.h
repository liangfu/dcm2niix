//#include <stdbool.h>
#include <string.h>
#include "nifti1.h"
#include "_dcm2nii.h"

#ifndef MRIpro_nii_dcm_h

#define MRIpro_nii_dcm_h

#ifdef  __cplusplus
extern "C" {
#endif

#define kDCMvers "24Nov2014"
static const int kMaxDTIv = 4000;
#define kDICOMStr  31
#define kMaxDTIv  4000
#define kMANUFACTURER_UNKNOWN  0
#define kMANUFACTURER_SIEMENS  1
#define kMANUFACTURER_GE  2
#define kMANUFACTURER_PHILIPS  3
static const int kSliceOrientUnknown = 0;
static const int kSliceOrientTra = 1;
static const int kSliceOrientSag = 2;
static const int kSliceOrientCor = 3;
static const int kSliceOrientMosaicNegativeDeterminant = 4;
    


    typedef struct TCSAdata {
        float dtiV[kMaxDTIv][4], sliceNormV[4], bandwidthPerPixelPhaseEncode, sliceMeasurementDuration;
        int numDti, multiBandFactor, sliceOrder, mosaicSlices,protocolSliceNumber1,phaseEncodingDirectionPositive;
    }TCSAdata;
    typedef struct TDICOMdata {
        long seriesNum;
        int xyzDim[5];//, xyzOri[4];
        int sliceOrient,numberOfDynamicScans, manufacturer, converted2NII, acquNum, imageNum, imageStart, bitsStored, bitsAllocated, samplesPerPixel,patientPositionSequentialRepeats,locationsInAcquisition; //
        float TE, TR,intenScale,intenIntercept, gantryTilt, lastScanLoc, angulation[4];
        float orient[7], patientPosition[4], patientPositionLast[4], xyzMM[4], stackOffcentre[4]; //patientPosition2nd[4],
        double dateTime, acquisitionTime;
        bool isValid, is3DAcq, isExplicitVR, isLittleEndian, isPlanarRGB, isSigned, isHasPhase,isHasMagnitude,isHasMixed;
        char phaseEncodingRC;
        char  patientID[kDICOMStr], patientOrient[kDICOMStr], patientName[kDICOMStr],protocolName[kDICOMStr],studyDate[kDICOMStr],studyTime[kDICOMStr], imageComments[kDICOMStr];
        struct TCSAdata CSA;
    }TDICOMdata;
    
    size_t nii_ImgBytes(struct nifti_1_header hdr);
    struct TDICOMdata readDICOMv(char * fname, bool isVerbose);
    struct TDICOMdata readDICOM(char * fname);
    struct TDICOMdata clear_dicom_data();
    unsigned char * nii_flipY(unsigned char* bImg, struct nifti_1_header *h);
    unsigned char * nii_flipZ(unsigned char* bImg, struct nifti_1_header *h);
    struct TDICOMdata  nii_readParRec (char * parname);
    //void reportMat(struct nifti_1_header h);
    int headerDcm2Nii2(struct TDICOMdata d, struct TDICOMdata d2, struct nifti_1_header *h);
    unsigned char * nii_loadImgX(char* imgname, struct nifti_1_header *hdr, struct TDICOMdata dcm, bool iVaries);
    
#ifdef  __cplusplus
}
#endif

#endif
