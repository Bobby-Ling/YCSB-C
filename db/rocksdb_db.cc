//
// Created by wujy on 1/23/19.
//
#include <iostream>

#include "rocksdb/persistent_cache.h"
#include "rocksdb/table.h"
#include "rocksdb/env.h"
#include "rocksdb/status.h"
#include "rocksdb_db.h"
#include "lib/coding.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include "rocksdb/cloud/db_cloud.h"
#include "rocksdb/options.h"
#include "rocksdb/filter_policy.h"

using namespace std;

namespace ycsbc {

    RocksDB::RocksDB(const char *dbfilename, utils::Properties &props) :noResult(0){
        int r = hdr_init(1,INT64_C(3600000000),3,&hdr_);
        r |= hdr_init(1, INT64_C(3600000000), 3, &hdr_get_);
        r |= hdr_init(1, INT64_C(3600000000), 3, &hdr_put_);
        r |= hdr_init(1, INT64_C(3600000000), 3, &hdr_update_);
        r |= hdr_init(1, INT64_C(3600000000), 3, &hdr_scan_);
	    r |= hdr_init(1, INT64_C(3600000000), 3, &hdr_rmw_);
        if((0 != r) || (NULL == hdr_) 
		    || (NULL == hdr_get_) || (NULL == hdr_put_)
		    || (NULL == hdr_scan_) || (NULL == hdr_rmw_) 
		    || (NULL == hdr_update_) || (23552 < hdr_->counts_len)) {
                cout << "DEBUG- init hdrhistogram failed." << endl;
                cout << "DEBUG- r=" << r << endl;
                cout << "DEBUG- histogram=" << &hdr_ << endl;
                cout << "DEBUG- counts_len=" << hdr_->counts_len << endl;
                cout << "DEBUG- counts:" << hdr_->counts << ", total_c:" << hdr_->total_count << endl;
                cout << "DEBUG- lowest:" << hdr_->lowest_discernible_value << ", max:" <<hdr_->highest_trackable_value << endl;
                free(hdr_);
                exit(0);
        }
        //set option
        rocksdb::Options options;
        SetOptions(&options, props, dbfilename);
        // std::string persistent_cache = "/localdata/pcache";
        std::string persistent_cache = "";
        // rocksdb::Status s = rocksdb::DBCloud::Open(options,dbfilename,persistent_cache,20,&db_);
        rocksdb::Status s = rocksdb::DBCloud::Open(options,dbfilename,persistent_cache,0,&db_);
        if(!s.ok()){
            cout<<"Can't open rocksdb "<<dbfilename<<" "<<s.ToString()<<endl;
            exit(0);
        }
    }

