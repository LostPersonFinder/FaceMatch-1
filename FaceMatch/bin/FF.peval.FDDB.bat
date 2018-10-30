python FF.peval.py -fd:sub:LMC -lst:in ..\Lists\FDDB\FDDB.GT.all.lst -p \\ceb-facematch\Data\FDDB\ -setOutDN FF.peval.hist.sub.LMC.FDDB -skipNUL
python FF.peval.py -lst:in ..\Lists\FDDB\FDDB.GT.all.lst -setOutDN FF.peval.hist.sub.LMC.FDDB -skipRun 
copy FF.peval.hist.sub.LMC.FDDB\FF-eval.txt FF.peval.hist.sub.LMC.FDDB\FF.eval.sw-0.txt
python FF.peval.py -lst:in ..\Lists\FDDB\FDDB.GT.all.lst -setOutDN FF.peval.hist.sub.LMC.FDDB -skipRun -t:sw 1
copy FF.peval.hist.sub.LMC.FDDB\FF-eval.txt FF.peval.hist.sub.LMC.FDDB\FF.eval.sw-1.txt
