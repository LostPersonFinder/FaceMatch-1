FaceFinder -p ..\Data\HEPL\images\original\ -lst:in ..\Lists\HEPL\small.lst >profile.FF.out 2>profile.FF.log
FaceFinder -p ..\Data\HEPL\images\original\ -lst:in ..\Lists\HEPL\small.lst -r >profile.FF-r.out 2>profile.FF-r.log
FaceFinder -p ..\Data\HEPL\images\original\ -lst:in ..\Lists\HEPL\small.lst -s:in NET_PL_9_15_50_90.18.ann >profile.FF.ANN.out 2>profile.FF.ANN.log
FaceFinder -p ..\Data\HEPL\images\original\ -lst:in ..\Lists\HEPL\small.lst -r -s:in NET_PL_9_15_50_90.18.ann >profile.FF-r.ANN.out 2>profile.FF-r.ANN.log
FaceFinder -p ..\Data\HEPL\images\original\ -lst:in ..\Lists\HEPL\small.lst -fd:sub! -r -s:in NET_PL_9_15_50_90.18.ann >profile.FF-r.sub.ANN.out 2>profile.FF-r.sub.ANN.log