# Delta 查询负载

# <dbid_tableid_cuid_row_id_event_seq, row_val>
# - 全表范围uniform扫描
# - 平均扫描长度达到1000
# - 100% scan操作

recordcount=1000000
operationcount=300000

fieldcount=1
fieldlength=40960

workload=com.yahoo.ycsb.workloads.CoreWorkload
readallfields=true

# - 100% scan操作
insertproportion=0
readproportion=0
updateproportion=0
scanproportion=1.0
readmodifywriteproportion=0

# - 全表范围uniform扫描
requestdistribution=uniform

# - 平均扫描长度达到1000
maxscanlength=2000
scanlengthdistribution=uniform

insertorder=ordered
