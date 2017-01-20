#include "mainwindow.h"
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    qDebug() << "Let's start!";
    setupUi(this);

    freezeSelectors = true;

    //resources
    resourcesFolder = "../";
    //test mode (auto login)
    usernameEdit->setText("Duduf");
    passwordEdit->setText("tp");

    //======== UI ============

    //default stylesheet
    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    updateCSS();

    //center login widget and server error widget
    loginPageLayout->setAlignment(loginWidget, Qt::AlignHCenter);
    loginPageLayout->setAlignment(serverWidget, Qt::AlignHCenter);

    //Add project and stage selector
    ProjectSelectorWidget *projectSelectorW = new ProjectSelectorWidget();
    mainToolBar->insertWidget(actionSettings,projectSelectorW);
    projectSelector = projectSelectorW->projectsBox();
    //connect project selector project changed
    connect(projectSelector,SIGNAL(currentIndexChanged(int)),this,SLOT(selectorProjectChanged(int)));
    stageSelector = projectSelectorW->stagesBox();

    //Add window buttons
    maximizeButton = new QPushButton(QIcon(":/icons/maximize"),"");
    QPushButton *minimizeButton = new QPushButton(QIcon(":/icons/minimize"),"");
    QPushButton *quitButton = new QPushButton(QIcon(":/icons/close"),"");
    connect(maximizeButton,SIGNAL(clicked()),this,SLOT(maximizeButton_clicked()));
    connect(minimizeButton,SIGNAL(clicked()),this,SLOT(showMinimized()));
    connect(quitButton,SIGNAL(clicked()),qApp,SLOT(quit()));
    mainToolBar->addWidget(minimizeButton);
    mainToolBar->addWidget(maximizeButton);
    mainToolBar->addWidget(quitButton);

    //statusbar
    mainStatusStopButton = new QPushButton("X");
    connect(mainStatusStopButton, SIGNAL(clicked()), this, SLOT(stopWaiting()));

    mainStatusProgress = new QProgressBar();
    mainStatusProgress->setTextVisible(false);
    mainStatusProgress->setMaximum(0);
    mainStatusProgress->setMinimum(0);
    mainStatusProgress->setMaximumWidth(100);

    mainStatusBar->addPermanentWidget(mainStatusProgress);
    mainStatusBar->addPermanentWidget(mainStatusStopButton);

    mainStatusProgress->hide();
    mainStatusStopButton->hide();

    //drag window using the toolbar
    toolBarClicked = false;
    mainToolBar->installEventFilter(this);

    //hide settings logout widget
    settingsLogoutWidget->hide();

    //hider server error widget
    serverWidget->hide();

    //========= INITIALIZE ==========
    //dbinterface
    qDebug() << "Starting DBI";
    dbi = new DBInterface(this);
    connect(dbi,SIGNAL(connected(bool, QString)),this,SLOT(connected(bool, QString)));
    connect(dbi,SIGNAL(connecting()),this,SLOT(connecting()));
    connect(dbi,SIGNAL(message(QString,int)),this,SLOT(showMessage(QString,int)));

    //connect DBI status
    connect(dbi,SIGNAL(statusAdded(bool,QString)),this,SLOT(statusAdded(bool,QString)));
    connect(dbi,SIGNAL(gotStatuses(bool,QString,QJsonValue)),this,SLOT(gotStatuses(bool,QString,QJsonValue)));
    connect(dbi,SIGNAL(statusUpdated(bool,QString)),this,SLOT(statusUpdated(bool,QString)));
    connect(dbi,SIGNAL(statusRemoved(bool,QString)),this,SLOT(statusRemoved(bool,QString)));
    //connect DBI stages
    connect(dbi,SIGNAL(stageAdded(bool,QString)),this,SLOT(stageAdded(bool,QString)));
    connect(dbi,SIGNAL(gotStages(bool,QString,QJsonValue)),this,SLOT(gotStages(bool,QString,QJsonValue)));
    connect(dbi,SIGNAL(stageUpdated(bool,QString)),this,SLOT(stageUpdated(bool,QString)));
    connect(dbi,SIGNAL(stageRemoved(bool,QString)),this,SLOT(stageRemoved(bool,QString)));
    //connect DBI projects
    connect(dbi,SIGNAL(projectAdded(bool,QString)),this,SLOT(projectAdded(bool,QString)));
    connect(dbi,SIGNAL(gotProjects(bool,QString,QJsonValue)),this,SLOT(gotProjects(bool,QString,QJsonValue)));
    connect(dbi,SIGNAL(projectUpdated(bool,QString)),this,SLOT(projectUpdated(bool,QString)));
    connect(dbi,SIGNAL(projectRemoved(bool,QString)),this,SLOT(projectRemoved(bool,QString)));
    connect(dbi,SIGNAL(projectStageAdded(bool,QString)),this,SLOT(projectStageAdded(bool,QString)));
    connect(dbi,SIGNAL(projectStageRemoved(bool,QString)),this,SLOT(projectStageRemoved(bool,QString)));
    //connect DBI shots
    connect(dbi,SIGNAL(shotAdded(bool,QString)),this,SLOT(shotAdded(bool,QString)));
    connect(dbi,SIGNAL(gotShots(bool,QString,QJsonValue)),this,SLOT(gotShots(bool,QString,QJsonValue)));
    connect(dbi,SIGNAL(shotUpdated(bool,QString)),this,SLOT(shotUpdated(bool,QString)));
    connect(dbi,SIGNAL(shotStatusUpdated(bool,QString)),this,SLOT(shotStatusUpdated(bool,QString)));
    connect(dbi,SIGNAL(shotRemoved(bool,QString)),this,SLOT(shotRemoved(bool,QString)));
    //connect DBI assets
    connect(dbi,SIGNAL(assetAdded(bool,QString)),this,SLOT(assetAdded(bool,QString)));

    //========= LOAD SETTINGS ========
    qDebug() << "Loading settings";
    settingsDB = QSqlDatabase::addDatabase("QSQLITE");
    settingsDB.setDatabaseName(resourcesFolder + "settings.s3db");
    settingsDB.setHostName("localhost");
    settingsDB.open();
    qDebug() << "SettingsDB Opened";
    //settings
    QString q = "SELECT networkSettings.serverAddress, networkSettings.ssl, networkSettings.updateFrequency, networkSettings.timeout FROM networkSettings JOIN users ON users.id = networkSettings.userID WHERE users.username = 'Default';";
    QSqlQuery networkSettingsQuery(q,settingsDB);
    networkSettingsQuery.next();
    serverAddressEdit->setText(networkSettingsQuery.value(0).toString());
    sslCheckBox->setChecked(networkSettingsQuery.value(1).toBool());
    updateFreqSpinBox->setValue(networkSettingsQuery.value(2).toInt());
    timeOutEdit->setValue(networkSettingsQuery.value(3).toInt());
    //dispatch settings
    dbi->setServerAddress(networkSettingsQuery.value(0).toString());
    dbi->setSsl(networkSettingsQuery.value(1).toBool());
    dbi->setUpdateFreq(networkSettingsQuery.value(2).toInt());

    mainStack->setCurrentIndex(0);
    loginButton->setFocus();

    //Help Dialog
    helpDialog = new HelpDialog();
    connect(helpDialog,SIGNAL(visibilityChanged(bool)),actionHelp,SLOT(setChecked(bool)));

    //detect inactivity
    connect(qApp,SIGNAL(idle()),this,SLOT(idle()));

    freezeSelectors = false;

    qDebug() << "Ready!";
}

