sudo rm -rf /localdata/ycsb/*

sudo rm -rf /nvmedata/ycsb/*

iostat -dx /dev/nvme1n1p1 1 > disk_bw_load_update.log &
IOSTAT_PID=$!

./ycsbc -db rocksdb -dbpath /localdata/ycsb -threads 100 -P ./workloads/workloada.spec -load true -run true -dboption 1 -dbstatistics true > output.log 2>&1

kill $IOSTAT_PID

# iostat -dx /dev/nvme1n1p1 1 > disk_bw_read.log &
# IOSTAT_PID=$!

# ./ycsbc -db rocksdb -dbpath /localdata/ycsb -threads 16 -P ./workloads/workloadb.spec -run true -dboption 1 -dbstatistics true > read_output.log 2>&1

# kill $IOSTAT_PID

# mv /nvmedata/rocksdb-lat.hgrm ./read_lat.hgrm

# mv /nvmedata/rocksdb-lat.hiccup ./read_lat.hiccup