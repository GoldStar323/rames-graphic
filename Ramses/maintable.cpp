#include "maintable.h"

MainTable::MainTable(DBInterface *db, Updater *up, QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    dbi = db;
    updater = up;

    mapEvents();
}

void MainTable::mapEvents()
{
    connect(updater,SIGNAL(currentProjectChanged(RAMProject*)),this,SLOT(setCurrentProject(RAMProject*)));
}

void MainTable::clean()
{
    mainTable->clearContents();
    mainTable->setRowCount(0);
    mainTable->setColumnCount(1);
}

void MainTable::setCurrentProject(RAMProject *project)
{
    clean();

#ifdef QT_DEBUG
    qDebug() << "Set current project: " + project->getShortName() + " id: " + QString::number(project->getId());
#endif

    //set current stages
    QList<RAMStage*> stages = project->getStages();
    for (int i = 0 ; i < stages.count() ; i++)
    {
        RAMStage *rs = stages[i];
        addStage(project,rs);
    }

    //set shots
    for (int i = 0; i < project->getShots().count() ; i++)
    {
        RAMShot *shot = project->getShots()[i];
        addShot(project,shot,i);
    }

    connect(project,SIGNAL(shotAdded(RAMProject*,RAMShot*,int)),this,SLOT(addShot(RAMProject*,RAMShot*,int)));
    connect(project,SIGNAL(shotRemoved(RAMProject*,RAMShot*)),this,SLOT(removeShot(RAMProject*,RAMShot*)));
    connect(project,SIGNAL(stageAdded(RAMProject*,RAMStage*)),this,SLOT(addStage(RAMProject*,RAMStage*)));
    connect(project,SIGNAL(stageRemoved(RAMProject*,RAMStage*)),this,SLOT(removeStage(RAMProject*,RAMStage*)));
}

void MainTable::addShot(RAMProject *project,RAMShot *shot, int row)
{
    if (project != updater->getCurrentProject()) return;

    //set shots
    ShotWidget *widget= new ShotWidget(shot);
    mainTable->insertRow(row);
    mainTable->setCellWidget(row,0,widget);

    //add asset widgets
    //get the assets list for this stage
    QList<RAMStage*> stages = updater->getCurrentProject()->getStages();
    for (int i = 0 ; i < stages.count() ; i++)
    {
        ShotAssetsWidget *assetWidget = new ShotAssetsWidget(shot,stages[i],dbi,updater);
        connect(assetWidget,SIGNAL(editing(bool)),this,SLOT(setDisabled(bool)));
        //add widget to cell
        mainTable->setCellWidget(row,i+1,assetWidget);
    }
}

void MainTable::removeShot(RAMProject *project,RAMShot *shot)
{
    if (project != updater->getCurrentProject()) return;
    //get line
    for (int i = 0 ; i < mainTable->rowCount() ; i++)
    {
        ShotWidget *widget = (ShotWidget*)mainTable->cellWidget(i,0);
        if (widget->getShot() == shot)
        {
            mainTable->removeRow(i);
            break;
        }
    }
}

void MainTable::addStage(RAMProject *project, RAMStage *stage)
{
    if (project != updater->getCurrentProject()) return;
    QTableWidgetItem *item = new QTableWidgetItem(stage->getShortName());
    item->setToolTip(stage->getName());
    item->setData(Qt::UserRole,stage->getId());
    mainTable->setColumnCount(mainTable->columnCount()+1);
    mainTable->setHorizontalHeaderItem(mainTable->columnCount()-1,item);

    //add asset widgets
    for(int i = 0 ; i < mainTable->rowCount() ; i++)
    {
        ShotWidget *widget = (ShotWidget*)mainTable->cellWidget(i,0);
        RAMShot *shot = widget->getShot();
        ShotAssetsWidget *assetWidget = new ShotAssetsWidget(shot,stage,dbi,updater);
        connect(assetWidget,SIGNAL(editing(bool)),this,SLOT(setDisabled(bool)));
        //add widget to cell
        mainTable->setCellWidget(i,mainTable->columnCount()-1,assetWidget);
    }
}

void MainTable::removeStage(RAMProject *project, RAMStage *stage)
{
    if (project != updater->getCurrentProject()) return;
    //get column
    for (int i = 1 ; i < mainTable->columnCount() ; i++)
    {
        int stageId = mainTable->horizontalHeaderItem(i)->data(Qt::UserRole).toInt();
        if (stageId == stage->getId())
        {
            mainTable->removeColumn(i);
            break;
        }
    }
}