// ========= GENERAL METHODS ========

void MainWindow::updateCSS()
{
    QFile cssFile(resourcesFolder + "style.css");
    cssFile.open(QFile::ReadOnly);
    QString css = QString(cssFile.readAll());
    cssFile.close();
    qApp->setStyleSheet(css);
}

void MainWindow::login()
{
    //check login in database, initiate
    if (usernameEdit->text() == "")
    {
        connectionStatusLabel->setText("Please fill your username in.");
        return;
    }
    if (passwordEdit->text() == "")
    {
        connectionStatusLabel->setText("Please fill your password in.");
        return;
    }
    //hash password
    QString salt = "4JZFyVWhJLEM8nan";
    QString passToHash = salt + passwordEdit->text();
    passHash = QCryptographicHash::hash(passToHash.toUtf8(), QCryptographicHash::Sha3_512).toHex();
    username = usernameEdit->text();

    showPage(0);

    loginWidget->setEnabled(false);
    connectionStatusLabel->setText("Initializing connection...");
    connectionStatusLabel->setEnabled(true);
    serverSettingsButton->hide();
    setWaiting();
    this->repaint();

    connection();
}

void MainWindow::logout()
{
    connectionStatusLabel->setText("Ready...");
    connectionStatusLabel->setEnabled(false);
    loginWidget->setEnabled(true);
    passwordEdit->setText("");

    actionMain->setEnabled(false);
    actionStage->setEnabled(false);
    actionStats->setEnabled(false);
    actionAdmin->setEnabled(false);
    actionLogout->setText("Login");
    actionLogout->setChecked(false);


    //enable network settings
    serverSettingsWidget->show();
    settingsLogoutWidget->hide();

    //empty everything
    qDeleteAll(statusesList);
    statusesList.clear();
    qDeleteAll(stagesList);
    stagesList.clear();

    showPage(0);
}

void MainWindow::showPage(int page)
{
    mainStack->setCurrentIndex(page);

    actionMain->setChecked(false);
    actionStage->setChecked(false);
    actionStats->setChecked(false);
    actionAdmin->setChecked(false);
    actionSettings->setChecked(false);

    switch (page)
    {
    case 1:
        getShots();
        actionMain->setChecked(true);
        break;
    case 2:
        actionStage->setChecked(true);
        break;
    case 3:
        actionStats->setChecked(true);
        break;
    case 4:
        actionAdmin->setChecked(true);
        break;
    case 5:
        actionSettings->setChecked(true);
        break;
    default:
        break;
    }
}

