
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
#include "SkinHIST.h"

SkinHIST::SkinHIST(int dim, int BinCnt): nDimension(dim), nBins(BinCnt), nItems(0)
{
	RawSkinData = NULL;
	RawNoSkinData = NULL;
	Histogram = NULL;
	FACTOR = 255;

	//allocate memory for the 3d Cube histogram
	const int len=nBins;
	Cube = new double ** [len];
	for(int i=0; i<len; i++)
	{
		Cube[i] = new double*[len];
		for(int j=0; j<len; j++)
		{
			Cube[i][j] = new double[len];
			for (int k=0; k<len; ++k) Cube[i][j][k]=0;
		}
	}
}

SkinHIST::~SkinHIST()
{
	clear();
}

double ** SkinHIST::SetRawSkinData(double ** Data)
{
	return RawSkinData = Data;
}

double ** SkinHIST::SetRawNoSkinData(double ** Data)
{
	RawNoSkinData = Data;
	return (double**)RawNoSkinData;
}

void SkinHIST::normalizeCube()
{
	//normalize the complete Cube
	double Max = 0;
	for(int i=0;i<nBins;i++)
		for(int j=0;j<nBins;j++)
			for(int k=0;k<nBins;k++)
				if (Max< Cube[i][j][k])
					Max = Cube[i][j][k];
	// smooth
	for(int i=1;i<nBins-1;i++)
		for(int j=1;j<nBins-1;j++)
			for(int k=1;k<nBins-1;k++)
				Cube[i][j][k]=(Cube[i][j][k]
					+Cube[i-1][j][k]+Cube[i+1][j][k]
					+Cube[i][j-1][k]+Cube[i][j+1][k]
					+Cube[i][j][k-1]+Cube[i][j][k+1]
				)/7;
	//normalize the complete Cube
	for(int i=0;i<nBins;i++)
		for(int j=0;j<nBins;j++)
			for(int k=0;k<nBins;k++)
				Cube[i][j][k]/= Max;
}

double ** SkinHIST::ReadRawDataForHistogram(const char * FileName, int nDataDimension, int * nReadItems, int nMaxItemsToBeRead)
{
	FILE * pFile = NULL;
	float Value;
	int nCounter = 0;
	int nLineCounter = 0;
	double ** DATA = NULL;
	vector<double> OneItemStorage(nDataDimension);
	double * OneItem = &OneItemStorage.front();

#ifdef _DEBUG
	printf("readig the data file for the histograms ....\n");
#endif
	//open the file for reading purposes
	if ((pFile = fopen(FileName,"r"))==NULL)
	{
		fprintf(stderr, "Error in opening file:%s\n",FileName);
		return NULL;
	}

	// count the number of lines based on the number of columns (see nItems)
	while(!feof(pFile))
	{
		CHECK(fscanf(pFile,"%g",&Value)==1, "expecting a value");
		nCounter++;
		if (nCounter == (nDataDimension))
		{
			nCounter = 0;
			nLineCounter++;
		}
	}
	rewind(pFile);

	//allocate the number of rows (nItems) for the data
	DATA = new double * [nLineCounter];
	if (DATA == NULL)
	{
		printf("Not enough memory to store %d items!\n",nLineCounter);
		return NULL;
	}
	//read the file again, allocate memory for each item and place the elements accordingly
	nCounter = 0; nLineCounter = 0;
	while(!feof(pFile))
	{
		CHECK(fscanf(pFile,"%g",&Value)==1, "expecting a value");
		OneItem[nCounter] = Value;
		nCounter++;
		if (nCounter == (nDataDimension))
		{
			DATA[nLineCounter] = new double[nDataDimension];
			if (DATA[nLineCounter] == NULL)
			{
				printf("Not enough memory to store %d items!\n",nLineCounter);
				return NULL;
			}
			memcpy(DATA[nLineCounter],OneItem,(nDataDimension)*sizeof(double));
			//printf("%f\t",DATA[nLineCounter][2]);
			nCounter = 0;
			nLineCounter++;
		}
	}

	fclose(pFile);
	*nReadItems = nLineCounter;

	nItems = nLineCounter;

	printf("Readig the raw file for the histogram was successful (%d items read)\n",nLineCounter);
	fflush(stdout);

	return DATA;
}

void SkinHIST::FreeData(double ** Data, int nItems)
{
	for(int i=0; i< nItems;i++)
		if (Data[nItems-i+1]) delete []Data[nItems-i+1];
	if (Data) delete []Data;
}

void SkinHIST::BuildMarginalHistogram()
{
	int nIndex;
	//allocate memory for the histogram
	Histogram = new double * [nDimension];
	for(int i=0; i< nDimension;i++)
	{
		Histogram[i]= new double[nBins];
		memset(Histogram[i],0,sizeof(double)*(nBins));
	}
	//go trough each line and create the histograms
	for(int i=0;i<nItems;i++)
		for(int j=0;j<nDimension;j++)
		{
			nIndex=(int)(floor(RawSkinData[i][j]*(nBins-1)));
			Histogram[j][nIndex]++;
		}
		//normalize the complete Histogram to prior probabilities
		for(int i=0;i<nDimension;i++)
		{	double Max=1;
		//find the max for each histogram separately
		for(int j=0;j<nBins;j++)
			if (Max< Histogram[i][j])
				Max=Histogram[i][j];
		//normalize the histograms into [0,1] for each dimension separately
		for(int j=0;j<nBins;j++)
			Histogram[i][j]/=Max;
		}
}

