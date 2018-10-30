echo t:fr	FileName	Given	Found	Match	FNCnt	FPCnt	Recall	Precision	FScore >out.txt 2>log.txt
for %%t in (0.10 0.20 0.25 0.30 0.40 0.50 0.60 0.75) do (
	echo|set /p="%%t	" >>out.txt
	FaceFinder -eval -fd:sub:LM -lst:in ..\Lists\AFLW\5770.GT.FL.lst -lst:out ..\Lists\AFLW\5770.Chehra.FF.lst -t:fr %%t >>out.txt 2>>log.txt
)
