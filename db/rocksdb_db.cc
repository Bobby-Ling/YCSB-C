//
// Created by wujy on 1/23/19.
//
#include <iostream>


#include "rocksdb_db.h"
#include "lib/coding.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include "rocksdb/cloud/db_cloud.h"
#include "rocksdb/options.h"

using namespace std;

namespace ycsbc {
    RocksDB::RocksDB(const char *dbfilename, utils::Properties &props) :noResult(0){
    
        //set option
        rocksdb::Options options;
        SetOptions(&options, props, dbfilename);
        std::string persistent_cache = "";
        rocksdb::Status s = rocksdb::DBCloud::Open(options,dbfilename,persistent_cache,0,&db_);
        if(!s.ok()){
            cerr<<"Can't open rocksdb "<<dbfilename<<" "<<s.ToString()<<endl;
            exit(0);
        }
    }

    void RocksDB::SetOptions(rocksdb::Options *options, utils::Properties &props, const char *dbfilename) {

        //// 默认的Rocksdb配置
        options->create_if_missing = true;
        options->compression = rocksdb::kNoCompression;
        options->enable_pipelined_write = true;

        options->write_buffer_size = 64 * 1024 * 1024;
        options->target_file_size_base = 64 * 1024 * 1024;
        options->max_background_compactions = 8;
        options->use_direct_reads=true;
        options->use_direct_io_for_flush_and_compaction=true;

        ////

        //int dboption = stoi(props["dboption"]);

        // if ( dboption == 1) {  //use cloud db
        std::string kDBPath = dbfilename;
        std::string kBucketSuffix = "generalbuckets-jx";
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
        options->hyper_level = 0;
        options->upload_while_generate = true;
        options->cloud_move=true;
        options->db_paths = {{kDBPath + "/ebs", 1024l * 1024 * 1024 * 1024},{kDBPath + "/s3", 1024l * 1024 * 1024 * 1024}};
        // }
        // else if ( dboption == 2 ) { //

        // }
        
    }


    int RocksDB::Read(const std::string &table, const std::string &key, const std::vector<std::string> *fields,
                      std::vector<KVPair> &result) {
        string value;
        rocksdb::Status s = db_->Get(rocksdb::ReadOptions(),key,&value);
        if(s.ok()) {
            //printf("value:%lu\n",value.size());
            DeSerializeValues(value, result);
            /* printf("get:key:%lu-%s\n",key.size(),key.data());
            for( auto kv : result) {
                printf("get field:key:%lu-%s value:%lu-%s\n",kv.first.size(),kv.first.data(),kv.second.size(),kv.second.data());
            } */
            return DB::kOK;
        }
        if(s.IsNotFound()){
            noResult++;
            //cerr<<"read not found:"<<noResult<<endl;
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
        //printf("len:%d\n",len);
        for(int i=0;i < len && it->Valid(); i++){
            k = it->key().ToString();
            val = it->value().ToString();
            //printf("i:%d key:%lu value:%lu\n",i,k.size(),val.size());
            it->Next();
        } 
        delete it;
        return DB::kOK;
    }

    int RocksDB::Insert(const std::string &table, const std::string &key,
                        std::vector<KVPair> &values){
        rocksdb::Status s;
        string value;
        SerializeValues(values,value);
        /* printf("put:key:%lu-%s\n",key.size(),key.data());
        for( auto kv : values) {
            printf("put field:key:%lu-%s value:%lu-%s\n",kv.first.size(),kv.first.data(),kv.second.size(),kv.second.data());
        } */
        s = db_->Put(rocksdb::WriteOptions(), key, value);
        if(!s.ok()){
            cerr<<"insert error\n"<<endl;
            exit(0);
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
    }

    bool RocksDB::HaveBalancedDistribution() {
        //return db_->HaveBalancedDistribution();
        return true;
    }

    RocksDB::~RocksDB() {
        printf("wait delete db\n");
        delete db_;
        printf("delete\n");
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
