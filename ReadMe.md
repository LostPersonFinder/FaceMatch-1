ABOUT
-----
FaceMatch (comprising FaceMatchWebServices, FaceMatchMonitor, FaceMatcherCore, FaceMatch.lib) are products of the Lost Person Finder project (http://lpf.nlm.nih.gov) 
at the Lister Hill National Center for Biomedical Communications, which is an intramural R&D division of the U.S. National Library of Medicine, 
part of the U.S. National Institutes of Health. 

LICENCE
-------
This software was developed under contract funded by the National Library of Medicine, which is part of the National Institutes of Health, an agency of the Department of Health and Human Services, United States Government.

The license of this software is an open-source BSD license.  It allows use in both commercial and non-commercial products.

The license does not supersede any applicable United States law.

The license does not indemnify you from any claims brought by third parties whose proprietary rights may be infringed by your usage of this software.

Government usage rights for this software are established by Federal law, which includes, but may not be limited to, Federal Acquisition Regulation (FAR) 48 C.F.R. Part52.227-14, Rights in Data?General.
The license for this software is intended to be expansive, rather than restrictive, in encouraging the use of this software in both commercial and non-commercial products.

LICENSE:

Government Usage Rights Notice:  The U.S. Government retains unlimited, royalty-free usage rights to this software, but not ownership, as provided by Federal law.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

?	Redistributions of source code must retain the above Government Usage Rights Notice, this list of conditions and the following disclaimer.

?	Redistributions in binary form must reproduce the above Government Usage Rights Notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

?	The names,trademarks, and service marks of the National Library of Medicine, the National Cancer Institute, the National Institutes of Health, and the names of any of the software developers shall not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE U.S. GOVERNMENT AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITEDTO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE U.S. GOVERNMENT
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


FaceMatchWebServices
--------------------
Provides fast, low overhead, web services for Facematch.
Exposed Capabilities:
Face localizer, single and multiple regions.
Face ingest, single and multiple regions.
Face query, single and multiple regions.
Whole Image ingest and query."


FaceMatcherCore Server
----------------------
Provides a fast light weight bridge between the web service  and the core library with GPU acceleration. 
It transparently maps facematch core library capabilities to the caller. The FaceMatch service calls this 
module for its visual matching services. 

Build and Install
-----------------
By default this program is installed in the location C:\FacematchSL\FaceMatch\bin. To build this program
you will need Visual Studio 2015. 

Prerequisites
-------------
Visual Studio 2015 
CUDA 5.5 or later SDK from NVIDIA
Tesla k20 or better GPU from NVIDIA 
.NET4 CLR 
OpenCV 2.4.13 or newer

Installation Procedure
----------------------
On a Windows 2008R2 or Windows 2012 server do the following
Install a Tesla k20 card with the following packages 
Install OpenCV 2.4.13 or higher at C:\OpenCV, set environment variable OpenCVHome to point to this
Install CUDA 5.5 or higher SDK, set environment variable CUDA_PATH to point to this
Install Visual Studio 2015
Build the FaceMatch project from source it is located here 
Build the FaceMatcherCore project from source it is located here . 
Copy the server to the above FaceMatch\bin folder. Open a command window with admin rights and register the executable with the following command “FaceMatcherCore.exe /regserver”
Build the FaceMatchWebService project from source it is located here. Install the windows service.
Start the FaceMatchWebService from the services control panel. This starts the web services for use.

Further Documentation
---------------------
Please refer to the file FM_System_Overview.pptm for the process architecture 