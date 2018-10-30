python FF.peval.py -fd:sub:LMC -lst:in ..\Lists\Lehigh\Celebrities\512.GT.lst -p ..\Data\Lehigh\Celebrities\images\ -setOutDN FF.peval.hist.sub.LMC.Celebrities -skipNUL
python FF.peval.py -lst:in ..\Lists\Lehigh\Celebrities\512.GT.lst -setOutDN FF.peval.hist.sub.LMC.Celebrities -skipRun
copy FF.peval.hist.sub.LMC.Celebrities\FF-eval.txt FF.peval.hist.sub.LMC.Celebrities\FF.eval.sw-0.txt
python FF.peval.py -lst:in ..\Lists\Lehigh\Celebrities\512.GT.lst -setOutDN FF.peval.hist.sub.LMC.Celebrities -skipRun -t:sw 1
copy FF.peval.hist.sub.LMC.Celebrities\FF-eval.txt FF.peval.hist.sub.LMC.Celebrities\FF.eval.sw-1.txt
