#pragma once // 2012 (C) Eugene.Borovikov@NIH.gov

#include <map>

using namespace std;
using namespace FaceMatch;

/**
 * a representation for image meta-data
 */
class ImageMetaData
{
	typedef map<string, string> MapMetaInfo;
	MapMetaInfo mInfo;
public:
	/**
	 * Construct an instance by loading a comma separated values (CSV) for the image meta info.
	 * @param inFN	input comma-separated-values (CSV) file name
	 */
	ImageMetaData(const string & inFN)
	{
		if (inFN.empty()) return; // nothing to load from
		ListReader lr(inFN);
		string ln; lr.fetch(ln); // header
		while (!lr.end())
		{
			lr.fetch(ln); // next line
			if (ln.empty()) continue;
			stringstream s(ln);
			string ID; getline(s, ID, ','); trim(ID, "\"");
			string info; getline(s, info, ','); trim(info, "\"");
			mInfo[ID]=info;
		}
	}
	/**
	 * Look up meta info for the image file and output it to clog.
	 * @param ImgFNLine	image file name and possibly its parameters, e.g. face/profile regions, etc; in the format<br/>
	 *     FileName.ext[\\tRegion1\\tRegion2...]<br/>
	 * where each Region could be either simple (just the face/profile rectangle) or complex (including eyes, nose, and mouth sub-regions).
	 * The simple region format is: k[x,y;w,h], where k={f|p} is the region kind (face or profile);
	 * the brackets specify the enclosing rectangle:
	 * x,y being the top-left coordinates and w,h being its width,height dimensions, all in pixels.
	 * A complex region has the format: k{[x,y;w,h]\\tSubRegions}, where SubRegions are simple regions of three kinds:
	 * i (for eye), m (for mouth) and n (for nose); their dimensions are specified as for a simple region, with pixel coordinates relative to the parent.
	 */
	void FindAndLog(const string & ImgFNLine)
	{
		clog<<ImgFNLine;
		stringstream s(ImgFNLine); string ID; getline(s, ID, '_');
		MapMetaInfo::iterator it = mInfo.find(ID);
		if (it!=mInfo.end()) clog<<'\t'<<it->second;
		clog<<endl;
	}
};