void MainWindow::setWaiting(bool w)
{
    if (w)
    {
        mainStack->setEnabled(false);
        mainStatusProgress->show();
        mainStatusStopButton->show();
    }
    else
    {
        mainStack->setEnabled(true);
        mainStatusProgress->hide();
        mainStatusStopButton->hide();
    }
}

void MainWindow::stopWaiting()
{
    setWaiting(false);
}

void MainWindow::connection()
{
    //test connexion
    dbi->connection(username,passHash);
}

void MainWindow::showMessage(QString m, int i)
{
    mainStatusBar->showMessage(m,i);
    qDebug() << m;
}

void MainWindow::idle()
{
    showMessage("Session timed out, you have been logged out.");
    logout();
}

// ========= DEV AND DEBUG ==========

void MainWindow::on_updateCSSButton_clicked()
{
    updateCSS();
}

// ========== DBI ===================

void MainWindow::connected(bool available, QString err)
{
    setWaiting(false);
    if (available)
    {
        //enable actions
        actionMain->setEnabled(true);
        actionStage->setEnabled(true);
        actionStats->setEnabled(true);
        actionAdmin->setEnabled(true);
        actionLogout->setEnabled(true);
        actionLogout->setText("Logout");
        actionLogout->setChecked(true);

        //disable network settings
        serverSettingsWidget->hide();
        settingsLogoutWidget->show();

        //load everything
        getStatuses();
        getStages();
        getProjects();

        //go to main page
        actionMain->setChecked(true);
        mainStack->setCurrentIndex(1); //show main page


    }
    else
    {
        logout();
        serverSettingsButton->show();
        serverWidget->show();
        loginWidget->setEnabled(true);
        connectionStatusLabel->setText(err);
        connectionStatusLabel->setEnabled(true);
    }
}

void MainWindow::connecting()
{
    connectionStatusLabel->setText("Connecting...");
}

// ========= BUTTONS ================

//LOGIN
void MainWindow::on_loginButton_clicked()
{
    login();
}

void MainWindow::on_serverSettingsButton_clicked()
{
    actionSettings->setChecked(true);
    actionMain->setChecked(false);
    actionStage->setChecked(false);
    actionStats->setChecked(false);
    actionAdmin->setChecked(false);
    mainStack->setCurrentIndex(5); //show settings page
    settingsWidget->setCurrentIndex(0); //show network settings
}

void MainWindow::on_usernameEdit_returnPressed()
{
    passwordEdit->setFocus();
}

void MainWindow::on_passwordEdit_returnPressed()
{
    login();
}

//SELECTORS

void MainWindow::selectorProjectChanged(int i)
{
    if (freezeSelectors) return;

    stageSelector->clear();
    mainTable->clear();

    if (i<0) return;

    foreach(RAMProject *p,projectsList)
    {
        if (projectSelector->currentData().toInt() == p->getId())
        {
            currentProject = p;
            break;
        }
    }

    if (!currentProject) return;

    //setTooltip
    projectSelector->setToolTip(currentProject->getName());

    //load stages
    currentStages = currentProject->getStages();

    mainTable->setColumnCount(currentStages.count());

    for(int i = 0 ; i < currentStages.count() ; i++)
    {
        RAMStage *s = currentStages[i];
        stageSelector->addItem(s->getShortName(),s->getId());
        //add table columns
        QTableWidgetItem *item = new QTableWidgetItem(s->getShortName());
        item->setToolTip(s->getName());

        mainTable->setHorizontalHeaderItem(i,item);
    }

    //load shots
    getShots();

}

//SETTINGS
void MainWindow::on_settingsLogoutButton_clicked()
{
    logout();
}

void MainWindow::on_serverAddressEdit_editingFinished()
{
    QString q = "UPDATE networkSettings ";
    q += "SET serverAddress = '" + serverAddressEdit->text() + "' ";
    q += "WHERE userID = (SELECT id FROM users WHERE username = 'Default');";
    QSqlQuery query(q,settingsDB);
    dbi->setServerAddress(serverAddressEdit->text());
}

void MainWindow::on_sslCheckBox_clicked(bool checked)
{
    QString q = "UPDATE networkSettings ";
    if (checked) q += "SET ssl = 1 ";
    else q += "SET ssl = 0 ";
    q += "WHERE userID = (SELECT id FROM users WHERE username = 'Default');";
    QSqlQuery query(q,settingsDB);
    dbi->setSsl(checked);
}

void MainWindow::on_updateFreqSpinBox_editingFinished()
{
    QString q = "UPDATE networkSettings ";
    q += "SET updateFrequency = " + QString::number(updateFreqSpinBox->value()) + " ";
    q += "WHERE userID = (SELECT id FROM users WHERE username = 'Default');";
    QSqlQuery query(q,settingsDB);
    dbi->setUpdateFreq(updateFreqSpinBox->value());
}

