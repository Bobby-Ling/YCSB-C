# 在入库负载下，对三张表的访问比例为Index:Delta:CUDesc=1:2:1。
# 在查询负载下，对三张表的访问比例为Index:Delta:CUDesc=0:3:1。

# DWS Delta表 入库负载

# <dbid_tableid_cuid_row_id_event_seq, row_val>
# - Delta表的访问在所有负载下都表现出明显顺序特性，操作类型集中在scan、put和delete。
# - 入库负载对delta表现出明显的key局部性

recordcount=1000000
operationcount=500000

# row_val
fieldcount=1
fieldlength=40960

workload=com.yahoo.ycsb.workloads.CoreWorkload
readallfields=true
writeallfields=true

# put-scan-delete
insertproportion=0.33
readproportion=0
updateproportion=0.33
scanproportion=0.34
readmodifywriteproportion=0

# 入库负载对delta表现出明显的key局部性
requestdistribution=zipfian

zipfianconstant=0.99

# - 平均扫描长度达到1000
maxscanlength=2000
scanlengthdistribution=uniform

insertorder=ordered