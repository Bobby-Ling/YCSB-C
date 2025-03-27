sudo rm -rf /tmp/rocksdb-test/*

date

./ycsbc -db rocksdb -dbpath /tmp/rocksdb-test -threads 4 -P ./workloads/workloada.spec -load true -dboption 1 -dbstatistics true| tee load`date +%Y%m%d%H%M%S`.log

date

./ycsbc -db rocksdb -dbpath /tmp/rocksdb-test -threads 4 -P ./workloads/workloada.spec -run true -dboption 1 -dbstatistics true| tee run`date +%Y%m%d%H%M%S`.log

date