void MainWindow::on_timeOutEdit_editingFinished()
{
    QString q = "UPDATE networkSettings ";
    q += "SET timeout = " + QString::number(timeOutEdit->value()) + " ";
    q += "WHERE userID = (SELECT id FROM users WHERE username = 'Default');";
    QSqlQuery query(q,settingsDB);
}

//ADMIN GENERAL
void MainWindow::on_adminWidget_currentChanged(int index)
{
    statusesAdminReset();
    stagesAdminReset();
    projectsAdminReset();
    shotsAdminReset();
}

//ADMIN - STATUS
void MainWindow::on_statusColorButton_clicked()
{
    this->setEnabled(false);
    QColorDialog cd(QColor("#" + statusColorEdit->text()));
    cd.setOptions(QColorDialog::DontUseNativeDialog);
    cd.setWindowFlags(Qt::FramelessWindowHint);
    cd.move(this->geometry().center().x()-cd.geometry().width()/2,this->geometry().center().y()-cd.geometry().height()/2);
    if (cd.exec())
    {
        QColor color = cd.selectedColor();
        QString colorHex = color.name();
        statusColorEdit->setText(colorHex.right(6));
    }
    this->setEnabled(true);
}

//add a new default status
void MainWindow::on_addStatusButton_clicked()
{
    setWaiting();
    statusesAdminReset();
    dbi->addStatus();
}

void MainWindow::statusAdded(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    //refresh status list
    getStatuses();
}

//get all statuses
void MainWindow::getStatuses()
{
    setWaiting();
    statusesAdminReset();
    dbi->getStatuses();
}

void MainWindow::gotStatuses(bool success,QString message,QJsonValue statuses)
{
    setWaiting(false);
    if (!success) return;

    //clear UI and stored list
    statusAdminList->clear();
    qDeleteAll(statusesList);
    statusesList.clear();

    QJsonArray statusesArray = statuses.toArray();
    int newRow = -1;
    foreach (QJsonValue sta, statusesArray)
    {
        //get values
        QJsonObject status = sta.toObject();
        QString name = status.value("name").toString();
        QString shortName = status.value("shortName").toString();
        QColor color = "#" + status.value("color").toString();
        QString description = status.value("description").toString();
        int id = status.value("id").toInt();
        //create UI item
        QListWidgetItem *item = new QListWidgetItem(shortName + " | " + name);
        item->setBackgroundColor(color);
        item->setToolTip(description);
        statusAdminList->addItem(item);
        if (shortName == "New" && name == "New status")
        {
            newRow = statusAdminList->count()-1;
        }
        //add status to stored list
        RAMStatus *rs = new RAMStatus(id,name,shortName,color,description);
        statusesList << rs;
    }
    if (newRow > -1)
    {
        statusAdminList->setCurrentRow(newRow);
        on_statusAdminList_itemClicked(statusAdminList->item(newRow));
    }
}

//edit status
void MainWindow::on_statusAdminList_itemClicked(QListWidgetItem *i)
{
   int currentRow = statusAdminList->currentRow();

    RAMStatus *s = statusesList[currentRow];

    statusNameEdit->setText(s->getName());
    statusShortNameEdit->setText(s->getShortName());
    if (s->getShortName() == "STB" || s->getShortName() == "OK" || s->getShortName() == "TODO")
    {
        statusShortNameEdit->setEnabled(false);
    }
    else
    {
        statusShortNameEdit->setEnabled(true);
    }
    statusColorEdit->setText(s->getColor().name().right(6));
    statusDescriptionEdit->setPlainText(s->getDescription());

    statusConfigWidget->setEnabled(true);
}

void MainWindow::on_statusApplyButton_clicked()
{
    int currentRow = statusAdminList->currentRow();
    if (currentRow < 0) return;

    setWaiting();

    int id = statusesList[currentRow]->getId();
    dbi->updateStatus(id,statusNameEdit->text(),statusShortNameEdit->text(),statusColorEdit->text(),statusDescriptionEdit->toPlainText());

    statusesAdminReset();
}

void MainWindow::statusUpdated(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getStatuses();
}

//remove status
void MainWindow::on_removeStatusButton_clicked()
{
    int currentRow = statusAdminList->currentRow();
    if (currentRow < 0) return;

    QString sN = statusesList[currentRow]->getShortName();
    if (sN == "OK" || sN == "STB" || sN == "TODO")
    {
        showMessage("Cannot remove OK, STB and TODO statuses.");
        return;
    }


    setWaiting();

    int id = statusesList[currentRow]->getId();

    dbi->removeStatus(id);
    statusesAdminReset();
}

void MainWindow::statusRemoved(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getStatuses();
}

void MainWindow::statusesAdminReset()
{
    statusAdminList->setCurrentRow(-1);
    statusNameEdit->setText("");
    statusShortNameEdit->setText("");
    statusColorEdit->setText("");
    statusDescriptionEdit->setPlainText("");
    statusConfigWidget->setEnabled(false);
}

//ADMIN - STAGES

//add a new default stage
void MainWindow::on_addStageButton_clicked()
{
    setWaiting();
    stagesAdminReset();
    dbi->addStage();
}

