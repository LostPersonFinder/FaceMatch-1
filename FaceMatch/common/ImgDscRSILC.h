
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

#pragma once // 2013-2016 (C) FaceMatch@NIH.gov

#include "Image.h"
#include "ImgDscBase.h"

namespace FaceMatch
{
/// Rotation and Scale Invariant Line Color (RSILC) descriptor
class LIBDCL ImgDscRSILC: public ImgDscBase
{
	typedef ImgDscBase TBase;
public:
	/// key-line descriptor vector
	typedef vector<REALNUM> VectorReal;
	/// key-line descriptor row
	typedef MatReal DescriptorRow;
	/// collection of key-line descriptors
	typedef MatReal Descriptor;
	/// key-line
	typedef KeyPoint KeyLine;
	/// collection of key-lines
	typedef vector<KeyLine> KeyLines;

protected:
	bool mUseSpatialInfo;
	KeyLines mKeyLines;
	Descriptor mDescriptor;

	/**
	 * Initialize RSILC descriptor by getting the src and dimension and computing the descriptor (array) for the image.
	 * @param src source face image, assumed to be equalized
	 * @param dim diameter for the normalized image, if 0, skip the normalization
	 */
	void init(const Image & src, unsigned dim); //initialize the descriptor from the image
	/**
	 * Compute the coordinates of each line and store them in mLINES.
	 * @param SrcImg a gray-scale source image
	 * @param LineImg a binary image which contains the lines at specific angle
	 * @param LineIndex key-line filter index
	 */
	void ComputeLineCoords(const MatUC & SrcImg, const Mat & LineImg, int LineIndex);
	/**
	 * Take the gray scale image as input, compute edges by Canny edge detector, and apply line filters to create directional lines.
	 * Compute the coordinates of the prominent lines and save the coordinates into "LINES" array.
	 * LINES is a structure and holds the center, line length and angle of each line.
	 * @param img input/output gray-scale image
	 */
	void ApplyLineFilters(Mat & img);
	/// Compute the histograms of each key-line circle. The histograms are calculated for each subsection.
	void ComputeHistogramROI(VectorReal & hLine, ///< output descriptor row
		const Mat &crop_Img_Y, ///< Y band of the cropped image
		const Mat &crop_Img_Cb, ///< Cb band of the cropped image
		const Mat &crop_Img_Cr, ///< Cr band of the cropped image
		const Mat &crop_Grad_Angle, ///< cropped image gradient angle
		const Mat &crop_Grad_Mag, ///< cropped image gradient magnitude
		const Mat &crop_pixel_angle, ///< cropped image pixel angle
		const Mat &SectionMatrix, ///< section matrix
		const KeyLine &Main_Line, ///< major key line
		int SecCnt ///< sectors count
	);
	void drawKeyLines(Image img);
	/// Compute the spatial features for each section inside the circle.
	void SpatialInfo(vector<float> &h_angdif, vector<float> &h_nofLines, const KeyLine &Main_Line, const MatInt &SectionMatrix, int sec_no, int Nof_spt_bins);

public:
	/**
	 * Instantiate.
	 * @param s a binary stream to read values from
	 */
	ImgDscRSILC(istream & s);
	/**
	 * Instantiate.
	 * @param fn an X/YML file node to read values from
	 */
	ImgDscRSILC(const FileNode & fn);
	/**
	 * Instantiate.
	 * @param src source image/face patch
	 * @param dim face/image patch diameter to transform source to
	 * @param bUseSpatialInfo use the spatial information?
	 */
	ImgDscRSILC(const Image & src, unsigned dim=0, bool bUseSpatialInfo=true);
	/**
	 * Instantiate.
	 * @param pSrc source image/face patch
	 * @param dim face/image patch diameter to transform source to
	 * @param bUseSpatialInfo use the spatial information?
	 */
	ImgDscRSILC(const Image * pSrc=0, unsigned dim=DefaultFacePatchDim, bool bUseSpatialInfo=true);
	/// @return reference to key lines
	const KeyLines & getKeyLines()const{ return mKeyLines; }
	/**
	 * Compute the distance to another descriptor.
	 * @param a another descriptor
	 * @param matches (optional) pointer to the produced matches, e.g. for visualization
	 */
	virtual REALNUM dist(const ImgDscBase & a, Matches * matches=0)const override;
	virtual REALNUM getDistScale(bool OM=false)const override;
	virtual void setDistScale(REALNUM s, bool OM=false)const override;
	/**
	 * Get descriptor values.
	 * @return matrix with rows corresponding to the rows of descriptor
	 */
	virtual const Mat getVectors()const override;
	/**
	 * Is the descriptor empty?
	 * @return emty state
	 */
	virtual bool empty()const override;
	/**
	 * Write the descriptor.
	 * @param s output binary stream
	 */
	virtual void write(ostream & s)const override;
	virtual void print(ostream & s, const string & fmt="")const override;
	/**
	 * Write the descriptor.
	 * @param fs output X/YML stream
	 */
	virtual void write(FileStorage& fs)const override;
	/**
	 * Get descriptor type.
	 * @return string with the type
	 */
	static const string & Type()
	{
		StaticLkdCtor const string type="ImgDscRSILC";
		return type;
	}
	/**
	 * Get descriptor type.
	 * @return string with the type
	 */
	virtual const string & getType()const override {return Type();}
};

/// RSILC short: without the spatial information
class LIBDCL ImgDscRSILCS: public ImgDscRSILC
{
public:
	ImgDscRSILCS(istream & s): ImgDscRSILC(s) {mUseSpatialInfo=false;}
	ImgDscRSILCS(const FileNode & fn): ImgDscRSILC(fn) {mUseSpatialInfo=false;}
	ImgDscRSILCS(const Image & src, unsigned dim=0): ImgDscRSILC(src, dim, false){}
	ImgDscRSILCS(const Image * pSrc=0, unsigned dim=DefaultFacePatchDim): ImgDscRSILC(pSrc, dim, false){}
	static const string & Type()
	{
		StaticLkdCtor const string type="ImgDscRSILCS";
		return type;
	}
	virtual const string & getType()const override {return Type();}
};

} // namespace FaceMatch
