#/bin/bash

pushd .

ROOT_DIR=$(dirname "$(realpath "$0")")

cd build/debug-asan

# workload="./workloads/workloada.spec"
workload="$ROOT_DIR/workloads/test.spec"
dbpath="./rocksdb_test"

mkdir -p $dbpath


./ycsbc -db rocksdb -dbpath $dbpath -threads 1 -P $workload -load true -dboption 2

echo "run"
./ycsbc -db rocksdb -dbpath $dbpath -threads 1 -P $workload -run true -dboption 2
echo "run"

rm -fr $dbpath

popd