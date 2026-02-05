# CUDesc 入库和查询负载

# <dbid_tableid_cuid, cu-meta>
# - 在入库负载和查询负载下都是对全表范围的uniform顺序扫描
# - CUDesc的访问特征表现为全范围的scan访问
# - 在所有操作中scan操作的比例超过了90%
# - 平均长度达到3000

recordcount=1000000
operationcount=250000

fieldcount=1
fieldlength=256
workload=com.yahoo.ycsb.workloads.CoreWorkload
readallfields=true
writeallfields=true

# - 在所有操作中scan操作的比例超过了90%
insertproportion=0.05
readproportion=0.05
updateproportion=0
scanproportion=0.90
readmodifywriteproportion=0

requestdistribution=uniform

insertorder=ordered

# - 平均长度达到3000
maxscanlength=6000
scanlengthdistribution=uniform

