# Playing with Node.js template


## Configure and build project

```bash
cd $PROJECT_ROOT
mkdir -p build/release
cd build/release
export CAOS_DB_BACKEND=POSTGRESQL
# export CAOS_DB_BACKEND=MYSQL
# export CAOS_DB_BACKEND=MARIADB

# Configure
cmake -G Ninja -DCAOS_DB_BACKEND=${CAOS_DB_BACKEND} -DCAOS_PROJECT_TYPE=BINDING -DCAOS_BINDING_LANGUAGE=NODE ../../

# Build
cmake --build .
```

## Setup environment

### Redis server
```bash
export CAOS_CACHEUSER=""
export CAOS_CACHEPASS=""
export CAOS_CACHEHOST=""
export CAOS_CACHEPORT=6379
```

### Database configuration
```bash
export CAOS_DBUSER=""
export CAOS_DBPASS=""
export CAOS_DBHOST=""
export CAOS_DBPORT=
export CAOS_DBNAME=""
```

### TOKEN

```bash
export CAOS_API_TOKEN=ARBJi7cJuOYPXmFPPLVWsGrXmD4SU3LW
```

## Testing template query

```bash
node -e "const caos = require('./build/release/node_18/my_app');const result = caos.IQuery_Template_echoString({'token':'ARBJi7cJuOYPXmFPPLVWsGrXmD4SU3LW'}, 'Hello CAOSDBA\!');console.log(result);"
```

## Prepare for distribution

```bash
# Prepare DEB packages and tarball for distribution
sudo cmake --build . --target make_distribution_tarball

# Follow instruction
# sudo tar -xzf /home/mrbi314/qt-project/caos/dist/my-app-node-deb-repository-${CAOS_DB_BACKEND,,}-1.0.0+2.tar.gz -C /
# sudo /opt/caosdba/install-repository.sh
# sudo apt install my-app-node-${CAOS_DB_BACKEND,,}

# Test module globally accessible
node -e "const caos = require('my_app');const result = caos.IQuery_Template_echoString({'token':'ARBJi7cJuOYPXmFPPLVWsGrXmD4SU3LW'}, 'Hello CAOSDBA\!');console.log(result);"
```
