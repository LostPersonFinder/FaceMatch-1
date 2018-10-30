
//Informational Notice:
//
//This software was developed under contract funded by the National Library of Medicine, which is part of the National Institutes of Health, an agency of the Department of Health and Human Services, United States Government.
//
//The license of this software is an open-source BSD license.  It allows use in both commercial and non-commercial products.
//
//The license does not supersede any applicable United States law.
//
//The license does not indemnify you from any claims brought by third parties whose proprietary rights may be infringed by your usage of this software.
//
//Government usage rights for this software are established by Federal law, which includes, but may not be limited to, Federal Acquisition Regulation (FAR) 48 C.F.R. Part52.227-14, Rights in Data?General.
//The license for this software is intended to be expansive, rather than restrictive, in encouraging the use of this software in both commercial and non-commercial products.
//
//LICENSE:
//
//Government Usage Rights Notice:  The U.S. Government retains unlimited, royalty-free usage rights to this software, but not ownership, as provided by Federal law.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//?	Redistributions of source code must retain the above Government Usage Rights Notice, this list of conditions and the following disclaimer.
//
//?	Redistributions in binary form must reproduce the above Government Usage Rights Notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
//?	The names,trademarks, and service marks of the National Library of Medicine, the National Cancer Institute, the National Institutes of Health, and the names of any of the software developers shall not be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE U.S. GOVERNMENT AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITEDTO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE U.S. GOVERNMENT
//OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "common.h"
#include "ColorMapper.h"
#include "ColorFaceLandmarks.h"
#include "Image.h"
#include "ListStreamer.h"
#include <fstream>

