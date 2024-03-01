# FulfilMongoCpp

## Requirements 
Run the provided install script (install_mongo.sh). It will install the required mongocxx, bsoncxx, as well as the required C versions of those libraries and prerequisite debian packages.

To use the mock factory db (set up defined in scripts/start_mock_factory_db.sh) you will need to install docker. To help visualize results, install mongo and mongo compass. To connect to the mongo docker via compass, use conn string mongodb://localhost:27018.

### If boost over std smart pointers required 
Smart pointers through Boost library. Can be installed via debian package manager:
```$xslt
sudo apt-get update
sudo apt-get install -y libboost-dev && sudo apt-get install -y libboost-all-dev
```
