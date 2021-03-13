set -eu
prefix=${1}
python3 submit_code.py > /tmp/src.cpp
rm -rf /tmp/ahc001.out
g++ /tmp/src.cpp -O3 -fsanitize=address -o /tmp/ahc001.out
#g++ ./src/main.cpp -O3 -o /tmp/ahc001.out
target=0096.txt
p=${2:-16}
curtime=`date "+%Y-%m-%d_%H-%M-%S"`
mkdir -p output/$prefix-$curtime
./combination.py 0.0003,0.0005,0.0008 0.000001,0.000002,0.000003,0.000004,0.000005,0.000010,0.000020,0.000030,0.000100,0.000200,0.000300 0,1,2,3,4,5,6 | xargs -t -P$p -I{} sh -c "./run-single.sh '/tmp/ahc001.out {}' ${target} > 'output/$prefix-$curtime/{}.score'"
grep -H "" output/$prefix-$curtime/*
