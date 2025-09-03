sudo fdisk /dev/nvme1n1

sudo mkfs.ext4 /dev/nvme1n1p1

sudo mount /dev/nvme1n1p1 /localdata/

sudo chmod -R 777 /localdata/

sudo fdisk /dev/nvme2n1

sudo mkfs.ext4 /dev/nvme2n1p1

sudo mount /dev/nvme2n1p1 /nvmedata/

sudo chmod -R 777 /nvmedata/