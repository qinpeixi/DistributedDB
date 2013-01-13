DistributedDB
====
这是中科大软件学院2012年秋的网络程序设计课的课程项目，课程主页见：http://219.219.220.231/wiki/NP2012Fall

-----------

## 文件目录说明：
* client/   -- 客户端代码
* server/   -- 服务器端代码
* common/   -- 客户端服务器共用的代码
* test/     -- 自动化测试代码
* dbss.vim  -- vim的会话文件
* tags      -- Ctags的文件
* ToBeImproved -- 记录还需要改进的地方
* doc       -- 文档 

## Installation
* server
  1. ./ddbserver #this server will be the master server
  2. ./ddbserver masterIp masterPort #this will be slave server
  3. <CTRL>+c will shutdown a slave server
* client
  1. ./ddbclient 