void MainWindow::stageAdded(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    //refresh stages list
    getStages();
}

//get all stages
void MainWindow::getStages()
{
    setWaiting();
    stagesAdminReset();
    dbi->getStages();
}

void MainWindow::gotStages(bool success,QString message,QJsonValue stages)
{
    setWaiting(false);
    if (!success) return;

    //clear UI and stored list
    stagesAdminList->clear();
    qDeleteAll(stagesList);
    stagesList.clear();

    QJsonArray stagesArray = stages.toArray();
    int newRow = -1;
    foreach (QJsonValue sta, stagesArray)
    {
        //get values
        QJsonObject stage = sta.toObject();
        QString name = stage.value("name").toString();
        QString shortName = stage.value("shortName").toString();
        QString type = stage.value("type").toString();
        int id = stage.value("id").toInt();
        //create UI item
        QListWidgetItem *item = new QListWidgetItem(shortName + " | " + name);
        if (type == "s") item->setToolTip("Shots");
        else if (type == "a") item->setToolTip("Assets");

        stagesAdminList->addItem(item);
        if (shortName == "New" && name == "New stage")
        {
            newRow = stagesAdminList->count()-1;
        }
        //add status to stored list
        RAMStage *rs = new RAMStage(id,name,shortName,type);
        stagesList << rs;
    }
    if (newRow > -1)
    {
        stagesAdminList->setCurrentRow(newRow);
        on_stagesAdminList_itemClicked(stagesAdminList->item(newRow));
    }
}

//edit stage
void MainWindow::on_stagesAdminList_itemClicked(QListWidgetItem *item)
{
    int currentRow = stagesAdminList->currentRow();

     RAMStage *s = stagesList[currentRow];

     stageNameEdit->setText(s->getName());
     stageShortNameEdit->setText(s->getShortName()); 
     if (s->getType() == "s") stageTypeList->setCurrentIndex(0);
     else if (s->getType() == "a") stageTypeList->setCurrentIndex(1);
     stageConfigWidget->setEnabled(true);
}

void MainWindow::on_stageApplyButton_clicked()
{
    int currentRow = stagesAdminList->currentRow();
    if (currentRow < 0) return;

    setWaiting();

    int id = stagesList[currentRow]->getId();
    QString t = "";
    if (stageTypeList->currentIndex() == 0) t = "s";
    else t = "a";
    dbi->updateStage(id,stageNameEdit->text(),stageShortNameEdit->text(),t);
    stagesAdminReset();
}

void MainWindow::stageUpdated(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getStages();
}

//remove stage
void MainWindow::on_removeStageButton_clicked()
{
    int currentRow = stagesAdminList->currentRow();
    if (currentRow < 0) return;

    setWaiting();

    int id = stagesList[currentRow]->getId();
    dbi->removeStage(id);
    stagesAdminReset();
}

void MainWindow::stageRemoved(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getStages();
}

void MainWindow::stagesAdminReset()
{
    stagesAdminList->setCurrentRow(-1);
    stageNameEdit->setText("");
    stageShortNameEdit->setText("");
    stageTypeList->setCurrentIndex(0);
    stageConfigWidget->setEnabled(false);
}

//ADMIN - PROJECTS

//add a new default project
void MainWindow::on_addProjectButton_clicked()
{
    setWaiting();
    projectsAdminReset();
    dbi->addProject();
}

void MainWindow::projectAdded(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    //refresh projects list
    getProjects();
}

//get all projects
void MainWindow::getProjects()
{
    setWaiting();
    //get previous selection
    projectsAdminReset();
    //restore selection
    dbi->getProjects();
}

void MainWindow::gotProjects(bool success,QString message,QJsonValue projects)
{
    setWaiting(false);
    if (!success) return;

    freezeSelectors = true;

    //clear UI and stored list
    projectAdminList->clear();
    qDeleteAll(projectsList);
    projectsList.clear();
    projectSelector->clear();
    stageSelector->clear();

    QJsonArray projectsArray = projects.toArray();
    int newRow = -1;
    foreach (QJsonValue pro, projectsArray)
    {
        //get values
        QJsonObject project = pro.toObject();
        QString name = project.value("name").toString();
        QString shortName = project.value("shortName").toString();
        int id = project.value("id").toInt();

        //create UI item
        QListWidgetItem *item = new QListWidgetItem(shortName + " | " + name);

        projectAdminList->addItem(item);
        if (shortName == "New" && name == "New project")
        {
            newRow = projectAdminList->count()-1;
        }

        //add project to stored list
        RAMProject *rp = new RAMProject(id,name,shortName);
        projectsList << rp;

        //add stages
        QJsonArray projectStagesArray = project.value("stages").toArray();
        foreach(QJsonValue proS,projectStagesArray)
        {
            int stageId = proS.toInt();
            //get Stage
            foreach(RAMStage * stage,stagesList)
            {
                if (stage->getId() == stageId)
                {
                    rp->addStage(stage);
                    break;
                }
            }
        }

        //add to main project list
        projectSelector->addItem(rp->getShortName(),rp->getId());
    }
    if (newRow > -1)
    {
        projectAdminList->setCurrentRow(newRow);
        on_projectAdminList_itemClicked(projectAdminList->item(newRow));
    }

    freezeSelectors = false;
    selectorProjectChanged(0);
}

