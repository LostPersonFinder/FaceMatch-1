ImageMatcher -fm:RSILC -ndx:out FM.CalTech.tiny.RSILC.ndx -m:i -p ../Data/CalTech/Faces/Full/ -lst ../Lists/CalTech/Faces/tiny.GT.FL.lst
ImageMatcher -fm:RSILC -ndx:in FM.CalTech.tiny.RSILC.ndx -p ../Data/CalTech/Faces/Full/ -lst ../Lists/CalTech/Faces/tiny.GT.FL.lst -v
ImageMatcher -fm:RSILC -ndx:in FM.CalTech.tiny.RSILC.ndx -o:s -v=2
ImageMatcher -fm:RSILC -ndx:in FM.CalTech.tiny.RSILC.ndx -o:t -v=2