    void RocksDB::SetOptions(rocksdb::Options *options, utils::Properties &props, const char *dbfilename) {

        //// 默认的Rocksdb配置
        options->create_if_missing = true;
        options->compression = rocksdb::kNoCompression;
        options->enable_pipelined_write = true;
        options->target_file_size_base = 64 * 1024 * 1024;
        options->max_background_compactions = 16;
        options->max_subcompactions=4;
        options->use_direct_reads=true;
        options->use_direct_io_for_flush_and_compaction=true;
        options->wal_dir="/nvmedata/ycsb";

         //// 设置 512MB block cache
        rocksdb::BlockBasedTableOptions table_options;
        table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10));
        table_options.block_cache = rocksdb::NewLRUCache(512L * 1024 * 1024);
        options->table_factory.reset(NewBlockBasedTableFactory(table_options));
        ////

        int dboption = stoi(props["dboption"]);

        if ( dboption == 1) {  //use cloud db
            std::string kDBPath = dbfilename;
            std::string kBucketSuffix = "testbucket1-jx";
            std::string kRegion = "ap-northeast-1";
            // cloud environment config options here
            rocksdb::CloudFileSystemOptions cloud_fs_options;
            // // Store a reference to a cloud file system. A new cloud env object should be
            // associated with every new cloud-db.
            std::shared_ptr<rocksdb::FileSystem> cloud_fs;
            // std::cout << getenv("AWS_ACCESS_KEY_ID") << std::endl;
            // std::cout << getenv("AWS_SECRET_ACCESS_KEY") << std::endl;
            cloud_fs_options.credentials.InitializeSimple(
                getenv("AWS_ACCESS_KEY_ID"), getenv("AWS_SECRET_ACCESS_KEY"));
            if (!cloud_fs_options.credentials.HasValid().ok()) {
                fprintf(
                    stderr,
                    "Please set env variables "
                    "AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY with cloud credentials");
                return ;
            }
            // create a bucket name for debugging purposes
            const std::string bucketName = kBucketSuffix;
            // Create a new AWS cloud env Status
            rocksdb::CloudFileSystem* cfs;
            Aws::SDKOptions sdkoptions;
            Aws::InitAPI(sdkoptions);
            rocksdb::Status s;
            s = rocksdb::CloudFileSystemEnv::NewAwsFileSystem(
                rocksdb::FileSystem::Default(), kBucketSuffix, kDBPath, kRegion, kBucketSuffix,
                kDBPath, kRegion, cloud_fs_options, nullptr, &cfs);
            if (!s.ok()) {
                fprintf(stderr, "Unable to create cloud env in bucket %s. %s\n",
                        bucketName.c_str(), s.ToString().c_str());
                return ;
            }
            cloud_fs.reset(cfs);
            // Create options and use the AWS file system that we created earlier
            auto cloud_env = NewCompositeEnv(cloud_fs);
            options->env = cloud_env.release();
            options->hyper_level = 1;
            options->upload_while_generate = false;
            options->cloud_move=true;
            options->db_paths = {{kDBPath + "/ebs", 1024l * 1024 * 1024 * 1024},{kDBPath + "/s3", 1024l * 1024 * 1024 * 1024}};
        }
        else if ( dboption == 2 ) { //

        }
        
    }


    int RocksDB::Read(const std::string &table, const std::string &key, const std::vector<std::string> *fields,
                      std::vector<KVPair> &result) {
        string value;
        rocksdb::Status s = db_->Get(rocksdb::ReadOptions(),key,&value);
        if(s.ok()) {
            DeSerializeValues(value, result);
            return DB::kOK;
        }
        if(s.IsNotFound()){
            noResult++;
            return DB::kOK;
        }else{
            cerr<<"read error"<<endl;
            exit(0);
        }
    }


    int RocksDB::Scan(const std::string &table, const std::string &key, int len, const std::vector<std::string> *fields,
                      std::vector<std::vector<KVPair>> &result) {
         auto it=db_->NewIterator(rocksdb::ReadOptions());
        it->Seek(key);
        std::string val;
        std::string k;
        for(int i=0;i < len && it->Valid(); i++){
            k = it->key().ToString();
            val = it->value().ToString();
            it->Next();
        } 
        delete it;
        return DB::kOK;
    }

    int RocksDB::Insert(const std::string &table, const std::string &key,
        std::vector<KVPair> &values){
        rocksdb::Status s;
        std::string value;
        SerializeValues(values, value);
        s = db_->Put(rocksdb::WriteOptions(), key, value);
        if(!s.ok()){
            std::cerr << "Insert error: " << s.ToString() << std::endl;
            exit(1);
        }
        return DB::kOK;
    }

    int RocksDB::Update(const std::string &table, const std::string &key, std::vector<KVPair> &values) {
        return Insert(table,key,values);
    }

    int RocksDB::Delete(const std::string &table, const std::string &key) {
        rocksdb::Status s;
        s = db_->Delete(rocksdb::WriteOptions(),key);
        if(!s.ok()){
            cerr<<"Delete error\n"<<endl;
            exit(0);
        }
        return DB::kOK;
    }

    void RocksDB::PrintStats() {
        cout<<"read not found:"<<noResult<<endl;
        string stats;
        db_->GetProperty("rocksdb.stats",&stats);
        cout<<stats<<endl;
 
        cout << "SUMMARY latency (us) of this run with HDR measurement" << endl;
        cout << "         ALL      GET      PUT      UPD      SCAN    RMW" << endl;
        cout << "mean     "
            << hdr_mean(hdr_) << " "
            << hdr_mean(hdr_get_) << " "
            << hdr_mean(hdr_put_) << " "
            << hdr_mean(hdr_update_) << " "
            << hdr_mean(hdr_scan_) << " "
            << hdr_mean(hdr_rmw_) << endl;

        cout << "95th     "
            << hdr_value_at_percentile(hdr_, 95) << " "
            << hdr_value_at_percentile(hdr_get_, 95) << " "
            << hdr_value_at_percentile(hdr_put_, 95) << " "
            << hdr_value_at_percentile(hdr_update_, 95) << " "
            << hdr_value_at_percentile(hdr_scan_, 95) << " "
            << hdr_value_at_percentile(hdr_rmw_, 95) << endl;

        cout << "99th     "
            << hdr_value_at_percentile(hdr_, 99) << " "
            << hdr_value_at_percentile(hdr_get_, 99) << " "
            << hdr_value_at_percentile(hdr_put_, 99) << " "
            << hdr_value_at_percentile(hdr_update_, 99) << " "
            << hdr_value_at_percentile(hdr_scan_, 99) << " "
            << hdr_value_at_percentile(hdr_rmw_, 99) << endl;

        cout << "99.99th  "
            << hdr_value_at_percentile(hdr_, 99.99) << " "
            << hdr_value_at_percentile(hdr_get_, 99.99) << " "
            << hdr_value_at_percentile(hdr_put_, 99.99) << " "
            << hdr_value_at_percentile(hdr_update_, 99.99) << " "
            << hdr_value_at_percentile(hdr_scan_, 99.99) << " "
            << hdr_value_at_percentile(hdr_rmw_, 99.99) << endl;
    }

    bool RocksDB::HaveBalancedDistribution() {
        //return db_->HaveBalancedDistribution();
        return true;
    }

    RocksDB::~RocksDB() {
        printf("wait delete db\n");
        free(hdr_);
        free(hdr_get_);
        free(hdr_put_);
        free(hdr_update_);
        free(hdr_scan_);
        free(hdr_rmw_);
        delete db_;
        printf("delete\n");
    }

    void RocksDB::RecordTime(int op,uint64_t tx_xtime){
            if(tx_xtime > 3600000000) {
            cout << "too large tx_xtime" << endl;
        }

        hdr_record_value(hdr_, tx_xtime);

        if(op == 1){
            hdr_record_value(hdr_put_, tx_xtime);
        } else if(op == 2) {
            hdr_record_value(hdr_get_, tx_xtime);
        } else if(op == 3) {
            hdr_record_value(hdr_update_, tx_xtime);
        } else if(op == 4) {
            hdr_record_value(hdr_scan_, tx_xtime);
        } else if(op == 5) {
            hdr_record_value(hdr_rmw_, tx_xtime);
        } else {
            cout << "record time err with op error" << endl;
        }
    }

    void RocksDB::SerializeValues(std::vector<KVPair> &kvs, std::string &value) {
        value.clear();
        PutFixed64(&value, kvs.size());
        for(unsigned int i=0; i < kvs.size(); i++){
            PutFixed64(&value, kvs[i].first.size());
            value.append(kvs[i].first);
            PutFixed64(&value, kvs[i].second.size());
            value.append(kvs[i].second);
        }
    }

    void RocksDB::DeSerializeValues(std::string &value, std::vector<KVPair> &kvs){
        uint64_t offset = 0;
        uint64_t kv_num = 0;
        uint64_t key_size = 0;
        uint64_t value_size = 0;

        kv_num = DecodeFixed64(value.c_str());
        offset += 8;
        for( unsigned int i = 0; i < kv_num; i++){
            ycsbc::DB::KVPair pair;
            key_size = DecodeFixed64(value.c_str() + offset);
            offset += 8;

            pair.first.assign(value.c_str() + offset, key_size);
            offset += key_size;

            value_size = DecodeFixed64(value.c_str() + offset);
            offset += 8;

            pair.second.assign(value.c_str() + offset, value_size);
            offset += value_size;
            kvs.push_back(pair);
        }
    }
}
