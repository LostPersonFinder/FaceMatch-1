set GTL=..\Lists\HEPL\500.reasonable\512.GT.lst
set DATA=\\ceb-facematch\Data\HEPL\images\original\

for %%m in (0.2 0.25 0.3 0.4) do (
	for %%l in (0.25 0.5 0.75) do (
		rem python FF.peval.py -lst:in %GTL% -p %DATA% -t:sm %%m -t:sl %%l -fd:sub:LMC -setOutDN FF.peval.LMC.HEPL.tsm-%%m.tsl-%%l
		python FF.peval.py -lst:in %GTL% -setOutDN FF.peval.LMC.HEPL.tsm-%%m.tsl-%%l -skipRun 
	)
)