void SkinHIST::BuildCubeHistogram()
{
	if (!RawSkinData) return;

	int i1,i2,i3;

	//go trough each line in the raw data and create the histograms
	for(int i=0;i<nItems;i++)
	{
		i1=(int)(floor(RawSkinData[i][0]*(nBins-1)));
		i2=(int)(floor(RawSkinData[i][1]*(nBins-1)));
		i3=(int)(floor(RawSkinData[i][2]*(nBins-1)));
		Cube[i1][i2][i3]++;
	}
	normalizeCube();
}

void SkinHIST::DeleteHistogram()
{
	if (!Histogram) return;
	//de-allocate memory for the histogram
	for(int i=0; i< nDimension;i++)
		delete [] Histogram[i];
	delete []Histogram;
}

void SkinHIST::DeleteCube()
{
	if (!Cube) return;
	for(int i=0;i<nBins;i++)
	{
		for(int j=0;j<nBins;j++)
			delete [] Cube[i][j];
		delete [] Cube[i];
	}
	delete [] Cube;

}

double SkinHIST::GetSkinLikelihoodMarginalHistogram(double * InputSignal, int nDim)
{
	double Probability = 1;
	int Index;
	for(int i=0; i < nDim;i++)
	{
		Index = int(floor(InputSignal[i]*(nBins-1)));
		Probability *= Histogram[i][Index];
	}
	return pow(Probability,1./3);
}

double SkinHIST::getSkinLikelihoodCubeHistogram(const double * InputSignal, int nDim)const
{
	int
		i1=(int)(floor(InputSignal[0]*(nBins-1))),
		i2=(int)(floor(InputSignal[1]*(nBins-1))),
		i3=(int)(floor(InputSignal[2]*(nBins-1)));
	return Cube[i1][i2][i3];
}

void SkinHIST::PrintRawData(double ** Data)
{
	for(int i = 0;i<nItems;i++)
	{
		for(int j=0; j< nDimension;j++)
			printf("%f\t",Data[i][j]);
		printf("\n"); getchar();
	}
}

void SkinHIST::SaveHistogramToFile(const char * FileName, const char * SourceString)const
{
	FILE * pFile = NULL;

	if((pFile  = fopen( FileName, "wb" )) == NULL)
	{
		printf("Error in opening histogram file:%s", FileName);
		return;
	}
	else
	{
		//start writing the different field in the binary file

		//write the source RGB|HSV|LAB
		fwrite(SourceString, sizeof(char)*HEADER_SIZE, 1, pFile);
		//write the dimension of the histogram
		fwrite(&nDimension, sizeof(int),1,pFile);
		//write the number of bins
		fwrite(&nBins, sizeof(int),1,pFile);
		//write the factor
		fwrite(&FACTOR, sizeof(double),1,pFile);

		//writing the cube
		for(int i=0;i<nBins;i++)
			for(int j=0;j<nBins;j++)
				for(int k=0;k<nBins;k++)
					fwrite(&Cube[i][j][k],sizeof(double),1,pFile);

		//writing the histogram
		/*
		for(int i=0;i<nDimension;i++)
			for(int j=0;j<nBins;j++)
				fwrite(&Histogram[i][j],sizeof(double),1,pFile);
		*/
		//close the file
		fclose(pFile);
	}
}
void SkinHIST::reAllocateCube(unsigned bins)
{
	if (Cube) DeleteCube();
	nBins=bins;
	Cube = new double ** [nBins];
	for(int i=0; i<nBins; i++)
	{
		Cube[i] = new double*[nBins];
		for(int j=0; j<nBins; j++)
		{
			Cube[i][j] = new double[nBins];
			for (int k=0; k<nBins; ++k) Cube[i][j][k]=0;
		}
	}
}
string SkinHIST::LoadHistogramFromFile(const string & FileName)
{
	FILE * pFile = fopen(FileName.c_str(), "rb");
	if(!pFile) throw FaceMatch::Exception("unable to open histogram file "+FileName);

	char Header[HEADER_SIZE];

	//reading the header
	CHECK(fread(Header,sizeof(char),HEADER_SIZE,pFile)==HEADER_SIZE, format("expecting a %d byte buffer", HEADER_SIZE));
	//reading the dimension of the histogram
	CHECK(fread(&nDimension,sizeof(int),1,pFile)==1, "expecting an int value");
	//read the number of bins
	CHECK(fread(&nBins,sizeof(int),1,pFile)==1, "expecting an int value");
	//read the factor
	CHECK(fread(&FACTOR,sizeof(double),1,pFile)==1, "expecting an double value");

	//allocate memory for the 3d Cube histogram
	reAllocateCube();

	//reading the 3d cube histogram
	for(int i=0;i<nBins;i++)
		for(int j=0;j<nBins;j++)
			for(int k=0;k<nBins;k++)
				CHECK(fread(&Cube[i][j][k],sizeof(double),1,pFile)==1, "expecting an double value");
	//close the file
	fclose(pFile);
	//"return" value for the source string
	return Header;
}