//edit project
void MainWindow::on_projectAdminList_itemClicked(QListWidgetItem *item)
{
    int currentRow = projectAdminList->currentRow();

     RAMProject *p = projectsList[currentRow];

     projectNameEdit->setText(p->getName());
     projectShortNameEdit->setText(p->getShortName());

     //populate stages combo box and list
     projectStagesList->clear();
     projectStagesComboBox->clear();
     QList<RAMStage*> pStages = p->getStages();
     foreach(RAMStage*s,stagesList)
     {
         //check if it is used
         bool usedByProject = false;
         foreach(RAMStage*ps,pStages)
         {
             if (ps->getId() == s->getId())
             {
                 //add to list
                 QListWidgetItem *i = new QListWidgetItem(s->getShortName());
                 i->setToolTip(s->getName());
                 projectStagesList->addItem(i);
                 usedByProject = true;
                 break;
             }
         }
         //add to combo box
         if (!usedByProject)
         {
            projectStagesComboBox->addItem(s->getShortName(),s->getId());
         }
     }

     projectConfigWidget->setEnabled(true);
}

void MainWindow::on_projectApplyButton_clicked()
{
    int currentRow = projectAdminList->currentRow();
    if (currentRow < 0) return;

    setWaiting();

    int id = projectsList[currentRow]->getId();

    dbi->updateProject(id,projectNameEdit->text(),projectShortNameEdit->text());
}

void MainWindow::projectUpdated(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getProjects();
}

//remove project
void MainWindow::on_removeProjectButton_clicked()
{
    int currentRow = projectAdminList->currentRow();
    if (currentRow < 0) return;

    setWaiting();

    int id = projectsList[currentRow]->getId();
    dbi->removeProject(id);
}

void MainWindow::projectRemoved(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getProjects();
}

//project stages
void MainWindow::on_projectAddStageButton_clicked()
{
    int sId = projectStagesComboBox->currentData().toInt();
    int pId = projectsList[projectAdminList->currentRow()]->getId();
    dbi->addProjectStage(pId,sId);
}

void MainWindow::projectStageAdded(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getProjects();
}

void MainWindow::on_removeStageProjectButton_clicked()
{
    //get stage ID
    int sId = -1;
    if (projectStagesList->currentRow() < 0) return;
    foreach(RAMStage *s,stagesList)
    {
        if (s->getShortName() == projectStagesList->currentItem()->text() && s->getName() == projectStagesList->currentItem()->toolTip())
        {
            sId = s->getId();
            break;
        }
    }

    int pId = projectsList[projectAdminList->currentRow()]->getId();
    dbi->removeProjectStage(pId,sId);
}

void MainWindow::projectStageRemoved(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getProjects();
}

void MainWindow::projectsAdminReset()
{
    projectAdminList->setCurrentRow(-1);
    projectNameEdit->setText("");
    projectShortNameEdit->setText("");
    projectConfigWidget->setEnabled(false);
    projectStagesList->clear();
    //reload stages into combo box
    projectStagesComboBox->clear();
    projectStagesList->clear();
}

//ADMIN - SHOTS

//add a new default shot "000"
void MainWindow::on_addShotButton_clicked()
{
    setWaiting();
    shotsAdminReset();
    //getProject
    int projectId = projectSelector->currentData().toInt();
    //get status
    int statusId = 0;
    foreach(RAMStatus *s,statusesList)
    {
        if (s->getShortName() == "STB")
        {
            statusId = s->getId();
            break;
        }
    }
    QStringList shotsName("000");
    dbi->addShots(projectId,statusId,shotsName);
}

void MainWindow::on_batchAddShotButton_clicked()
{
    this->setEnabled(false);
    AddShotsDialog as;
    as.move(this->geometry().center().x()-as.geometry().width()/2,this->geometry().center().y()-as.geometry().height()/2);
    as.setWindowFlags(Qt::FramelessWindowHint);
    if (as.exec())
    {
        QStringList shotNames = as.getShots();
        setWaiting();
        shotsAdminReset();
        //getProject
        int projectId = projectSelector->currentData().toInt();
        //get status
        int statusId = 0;
        foreach(RAMStatus *s,statusesList)
        {
            if (s->getShortName() == "STB")
            {
                statusId = s->getId();
                break;
            }
        }
        dbi->addShots(projectId,statusId,shotNames);
    }
    this->setEnabled(true);
}

void MainWindow::shotAdded(bool success,QString message)
{
    setWaiting(false);
    //refresh shots list
    getShots();
}

//get all shots
void MainWindow::getShots()
{
    setWaiting();
    //get previous selection
    shotsAdminReset();
    //restore selection
    int projectId = projectSelector->currentData().toInt();
    dbi->getShots(projectId);
}

