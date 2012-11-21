#include <QStringList>
#include "MongoServer.h"
#include "MongoException.h"
#include "MongoDatabase.h"
#include "settings/ConnectionRecord.h"
#include "MongoClient.h"

using namespace Robomongo;
using namespace std;

MongoServer::MongoServer(const ConnectionRecordPtr &connectionRecord) : QObject(),
    _connectionRecord(connectionRecord)
{
    _host = _connectionRecord->databaseAddress();
    _port = QString::number(_connectionRecord->databasePort());
    _address = QString("%1:%2").arg(_host).arg(_port);

//    mongo::ScopedDbConnection con("_host");
//    con->findOne();

    _connection.reset(new mongo::DBClientConnection);

    _client = new MongoClient(_address);
    connect(_client, SIGNAL(databaseNamesLoaded(QStringList)), this, SLOT(onDatabaseNameLoaded(QStringList)));
}

/**
 * @brief Try to connect to MongoDB server.
 * @throws MongoException, if fails
 */
void MongoServer::tryConnect()
{
    try {
        _connection->connect(_address.toStdString());
    }
    catch (mongo::UserException & e) {
        throw MongoException("Unable to connect");
    }
}

/**
 * @brief Try to connect to MongoDB server.
 * @throws MongoException, if fails
 */
bool MongoServer::authenticate(const QString &database, const QString &username, const QString &password)
{
    std::string errmsg;
    bool ok = _connection->auth(database.toStdString(), username.toStdString(), password.toStdString(), errmsg);

    if (!ok)
    {
        _lastErrorMessage = QString::fromStdString(errmsg);
        throw MongoException("Unable to authorize");
    }
}

void MongoServer::listDatabases()
{
    _client->loadDatabaseNames();
}

void MongoServer::onDatabaseNameLoaded(const QStringList &names)
{
    QList<MongoDatabasePtr> list;

    foreach(QString name, names)
    {
        // this not leaks:
        //MongoDatabase *ddd = new MongoDatabase(this, name);
        //delete ddd;

        // this leaks:
        MongoDatabasePtr db(new MongoDatabase(this, name));
        list.append(db);
    }

    emit databaseListLoaded(list);
}