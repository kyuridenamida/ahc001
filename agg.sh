set -eu
files=`ls $1/*`
echo -n "Avg: "
cat $files | grep -v "^0$" | st --format="%.0f" --avg
echo -n "  -- var: "
cat $files |grep -v "^0$"  | st --format="%.0f" --stddev
echo

echo -n "Min: "
cat $files |grep -v "^0$"  | st --format="%.0f" --min
res=`cat $files |grep -v "^0$"  | st --format="%.0f" --min`
grep "^$res$" $files
echo

echo -n "50p: "
cat $files |grep -v "^0$"  | st --format="%.0f" --percentile=50


echo -n "Max: "
cat $files |grep -v "^0$"  | st --format="%.0f" --max


echo -n "ZERO "
cat $files |grep "^0$"  | wc -l
echo