void MainWindow::gotShots(bool success,QString message,QJsonValue shots)
{
    setWaiting(false);
    if (!success) return;

    //clear UI and stored list
    shotsAdminList->clear();
    qDeleteAll(shotsList);
    shotsList.clear();
    mainTable->clearContents();
    mainTable->setRowCount(0);
    mainTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mainTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QJsonArray shotsJsonArray = shots.toArray();

    //LOAD DATA
    foreach (QJsonValue shotJson, shotsJsonArray)
    {
        //get values
        QJsonObject shot = shotJson.toObject();
        QString shotName = shot.value("shotName").toString();
        double shotDuration = shot.value("duration").toDouble();
        QString shotComment = shot.value("comment").toString();
        int shotStageId = shot.value("stageId").toInt();
        int shotStatusId = shot.value("statusId").toInt();
        int shotId = shot.value("id").toInt();

        RAMShot *ramShot;
        //if shot is not already existing, create it
        bool found = false;
        foreach(RAMShot *testShot,shotsList)
        {
            if (testShot->getId() == shotId)
            {
                found = true;
                ramShot = testShot;
                break;
            }
        }

        if (!found)
        {
            ramShot = new RAMShot(shotId,shotName,shotDuration);
            connect(ramShot,SIGNAL(shotStatusUpdated(RAMStatus*,RAMStage*,RAMShot*)),this,SLOT(updateShotStatus(RAMStatus*,RAMStage*,RAMShot*)));
            shotsList << ramShot;
        }

        //find stage
        RAMStage *shotStage;
        foreach(RAMStage *stage,stagesList)
        {
            if (stage->getId() == shotStageId)
            {
                shotStage = stage;
                break;
            }
        }

        //find status
        RAMStatus *shotStatus;
        foreach(RAMStatus *status,statusesList)
        {
            if (status->getId() == shotStatusId)
            {
                shotStatus = status;
                break;
            }
        }

        if (shotStage)
        {
            //if shot production stage, add status
            if (shotStage->getType() == "s")
            {
                //add status
                RAMStageStatus *shotStageStatus = new RAMStageStatus(shotStatus,shotStage,shotComment);
                ramShot->addStatus(shotStageStatus);
            }
            //if asset production stage, add asset
            else if (shotStage->getType() == "a")
            {
                QString assetName = shot.value("assetName").toString();
                QString assetShortName = shot.value("assetShortName").toString();
                RAMAsset *shotAsset = new RAMAsset(assetName, assetShortName, shotStage, shotStatus);
                ramShot->addAsset(shotAsset);
            }
        }


    }

    //UPDATE UI
    int newRow = -1;

    foreach(RAMShot *ramShot,shotsList)
    {
        QString shotName = ramShot->getName();
        double shotDuration = ramShot->getDuration();
        QList<RAMStageStatus *> shotStatuses = ramShot->getStatuses();
        QList<RAMAsset *> shotAssets = ramShot->getAssets();

        //create admin list item
        QListWidgetItem *adminItem = new QListWidgetItem(shotName);
        shotsAdminList->addItem(adminItem);
        if (shotName == "000")
        {
            newRow = shotsAdminList->count()-1;
        }

        //create Table row
        QTableWidgetItem *rowHeader = new QTableWidgetItem(shotName);
        rowHeader->setToolTip(QString::number(shotDuration) + "s");
        mainTable->setRowCount(mainTable->rowCount() + 1);
        mainTable->setVerticalHeaderItem(mainTable->rowCount()-1,rowHeader);

        //add shot stage statuses
        foreach(RAMStageStatus * ramStatus,shotStatuses)
        {
            //stability fix (if shot was not correctly loaded, skip)
            if (!ramStatus->getStatus() && !ramStatus->getStage()) continue;

            //create widget
            ShotStatusWidget *shotStatusWidget = new ShotStatusWidget(ramShot,ramStatus,statusesList);

            //find column and add widget into cell
            for (int i = 0 ; i < currentStages.count() ; i++)
            {
                if (currentStages[i]->getId() == ramStatus->getStage()->getId())
                {
                    mainTable->setCellWidget(mainTable->rowCount()-1,i,shotStatusWidget);
                    break;
                }
            }
        }

        //add assets
        for (int i = 0 ; i < currentStages.count() ; i++)
        {
            RAMStage *stage = currentStages[i];
            if (stage->getType() == "a")
            {
                //create asset widget
                AssetStatusWidget *assetWidget = new AssetStatusWidget(ramShot,stage,statusesList,dbi);
                connect(assetWidget,SIGNAL(editing(bool)),this,SLOT(setDisabled(bool)));

                foreach(RAMAsset *asset, shotAssets)
                {
                    if (asset->getStage()->getId() == stage->getId())
                    {
                        //add asset to widget
                        assetWidget->addAsset(asset);
                    }
                }

                //add widget to cell
                mainTable->setCellWidget(mainTable->rowCount()-1,i,assetWidget);
            }
        }
    }

    mainTable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    mainTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    if (newRow > -1)
    {
        shotsAdminList->setCurrentRow(newRow);
        on_shotsAdminList_itemClicked(shotsAdminList->item(newRow));
    }
}

