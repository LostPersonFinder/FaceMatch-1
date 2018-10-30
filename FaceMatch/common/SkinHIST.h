
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

#ifndef SkinHIST_h_2012_dec_7
#define SkinHIST_h_2012_dec_7

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>

//**********************************************************
// standard libraries inludes
//*********************************************************

//**********************************************************
// personal libraries inludes
//**********************************************************
#define HEADER_SIZE 128
#define DEFBINCNT 16

/** skin color histogram */
class LIBDCL SkinHIST
{
	double ** RawSkinData;		
	double ** RawNoSkinData;	
	double ** Histogram;		

	int nItems;					

protected:
	/** number of bins */
	int nBins;
	/** number of dimensions */
	int nDimension;
	/** histogram storage */
	double *** Cube;
	/** histogram factor */
	double FACTOR ;

	/**
	 * Free the memory for the marginal and the cube histogram. This function is called by the destructor.
	 */
	void clear()
	{
		if (Cube) DeleteCube();
		if (Histogram) DeleteHistogram();
	}
	/**
	 * Re-allocate the cube storage.
	 */
	void reAllocateCube(unsigned bins=DEFBINCNT);

	/**
	 * Normalize the 3 dimensional cube to [0,1], followed by a smotthing (averaging the values among the 8 neighbours.
	 */
	void normalizeCube();

	/**
	 * Assign the raw data to the RawSkinData.
	 * @param Data contains the data (raw) in a two dimensional vector, each line representing one raw data point
	 * @return the pointer to the raw skin data
	 */
	double ** SetRawSkinData(double ** Data);

	/**
	 * Assign the raw non-skin data to the RawNoSkinData.
	 * @param Data contains the non-skin data (raw) in a two dimensional vector, each line representing one raw data point
	 * @return the pointer to the raw non-skin data
	 */
	double ** SetRawNoSkinData(double ** Data);

	/**
	 * Get the number of bins.
	 * @return the the bins used for the histogram.
	 */
	int getBinCount()const{return nBins;}

	/**
	 * Set the number of bins for the histogram. Usually 16, 32, 64, 128.
	 * @param nNumber is the number of bins to be set.
	 * @return the number of bins set in the class.
	 */
	int SetBins(int nNumber);

	/**
	 * Get the dimension of the raw data. In our case it is 3.
	 * @return the dimension of the data used for the histogram building.
	 */
	int getDimensionCount()const{return nDimension;}

	/**
	 * Set the dimension of the raw data for the histogram. Usually 3.
	 * @param nNumber is the dimension of the data to be set.
	 * @return the number of dimension of the raw data.
	 */
	int SetDimension(int nNumber);

	/**
	 * Reading a tsv (tab separated values) file containing the raw data for the histogram. Only data is involved, no label.
	 * @param FileName is the name of the text file containing the raw data (in tsv format).
	 * @param nDataDimension is the dimension of the data column wise. In our case is 3 dimensional.
	 * @param nReadItems is returned to be able to know how many entries have been found (read) from the text file.
	 * @param nMaxItemsToBeRead is meant to limit the number of items to be read. 
	 * @return the pointer to the bidemensional raw data.
	 */
	double ** ReadRawDataForHistogram(const char * FileName, int nDataDimension, int * nReadItems, int nMaxItemsToBeRead = 0);

	/**
	 * Free the memory occupied by the data knowing exactly how many items have been allocated.
	 * @param Data is the pointer to the data
	 * @param nItems in the number of items used to build the histogram
	 */
	void FreeData(double ** Data, int nItems);

	/**
	 * Build the marginal histogram. Each dimension of the data will be considered separately and a particular histogram will be built containing nBins.
	 */
	void BuildMarginalHistogram();  

	/**
	 * Build the Behaviour Knowledge Space (cube histogram). The elements in the cube are index by the different dimensions of the raw data.
	 */
	void BuildCubeHistogram();

	/**
	 * Delete the marginal histogram object.
	 */
	void DeleteHistogram();  

	/**
	 * Delete the cube histogram object.
	 */
	void DeleteCube();  

	/**
	 * Print the raw data. Each data point dimension separated by tab. And each data point separated by 
	 * @param Data is the data point to be considered.
	 */
	void PrintRawData(double ** Data);

	

public:

	//-------------------------------------------------------
	// CONSTRUCTOR DESTRUCTOR
	//-------------------------------------------------------
	/**
	 * Constructor for the histogram object.
	 * @param dim give the dimension of the histogram
	 * @param BinCnt gives the number of bins used to build the histogram
	 */
	SkinHIST(int dim, int BinCnt);

	/**
	 * Destructor for the histogram object.
	 */
	virtual ~SkinHIST();

	/**
	 * Calculate the likelihood of a data point considering the marginal histogram.
	 * @param InputSignal is the data point to be considered.
	 * @param nDim is the dimension of the InputSignal (in our case it is rather just  3).
	 * @return the likelihood for the data point. 0 [non skin], 1[skin].
	 */
	double GetSkinLikelihoodMarginalHistogram(double * InputSignal, int nDim);

	/**
	 * Calculate the likelihood of a data point considering the cube histogram.
	 * @param InputSignal is the data point to be considered.
	 * @param nDim is the dimension of the InputSignal (in our case it is rather just  3).
	 * @return the likelihood for the data point. 0 [non skin], 1[skin].
	 */
	double getSkinLikelihoodCubeHistogram(const double * InputSignal, int nDim)const;

	/**
	 * Save the histogram to a binary file.
	 * @param FileName is the name of the file where the data should be saved.
	 * @param SourceString is a constant size string which contains the source of the data (ex. RGB, HSB, LAB, etc).
	 */
	void SaveHistogramToFile(const char * FileName, const char * SourceString)const;

	/**
	 * Load the histogram from a binary file.
	 * @param FileName is the name of the file where the data should be saved.
	 */
	string LoadHistogramFromFile(const string & FileName);

};

#endif
