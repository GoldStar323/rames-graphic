#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include <QObject>
#include <QSqlDatabase>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkCookieJar>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>

class DBInterface : public QObject
{
    Q_OBJECT
public:
    explicit DBInterface(QObject *parent = 0);
    //parameters
    void setServerAddress(QString address);
    void setSsl(bool ssl);
    void setUpdateFreq(int freq);
    //connection
    void connection(QString user,QString passHash);
    void sendRequest(QString request, QJsonDocument content = QJsonDocument());
    //Status
    void addStatus(QString name = "New status", QString shortName = "New", QString color = "6d6d6d", QString description = "", int id = -1);
    void getStatuses();
    void updateStatus(int id, QString name, QString shortName, QString color, QString description);
    void removeStatus(int id);
    //Stage
    void addStage(QString name = "New stage", QString shortName = "New", int id = -1);
    void getStages();
    void updateStage(int id, QString name, QString shortName);
    void removeStage(int id);
    //Project
    void addProject(QString name = "New project",QString shortName = "New", int id = -1);
    void getProjects();
    void updateProject(int id, QString name, QString shortName);
    void removeProject(int id);
    void addProjectStage(int projectId, int stageId);
    void removeProjectStage(int projectId, int stageId);
    //Shot
    void addShot(int id, QString name, double duration);
    void addShots(QList<QStringList> shots);
    void insertShots(QList<int> shots, int projectId, int order);
    void insertShot(int id, int projectId, int order);
    void getShots(int projectId);
    void updateShot(int id, QString name, double duration);
    void removeShot(int id);
    void removeShots(QList<int> ids, int projectId);
    void resetShotsOrder(QList<int> ids);
    //Asset
    int addAsset(QString name, QString shortName, int statusId, int stageId, int projectId, QString comment);
    QList<int> addAssets(QList<QStringList> assets, int stageId, int projectId);
    void updateAsset(int id,QString name,QString shortName,QString comment);
    void assignAsset(int assetId, int shotId);
    void assignAssets(QList<QStringList> assignments);
    void unAssignAsset(int assetId, int shotId);
    void getAssets(int projectId);
    void setAssetStatus(int statusId, int assetId);
    void removeAsset(int id);
signals:
    void connecting();
    void connected(bool,QString);
    void message(QString,QString type);
    void data(QJsonObject d);

public slots:
    void dataReceived(QNetworkReply *rep);
    void networkError(QNetworkReply::NetworkError err);
    void sslError(QNetworkReply *rep, QList<QSslError> errs);
private:
    //LOCAL DB
    QSqlDatabase localDB;
    //PHP API
    QString serverAddress; //server
    QString protocol; //use SSL
    int updateFreq; //update freq in minutes
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    //QSqlDatabase localDB; //SQLite used for offline mode and sync
};

#endif // BDDINTERFACE_H
