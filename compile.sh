g++ TranMgr.cpp -o TranMgr -I/usr/local/include/ -I/usr/include/ -L/usr/local/lib/ -L/usr/lib/postgresql/9.5/ -lPocoFoundation -lPocoNet -lPocoJSON -lPocoUtil -lPocoCrypto -lpqxx -lpq
ls -l
sudo /usr/local/perseuspay/bin/tm.sh stop
sudo cp TranMgr /usr/local/perseuspay/hub
sudo /usr/local/perseuspay/bin/tm.sh start
