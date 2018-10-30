:: Batch script to run ImageMatcher and prepare for evaluation with CSU scripts
:: This batch will index a list and then run that index and list against every 
:: item in the list for each of the FaceMatch algorithms: HAAR, LBPH, ORB, SIFT, 
:: SURF, DIST, MANY & RANK. Results will be stored under the folder distances in
:: in the bin. For each batch you run you should use a different test name, 
:: results are futher differentiated by being saved under the folder with the
:: test name. Each algorithm will have its results pushed to a file 
:: (algorithm name).results.txt 
:: When evaluation finishes the java program AdjustDistancesForCSU.jar is called
:: This program requires to inputs, the folder of your *.results.txt files and
:: that of your csuHome directory. With these inputs it will read the *.results.txt
:: break up the quarries into individual files and save them in CSU's format at 
:: csuHome/distances/feret/algorithmName/quarryimage.sfi  
:: It will only save distances for images in the all.srt file under imagelists sub-
:: folder of the csuHome directory. 

:: Path to image directory/repository
set p=C:/cygwin/home/bonifantmc/Images/CalTechJPEGS-Original/

:: Selection of images to test from image directory/repository
set list=../Lists/CalTech/all.chase.lst

:: The name you're giving this test
set test=CalTechReg

:: The location of the Colorado State Face Evaluation Program's all.srt in use
set allSort=../Tools/csuFaceIdEval-5.1/imagelists/all.srt
:: The location you want to output your CSU distance files to
set dist="//LHCDEVFILER/facematch-project/Data/FERET/distances/feret"

:: The image file deliminator to mark where subj id ends in prefix
set delim='_'

:: Tolerance for obtaining results, for CSU this is however many images you are testing
set tol=450

:: Distance folder for output in the bin
if not exist ./distances mkdir distances
set distances=./distances

:: Output directories for distances in facematch formatting
if not exist ./distances/%TEST% mkdir ./distances/%TEST%

::Index the input images
ImageMatcher -fm -m:i -ndx:in many.dsc.txt -ndx:out %test%.txt -p %p% -lst %list% > %test%.ndx.out 2> %test%.ndx.err

:: Run query on images with supplied created ingests
for /f "tokens=1" %%a in (%test%.txt) do (
	ImageMatcher -m:q -fm:%%a -ld %delim% -p %p% -lst %list% -ndx:in %test%.%%a.ndx -t %tol% > %distances%/%test%/%%a.results.txt
)
::Run Query on different ImageMatcher Modes

ImageMatcher -m:q -fm:MANY -ld %delim% -p %p% -lst %list% -ndx:in %test%.txt -t %tol% > %distances%/%test%/MANY.results.txt
ImageMatcher -m:q -fm:RANK -ld %delim% -p %p% -lst %list% -ndx:in %test%.txt -t %tol% > %distances%/%test%/RANK.results.txt
ImageMatcher -m:q -fm:DIST -ld %delim% -p %p% -lst %list% -ndx:in %test%.txt -t %tol% > %distances%/%test%/DIST.results.txt

:: Adjust output of imagematcher to match CSU standards
java -jar AdjustDistancesForCSU.jar %distances%/%test% %allSort% %dist%
pause
::End