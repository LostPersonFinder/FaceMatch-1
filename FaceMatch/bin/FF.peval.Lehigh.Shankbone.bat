python FF.peval.py -fd:sub:LMC -lst:in ..\Lists\Lehigh\Shankbone\all.GT.lst -p ..\Data\Lehigh\Shankbone\images\original\ -setOutDN FF.peval.hist.sub.LMC.Shankbone -skipNUL
python FF.peval.py -lst:in ..\Lists\Lehigh\Shankbone\all.GT.lst -setOutDN FF.peval.hist.sub.LMC.Shankbone -skipRun 
copy FF.peval.hist.sub.LMC.Shankbone\FF-eval.txt FF.peval.hist.sub.LMC.Shankbone\FF.eval.sw-0.txt
python FF.peval.py -lst:in ..\Lists\Lehigh\Shankbone\all.GT.lst -setOutDN FF.peval.hist.sub.LMC.Shankbone -skipRun -t:sw 1
copy FF.peval.hist.sub.LMC.Shankbone\FF-eval.txt FF.peval.hist.sub.LMC.Shankbone\FF.eval.sw-1.txt
