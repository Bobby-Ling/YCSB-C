# Index 入库负载配置

# <dbid_tableid_pk, cuid + row-id>
# - Index表只在入库负载下会被访问
# - 全范围内的随机读取与随机更新
# - 操作比例为get_for_update:put:put_if_unique:delete = 30:30:15:18

recordcount=1000000
operationcount=250000

fieldcount=1
fieldlength=64

workload=com.yahoo.ycsb.workloads.CoreWorkload
readallfields=true
writeallfields=true

# - 操作比例为get_for_update:put:put_if_unique:delete = 30:30:15:18
readproportion=0.323        # 30/93 ≈ 0.323
updateproportion=0.323      # 30/93 ≈ 0.323
insertproportion=0.354      # 33/93 ≈ 0.354
scanproportion=0
readmodifywriteproportion=0

# - 全范围内的随机读取与随机更新
requestdistribution=uniform
insertorder=hashed

maxscanlength=1
scanlengthdistribution=uniform
