for f in `cat ../Lists/LFWA/tiny.lst`
do
	FaceMatcher $1 $2 $3 $4 ../Data/LFW.aligned/images/Yashwant_Sinha/Yashwant_Sinha_0001.jpg ../Data/LFW.aligned/images/$f
done