//edit shot
void MainWindow::on_shotsAdminList_itemClicked(QListWidgetItem *item)
{
    int currentRow = shotsAdminList->currentRow();

     RAMShot *s = shotsList[currentRow];

     shotNameEdit->setText(s->getName());
     shotDurationSpinBox->setValue(s->getDuration());
     shotConfigWidget->setEnabled(true);
}

void MainWindow::on_shotApplyButton_clicked()
{
    int currentRow = shotsAdminList->currentRow();
    if (currentRow < 0) return;

    setWaiting();

    int id = shotsList[currentRow]->getId();

    dbi->updateShot(id,shotNameEdit->text(),shotDurationSpinBox->value());
}

void MainWindow::shotUpdated(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getShots();
}

//remove shot
void MainWindow::on_removeShotButton_clicked()
{
    setWaiting();

    QList<int> shotIds;
    foreach(QListWidgetItem *i,shotsAdminList->selectedItems())
    {
        int row = i->listWidget()->row(i);
        int id = shotsList[row]->getId();
        shotIds << id;
    }
    dbi->removeShots(shotIds);
}

void MainWindow::shotRemoved(bool success,QString message)
{
    setWaiting(false);
    if (!success) return;
    getShots();
}

//Shot statuses
void MainWindow::updateShotStatus(RAMStatus* status,RAMStage* stage,RAMShot* shot)
{
    qDebug()<< "update status " + shot->getName();
    dbi->setShotStatus(status->getId(),stage->getId(),shot->getId());
}

void MainWindow::shotStatusUpdated(bool success,QString message)
{
    if (!success) connected(false,message);
}

void MainWindow::shotsAdminReset()
{
    shotsAdminList->setCurrentRow(-1);
    shotNameEdit->setText("");
    shotDurationSpinBox->setValue(0.0);
    shotConfigWidget->setEnabled(false);
}

//ADMIN - ASSETS

//add asset
void MainWindow::assetAdded(bool success,QString message)
{
    setWaiting(false);
    //refresh shots list
    getShots();
}


// ========= ACTIONS ================

void MainWindow::on_actionMain_triggered(bool checked)
{
    showPage(1);
}

void MainWindow::on_actionStage_triggered(bool checked)
{
    showPage(2);
}

void MainWindow::on_actionStats_triggered(bool checked)
{
    showPage(3);
}

void MainWindow::on_actionAdmin_triggered(bool checked)
{
    showPage(4);
}

void MainWindow::on_actionSettings_triggered(bool checked)
{
    showPage(5);
}

void MainWindow::on_actionLogout_triggered(bool checked)
{
    actionMain->setChecked(false);
    actionStage->setChecked(false);
    actionStats->setChecked(false);
    actionAdmin->setChecked(false);
    actionSettings->setChecked(false);

    if (!checked)
    {
        logout();
        actionLogout->setIcon(QIcon(":/icons/logout"));
        mainStack->setCurrentIndex(0); //show login page
    }
    else
    {
        actionLogout->setChecked(false);
        actionLogout->setIcon(QIcon(":/icons/login"));
        if (mainStack->currentIndex() == 0) login();
        else mainStack->setCurrentIndex(0);
    }
}

void MainWindow::on_actionHelp_triggered(bool checked)
{
    if (checked) helpDialog->show();
    else helpDialog->hide();
}

// ============ WINDOW BUTTONS ======

void MainWindow::maximizeButton_clicked()
{
    if (this->isMaximized())
    {
        maximizeButton->setIcon(QIcon(":/icons/maximize"));
        this->showNormal();
    }
    else
    {
        maximizeButton->setIcon(QIcon(":/icons/minimize2"));
        this->showMaximized();
    }
}

// ============ EVENTS ==============

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress)
  {
      QMouseEvent *mouseEvent = (QMouseEvent*)event;
      if (mouseEvent->button() == Qt::LeftButton)
      {
        toolBarClicked = true;
        dragPosition = mouseEvent->globalPos() - this->frameGeometry().topLeft();
        event->accept();
      }
      return true;
  }
  else if (event->type() == QEvent::MouseMove)
  {
    QMouseEvent *mouseEvent = (QMouseEvent*)event;
    if (mouseEvent->buttons() & Qt::LeftButton && toolBarClicked)
    {
        if (this->isMaximized()) this->showNormal();
      this->move(mouseEvent->globalPos() - dragPosition);
      event->accept();
    }
    return true;
  }
  else if (event->type() == QEvent::MouseButtonRelease)
  {
      toolBarClicked = false;
      return true;
  }
  else if (event->type() == QEvent::MouseButtonDblClick)
  {
      maximizeButton_clicked();
      event->accept();
      return true;
  }
  else
  {
      // standard event processing
      return QObject::eventFilter(obj, event);
  }
}