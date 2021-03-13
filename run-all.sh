set -eu
python3 submit_code.py > /tmp/src.cpp
rm -rf /tmp/ahc001.out
g++ /tmp/src.cpp -O3 -fsanitize=address -o /tmp/ahc001.out
#g++ ./src/main.cpp -O3 -std=c++14-o /tmp/ahc001.out
prefix=${1}
p=${2:-16}
curtime=`date "+%Y-%m-%d_%H-%M-%S"`
mkdir -p output/$prefix-$curtime
ls in | xargs -t -P$p -I{} sh -c "./run-single.sh /tmp/ahc001.out {} > output/$prefix-$curtime/{}.score"
./agg.sh output/$prefix-$curtime
