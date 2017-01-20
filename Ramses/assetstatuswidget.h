#ifndef ASSETSTATUSWIDGET_H
#define ASSETSTATUSWIDGET_H

#include "ui_assetstatuswidget.h"
#include "ramasset.h"
#include "ramshot.h"
#include "ramstage.h"
#include "ramstatus.h"
#include "addassetdialog.h"
#include "dbinterface.h"

class AssetStatusWidget : public QWidget, private Ui::AssetStatusWidget
{
    Q_OBJECT

public:
    explicit AssetStatusWidget(RAMShot *s, RAMStage *st, QList<RAMStatus *> sl, DBInterface *d, QWidget *parent = 0);
    void addAsset(RAMAsset *asset);

private slots:
    void on_addButton_clicked();

signals:
    void editing(bool);

private:
    QList<RAMAsset *> assets;
    QList<RAMStatus *> statusesList;
    RAMShot *shot;
    RAMStage *stage;
    DBInterface *dbi;
};

#endif // ASSETSTATUSWIDGET_H