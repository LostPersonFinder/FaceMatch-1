
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
#include "cv_supp.h"
using namespace std::placeholders;
namespace cv
{
	inline unsigned minDim(const Mat & src){ return min(src.rows, src.cols); }
	bool OK2GPU(const Mat & src, bool rethrow=false)
	{
		static const unsigned cImgDiam4GPU = 256; // TODO: param/config
		return preferGPU() && getGPUCount(rethrow) && minDim(src)>cImgDiam4GPU;
	}
	Mat GPU(const Mat & src, std::function<void(const gpu::GpuMat&, gpu::GpuMat&)> f)
	{
		GPULocker lkr;
		gpu::GpuMat gsrc(src), gdst;
		f(gsrc, gdst);
		return Mat(gdst);
	}
	Mat getRotMx(const Size & dim, REALNUM angle)
	{
		Point2f center(dim.width / 2.0, dim.height / 2.0);
		return getRotationMatrix2D(center, angle, 1.0);
	}
	Mat convert(const Mat & src, int code)
	{
		Mat dst;
		if (OK2GPU(src))
			dst=GPU(src, [code](const gpu::GpuMat & gsrc, gpu::GpuMat& gdst){ gpu::cvtColor(gsrc, gdst, code); });
		else cvtColor(src, dst, code);
		return dst;
	}
	Mat eqHist(const Mat & src)
	{
		Mat dst;
		if (OK2GPU(src))
			dst = GPU(src, [](const gpu::GpuMat & gsrc, gpu::GpuMat& gdst) { gpu::equalizeHist(gsrc, gdst); });
		else equalizeHist(src, dst);
		return dst;
	}
	Mat normalizeGPUGS(const Mat & src, double alpha, double beta, int norm_type, int dtype, const Mat & mask)
	{
		return GPU(src, [alpha, beta, norm_type, dtype, mask]
			(const gpu::GpuMat & gsrc, gpu::GpuMat& gdst){ gpu::normalize(gsrc, gdst, alpha, beta, norm_type, dtype, gpu::GpuMat(mask)); });
	}
	Mat normalize(const Mat & src, double alpha, double beta, int norm_type, int dtype, const Mat & mask)
	{
		Mat dst;
		if (OK2GPU(src))
		{
			dst=GPU(src, [alpha, beta, norm_type, dtype, mask] (const gpu::GpuMat & gsrc, gpu::GpuMat& gdst)
			{
				gpu::GpuMat gmsk = mask.empty() ? gpu::GpuMat() : gpu::GpuMat(mask);
				int CC = gsrc.channels();
				if (CC == 1) gpu::normalize(gsrc, gdst, alpha, beta, norm_type, dtype, gmsk);
				else
				{
					vector<gpu::GpuMat> gpuDstChs(CC), gpuSrcChs; gpu::split(gsrc, gpuSrcChs);
					FaceMatch::ParallelErrorStream PES;
				#pragma omp parallel for shared (PES, gmsk, gpuDstChs, gpuSrcChs)
					for (int i = 0; i < CC; ++i)
						try
						{
							gpu::normalize(gpuSrcChs[i], gpuDstChs[i], alpha, beta, norm_type, dtype, gmsk);
						}
						PCATCH(PES, format("normalize[%d] error", i))
					PES.report("parallel normalize errors");
					gpu::merge(gpuDstChs, gdst);
				} 
			});
		}
		else normalize(src, dst, alpha, beta, norm_type, dtype, mask);
		return dst;
	}
	Mat GrayScale(const Mat & src)
	{
		Mat ImgGS;
		if (src.channels() == 1) ImgGS = src;
		else ImgGS = convert(src, CV_BGR2GRAY);
		return ImgGS;
	}
	Mat rotate(const Mat & src, REALNUM angle, int flags)
	{
		if (angle==0) return src;
		Mat R = getRotMx(src.size(), angle);
		if (flags<0) flags=INTER_LINEAR;
		Mat dst;
		if (OK2GPU(src) && flags <= INTER_CUBIC)
			dst=GPU(src, [R, flags](const gpu::GpuMat & gsrc, gpu::GpuMat& gdst){ gpu::warpAffine(gsrc, gdst, R, gsrc.size(), flags); });
		else warpAffine(src, dst, R, src.size(), flags);
		return dst;
	}
	Mat scale(const Mat & src, REALNUM s)
	{
		if (!s) return Mat();
		if (s==1) return src;
		Mat dst;
		if (OK2GPU(src))
			dst=GPU(src, [s](const gpu::GpuMat & gsrc, gpu::GpuMat& gdst){ gpu::resize(gsrc, gdst, Size(), s, s); });
		else cv::resize(src, dst, Size(), s, s);
		return dst;
	}
	Mat gblur(const Mat & src, Size sz, REALNUM sgm)
	{
		Mat dst;
		if (OK2GPU(src))
			dst=GPU(src, [sz, sgm](const gpu::GpuMat & gsrc, gpu::GpuMat& gdst){ gpu::GaussianBlur(gsrc, gdst, sz, sgm); });
		else GaussianBlur(src, dst, sz, sgm);
		return dst;
	}
	MatReal match(const Mat & src, const Mat & tmp, int method)
	{
		MatReal dst;
		if (OK2GPU(src))
			dst=GPU(src, [tmp, method](const gpu::GpuMat & gsrc, gpu::GpuMat& gdst)
				{ const gpu::GpuMat gtmp(tmp); gpu::matchTemplate(gsrc, gtmp, gdst, method); });
		else cv::matchTemplate(src, tmp, dst, method);
		return dst;
	}
	void write(ostream & s, const Mat & m)
	{
		writeSimple(s, m.rows);
		writeSimple(s, m.cols);
		writeSimple(s, m.type());
		size_t matDataSize = m.total()*m.elemSize();
		writeSimple(s, matDataSize);
		if (matDataSize > 0) writeBulk(s, m.data, matDataSize);
	}
	void read(istream & s, Mat & m)
	{
		int rows = 0, cols = 0, type = 0;
		size_t matDataSize = 0;
		readSimple(s, rows);
		readSimple(s, cols);
		readSimple(s, type);
		readSimple(s, matDataSize);
		if (matDataSize > 0)
		{
			m = Mat(rows, cols, type);
			readBulk(s, m.data, matDataSize);
		}
	}
	void write(ostream & s, const KeyPoints & kpts)
	{
		auto
			ki = kpts.begin(),
			ke = kpts.end();
		unsigned len = kpts.size();

		writeSimple(s, len);
		for (; ki != ke; ki++)
		{
			writeSimple(s, ki->pt.x);
			writeSimple(s, ki->pt.y);
			writeSimple(s, ki->size);
			writeSimple(s, ki->angle);
			writeSimple(s, ki->response);
			writeSimple(s, ki->octave);
			writeSimple(s, ki->class_id);
		}
	}
	void read(istream & s, KeyPoints & kpts)
	{
		unsigned len = 0;
		readSimple(s, len);
		for (int i = 0; i<len; i++)
		{
			KeyPoint kp;
			readSimple(s, kp.pt.x);
			readSimple(s, kp.pt.y);
			readSimple(s, kp.size);
			readSimple(s, kp.angle);
			readSimple(s, kp.response);
			readSimple(s, kp.octave);
			readSimple(s, kp.class_id);
			kpts.push_back(kp);
		}
	}
	Mat mult(const Mat & a, const Mat & b, REALNUM s=1)
	{
		Mat res;
		multiply(a, b, res, s, a.type()); // TODO: consider GPU
		return res;
	}
	Mat & operator*=(Mat & src, const MatReal & map)
	{
		auto CC = src.channels();
		if (CC == 1) return src=mult(src, map);
		Mat maps[] = { map, map, map };
		Mat CM3; merge(maps, 3, CM3);
		return src=mult(src, CM3);
	}
} // namespace cv