namespace FaceMatch
{
void ColorMapperCvtImg::setColorConversionCode(const string & HistColorSpace)
{
	if (HistColorSpace.empty() || HistColorSpace=="BGR") mColorConversionCode=0;
	else if (HistColorSpace=="RGB") mColorConversionCode=CV_BGR2RGB;
	else if (HistColorSpace=="HSV") mColorConversionCode=CV_BGR2HSV;
	else if (HistColorSpace=="Lab") mColorConversionCode=CV_BGR2Lab;
	else if (HistColorSpace=="YCrCb") mColorConversionCode=CV_BGR2YCrCb;
	else throw Exception("unsupported histogram color space: "+HistColorSpace);
}

Mat ColorMapper::morph(const Mat & img)
{ // TODO: use GPU, if available
	FTIMELOG
	Mat res=img.clone();
	erode(res, res, Mat()); // clean up noise
	static Mat ker=getStructuringElement(MORPH_ELLIPSE, Size(5,5)); // TODO: param/config
	dilate(res, res, ker, Point(-1,-1), 2); // close holes: two iterations with 5x5 ker are faster than one iteration with 8x8 ker
	return res;
}

void ColorMapper::getCCRegions(FaceRegions & outLikelyRegions, const Mat & bitonal, const string & ImgOutDir)
{
	FTIMELOG
	MatUC LabelImage = bitonal.clone();
	const uchar MaxLabel = -1;
	uchar label=1;
	const Rect ImgR(Point(), bitonal.size());
	{ TIMELOG("floodFillCC");
	for (int r=0; r<LabelImage.rows; ++r)
		for (int c=0; c<LabelImage.cols; ++c)
		{
			if (label==MaxLabel) break; // max label
			if (LabelImage(r,c)<label) continue;
			Rect rect;
			floodFill(LabelImage, Point(c,r), Scalar(label++), &rect);
			rect&=ImgR;
			outLikelyRegions.add(new FaceRegion("s", rect));
		}
	}
	if (getVisVerbLevel()>1) // TODO: ImgOutDir may be image specific, not global in multi-threaded environment
		imOut(ImgOutDir+"ConnComp", Mat(LabelImage*(MaxLabel/(REALNUM)label)));
}
void ColorMapper::computeColorLikelihoodMap()
{
	mColorLikelihoodMap = ColorLikelihoodMap::zeros(mSrcImg.size());
	ParallelErrorStream PES;
#pragma omp parallel for shared (PES)
	for (int r = 0; r<mSrcImg.rows; ++r)
		try
		{
			for (int c = 0; c < mSrcImg.cols; ++c)
				mColorLikelihoodMap(r, c) = computeColorLikelihood(r, c);
		}
		PCATCH(PES, format("%d-th row parallel computeColorLikelihoodMap", r))
	PES.report("parallel ColorMapper::computeColorLikelihoodMap errors");
}
void ColorMapper::init(const Image3b & img, const string & ImgOutDir)
{
	mImgOutDir=ImgOutDir;
	setImage(img);
	computeColorLikelihoodMap();
	mColorLikelihoodMap = gblur(mColorLikelihoodMap, Size(5, 5)); // TODO: param/config
	if (getVisVerbLevel()>1)
		imOut(mImgOutDir+"ColorLikelihood", MatUC(mColorLikelihoodMap*0xFF));
}
PColorMapper ColorMapper::clone(bool deep)const
{
	PColorMapper pCM = new ColorMapper();
	pCM->mSrcImg = deep ? mSrcImg : mSrcImg.clone();
	pCM->mColorLikelihoodMap = deep ? mColorLikelihoodMap : mColorLikelihoodMap.clone();
	pCM->mImgOutDir=mImgOutDir;
	return pCM;
}
PColorMapper ColorMapper::create(const Image3b & img, const string & OutImgDir)
{
	OMPLocker locker(mLock);
	init(img, OutImgDir);
	return clone();
}
void ColorMapper::getLikelyColorBlobs(FaceRegions & outLikelyRegions, REALNUM skinThreshold, bool detectLandmarks)
{
	Mat SkinMask = threshold(skinThreshold);
	unsigned VL = getVisVerbLevel();
	if (VL>1) imshow("threshold", SkinMask);
	SkinMask = morph(SkinMask);
	if (VL>1) imshow("morph", SkinMask);
	getCCRegions(outLikelyRegions, SkinMask, mImgOutDir);
	if (!detectLandmarks) return;
	for (auto & p: outLikelyRegions)
	{
		if (!isFRRectKind(p->Kind)) continue;
		FaceRegion & rgn=dynamic_cast<FaceRegion&>(*p);
		getFaceLandmarkRegions(rgn.mSubregions, enhance(rgn)); // NOTE: simple cropping may sometimes produce better results
		if (VL) clog<<rgn<<endl;
	}
}
REALNUM ColorMapper::integrate(const FaceRegion & fr)const
{
	Rect R = fr & Rect(Point(0,0), mColorLikelihoodMap.size()); // crop
	const ColorLikelihoodMap & CM = mColorLikelihoodMap(R);
	REALNUM res=0;
	if (CM.empty()) return res;
	for (int r=0; r<CM.rows; ++r)
		for (int c=0; c<CM.cols; ++c)
			res += CM(r,c);
	return res/CM.rows/CM.cols;
}
Mat ColorMapper::enhance(const FaceRegion & fr)const
{
	FTIMELOG
	Image3b src = Image::cropad(mSrcImg, fr);
	ColorLikelihoodMap CM = Image::cropad(mColorLikelihoodMap, fr);
	Image3b res = src;
	if (CM.empty()) return res;
	return res *= CM;
}
Mat ColorMapper::threshold(REALNUM tol)const
{
	const Image3b & img = mSrcImg;
	FTIMELOG
	MatUC res(img.size());
	if (mColorLikelihoodMap.empty()) return res;
	for (int r=0; r<img.rows; ++r)
		for (int c=0; c<img.cols; ++c)
			res(r,c) = mColorLikelihoodMap(r,c)<tol ? 0 : 0xFF;
	return res;
}

ColorMapperStat::ColorMapperStat(const string & FN): ColorMapperCvtImg(0)
{
	if (FN.empty()) return;
	FileStorage fs(FN, FileStorage::READ);
	if (!fs.isOpened()) throw Exception("unable to open color stat file "+FN);
	fs["ColorConversionCode"]>>mColorConversionCode;
	Mat mean; fs["mean"]>>mean;
	mMean=mean;
	if (norm(mMean)<=1)
	{
		const REALNUM scale=0xFF;
		mMean*=scale;
		mCov*=scale*scale;
	}
	fs["COV"]>>mCov;
	invert(mCov, mCovInv, DECOMP_SVD);
}

void ColorMapperStat::save(const string & FN)const
{
	if (FN.empty()) return;
	FileStorage fs(FN, FileStorage::WRITE);
	if (!fs.isOpened()) throw Exception("unable to open for writing color stat file "+FN);
	fs<<"ColorConversionCode"<<mColorConversionCode
	<<"mean"<<Mat(mMean)
	<<"COV"<<mCov;
}

/**
 * Print color samples to a text stream.
 * @param s	output stream
 * @param ColorSamples	color samples for output
 */
void print(ostream & s, const MatColorSample & ColorSamples)
{
	s<<"["<<ColorSamples.rows<<","<<ColorSamples.cols<<";"<<ColorSamples.channels()<<"]: "<<ColorSamples<<endl;
}

ColorMapperStat::ColorMapperStat(const MatColorSample & SrcPixels, int ColorConversionCode): ColorMapperCvtImg(ColorConversionCode)
{
	MatColorSample CvtPixels = SrcPixels;
	if (ColorConversionCode) CvtPixels = convert(SrcPixels, mColorConversionCode);
	Mat_<byte> samples;
	for (int r=0; r<CvtPixels.rows; ++r)
		for (int c=0; c<CvtPixels.cols; ++c)
		{
			const Vec3b & p = CvtPixels(r,c);
			Mat_<byte> s(p.t());
			samples.push_back(s);
		}
	Mat mean;
	calcCovarMatrix(samples, mCov, mean, CV_COVAR_ROWS|CV_COVAR_NORMAL|CV_COVAR_SCALE);
	mMean=mean;
	invert(mCov, mCovInv, DECOMP_SVD);
}

ColorMapperHist::ColorMapperHist(const string & FN): ColorMapperCvtImg(0)
{
	if (FN.empty()) return;
	const string & ext=getFileExt(FN);
	if (ext==".hst") // load bin data
		mColorHist.load(FN);
	else if (ext==".yml" || ext==".xml") // read a histogram
	{
		FileStorage fs(FN, FileStorage::READ);
		if (!fs.isOpened()) throw Exception("unable to open color histogram file "+FN);
		mColorHist.read(fs);
	}
	else if (ext==".tsv" || ext==".txt")
		mColorHist.loadData(FN); // assume tab separated 0/1 labeled data
	else if (ext==".xyz") // load plain text data
		mColorHist.inXYZV(FN);
	setColorConversionCode(mColorHist.getColorSpace());
}
void ColorMapperHist::save(const string & FN)const
{
	if (FN.empty()) return;
	const string & ext=getFileExt(FN);
	if (ext==".hst") // save bin data
		mColorHist.save(FN);
	else if (ext==".xyz" || ext==".ijk") // output text as x y z v tuples
		mColorHist.outXYZV(FN);
	else if (ext==".yml" || ext==".xml") // use OpenCV storage
	{
		FileStorage fs(FN, FileStorage::WRITE);
		if (!fs.isOpened()) throw Exception("unable to open for writing color histogram file "+FN);
		mColorHist.write(fs);
	}
	else
	{
		ofstream fs(FN.c_str()); // dump to a text file
		fs<<mColorHist<<endl;
	}
}
void ColorMapperANN::computeColorLikelihoodMap()
{
	FTIMELOG
	mColorLikelihoodMap = mSMDANN.getSkinLikelihoodMap(mSrcImg);
}
ColorMapperEnsemble::ColorMapperEnsemble(const string & FN, bool preferGPU)
{
	REALNUM totalWeight = 0;
	for (ListReader lr(FN); !lr.end();)
	{
		string line; trim(lr.fetch(line));
		if (line.empty() || startsWith(line, "#")) continue;
		string kind, ModelFN;
		stringstream strmLine(line);
		getline(strmLine, kind, '\t');
		getline(strmLine, ModelFN, '\t');
		REALNUM weight=1;
		if (!strmLine.eof()) strmLine >> weight;
		totalWeight += weight;
		if (weight)
			mWeightedColorMappers.push_back(WeightedColorMapper(kind,
			[&]()->ColorMapper*
			{
				if (kind == "Stat") return new ColorMapperStat(ModelFN);
				else if (kind == "Hist") return new ColorMapperHist(ModelFN);
				else if (kind == "ANN") return new ColorMapperANN(ModelFN, preferGPU);
				else throw Exception("invalid skin mapper kind: " + kind);
			}(),
			weight));
	}
	if (totalWeight && totalWeight != 1) for (auto & e : mWeightedColorMappers) e.weight /= totalWeight;
}
void ColorMapperEnsemble::setImage(const Image3b & img)
{
	TBase::setImage(img);
	int sz = mWeightedColorMappers.size();
	ParallelErrorStream PES;
#pragma omp parallel for shared (PES, img)
	for (int i = 0; i < sz; ++i)
		try	{ mWeightedColorMappers[i].pColorMapper->setImage(img); }
		PCATCH(PES, format("%i parallel set image error on %s", i, mWeightedColorMappers[i].kind.c_str()))
	PES.report("parallel set image errors");
}
void ColorMapperEnsemble::computeColorLikelihoodMap()
{
	int sz=mWeightedColorMappers.size();
	ParallelErrorStream PES;
#pragma omp parallel for shared (PES)
	for (int i = 0; i < sz; ++i)
		try { mWeightedColorMappers[i].pColorMapper->computeColorLikelihoodMap(); }
		PCATCH(PES, format("%i parallel computeColorLikelihoodMap error on %s", i, mWeightedColorMappers[i].kind.c_str()))
	PES.report("parallel computeColorLikelihoodMap errors");
	mColorLikelihoodMap = ColorLikelihoodMap::zeros(mSrcImg.size());
	for (const auto & e : mWeightedColorMappers)
		mColorLikelihoodMap += e.weight*e.pColorMapper->mColorLikelihoodMap;
}

} // namespace FaceMatch
