sudo rm -rf /tmp/rocksdb-test/*

date

./ycsbc -dbstatistics true -db rocksdb -dbpath /tmp/rocksdb-test -threads 4 -P ./workloads/workloada.spec -load true | tee load`date +%Y%m%d%H%M%S`.log

date

./ycsbc -db rocksdb -dbpath /tmp/rocksdb-test -threads 4 -P ./workloads/workloada.spec -dbstatistics true -run true| tee run`date +%Y%m%d%H%M%S`.log

date