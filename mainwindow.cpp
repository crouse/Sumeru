#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIcon>
#include <QDebug>
#include <QtCharts>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initToolBarLineEdits();


    // 如果不存在则创建本软件的临时目录
    {
        gDocumentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                + QDir::separator() + "Sumeru";
        QDir *tmp = new QDir;
        if (!tmp->exists(gDocumentPath)) {
            tmp->mkpath(gDocumentPath);
        }
    }

    ui->lineEditConfGPath->setText(gDocumentPath);
    ui->lineEditConfGPath->setReadOnly(true);
    ui->lineEditConfGHostName->setText(DB_HOSTNAME);
    ui->lineEditConfGHostName->setReadOnly(true);
    ui->centralWidget->setHidden(true);

    gRowNum = -1;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initToolBarLineEdits()
{

    // UserName, PassWord
    {
        lineEditUserName = new QLineEdit;
        lineEditPassWord = new QLineEdit;

        lineEditUserName->setFixedSize(100, 20);
        lineEditPassWord->setFixedSize(100, 20);

        lineEditUserName->setStyleSheet("border-radius: 5px;");
        lineEditPassWord->setStyleSheet("border-radius: 5px;");

        lineEditUserName->setPlaceholderText(" 用户名：");
        lineEditPassWord->setPlaceholderText(" 密码：");
        lineEditPassWord->setEchoMode(QLineEdit::PasswordEchoOnEdit);

        ui->mainToolBar->addWidget(lineEditUserName);
        ui->mainToolBar->addSeparator();

        ui->mainToolBar->addWidget(lineEditPassWord);
        ui->mainToolBar->addSeparator();
    }

    // search
    {
        lineEditSearch = new QLineEdit;
        lineEditSearch->setFixedSize(250, 20);
        lineEditSearch->setStyleSheet("border-radius: 5px;");
        lineEditSearch->setPlaceholderText(" 查询 <仅支持姓名/手机号/身份证号> ");
        ui->mainToolBar->addWidget(lineEditSearch);
    }

}

void MainWindow::setTable(QString tableName, QSqlTableModel *&queryModel,
                          QTableView *&tableView, QSqlTableModel::EditStrategy editStrategy)
{
    QSqlQuery query;
    QString sql = QString("show full columns from `%1`").arg(tableName);
    query.exec(sql);

    queryModel = new QSqlTableModel(this);
    queryModel->setTable(tableName);
    queryModel->setEditStrategy(editStrategy);
    queryModel->setSort(0, Qt::AscendingOrder);

    int i = 0;
    while(query.next()) {
        QString comment = query.value(8).toString();
        queryModel->setHeaderData(i, Qt::Horizontal, comment);
        i++;
    }

    //queryModel->select();

    tableView->setModel(queryModel);
    tableView->setColumnHidden(0, true);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->reset();
}

bool MainWindow::initDatabase(QString hostname, QString username, QString password)
{
    initDbHandle = QSqlDatabase::addDatabase("QMYSQL");
    initDbHandle.setHostName(hostname);
    initDbHandle.setUserName(username);
    initDbHandle.setPassword(password);

    if(!initDbHandle.open()) {
        qDebug() << "func=initDatabase,hostname=" << hostname << ",username="
                 << username << ",err=" << initDbHandle.lastError().text();
        return false;
    }

    QSqlQuery query(initDbHandle);
    QFile sqlFile(":/files/init.sql");
    sqlFile.open(QIODevice::ReadOnly);
    QTextStream in(&sqlFile);
    QString text = in.readAll();
    QStringList list = text.split(";");

    foreach (QString str, list) {
        QString sql = str.replace("\n", "");
        if (str.isEmpty()) continue;
        query.exec(sql);
    }

    sqlFile.close();

    return true;
}

bool MainWindow::connDatabase(QString hostname, QString username, QString password)
{
    connDbHandle = QSqlDatabase::addDatabase("QMYSQL");
    connDbHandle.setDatabaseName(DB_NAME);
    connDbHandle.setHostName(hostname);
    connDbHandle.setUserName(username);
    connDbHandle.setPassword(password);

    ui->centralWidget->setHidden(false);

    if(!connDbHandle.open()) {
        qDebug() << "func=connDatabase,hostname=" << hostname << ",username="
                 << username << ",err=" << connDbHandle.lastError().text();
        return false;
    }

    qDebug() << connDbHandle.databaseName();

    setTable("volunteer", queryModel, ui->tableView, QSqlTableModel::OnFieldChange);
    setTable("depart", departModel, ui->tableViewDepart, QSqlTableModel::OnFieldChange);

    return true;
}

bool MainWindow::portTest(QString ip, int port)
{
    QTcpSocket tsock;
    tsock.connectToHost(ip, port);
    bool ret = tsock.waitForConnected(1000);
    if (ret) tsock.close();
    qDebug() << "func=portTest,ip=" << ip << ",port=" << port << ",stat=" << ret;
    return ret;
}

void MainWindow::testIfLineEditEmpty()
{
    bool stat[14] = { ui->lineEditName->text().isEmpty(),
                      ui->comboBoxGender->currentText().startsWith(">"),
                      ui->comboBoxIfGuiyi->currentText().startsWith(">"),
                      ui->comboBoxIfGroup->currentText().startsWith(">"),
                      ui->comboBoxIfFZZJ->currentText().startsWith(">"),
                      ui->comboBoxRace->currentText().startsWith(">"),
                      ui->comboBoxDepart->currentText().startsWith(">"),
                      ui->lineEditBirthday->text().isEmpty(),
                      ui->lineEditPhone->text().isEmpty(),
                      ui->lineEditEmer->text().isEmpty(),
                      ui->lineEditStayDate->text().isEmpty(),
                      ui->lineEditEmerPhone->text().isEmpty(),
                      ui->comboBoxStat->currentText().startsWith(">"),
                      ui->comboBoxVolunteerType->currentText().startsWith(">")
                    };

    qDebug() << "testIfLineEditEmpty";

    for(int i = 0; i < 14; i++) {
        qDebug() << stat[i];
        if (stat[i]) {
            QMessageBox::critical(this, "", "所有表格不允许为空，选择框请下拉选择。");
            return;
        }
    }
}

bool MainWindow::testIfhasThisRecord(QString pid)
{
    QSqlQuery query;
    query.exec( QString("select * from volunteer where pid = '%1'").arg(pid));
    qDebug() << query.lastError().text();
    qDebug() << query.lastQuery();
    while(query.next()) {
        QMessageBox::information(this, "", "数据库内有此记录");
        return true;
    }

    return false;
}

void MainWindow::clearAddEdits()
{
    ui->lineEditName->clear();
    ui->comboBoxGender->setCurrentIndex(0);
    ui->lineEditBirthday->clear();
    ui->lineEditBirthday->clear();
    ui->lineEditPhone->clear();
    ui->lineEditHome->clear();
    ui->lineEditNowHome->clear();
    ui->lineEditGraduate->clear();
    ui->comboBoxAcademic->setCurrentIndex(0);
    ui->lineEditField->clear();
    ui->lineEditWorkPlace->clear();
    ui->lineEditJob->clear();
    ui->lineEditHobby->clear();
    ui->comboBoxIfGuiyi->setCurrentIndex(0);
    ui->lineEditCode->clear();
    ui->comboBoxIfGroup->setCurrentIndex(0);
    ui->comboBoxIfFZZJ->setCurrentIndex(0);
    ui->lineEditPID->clear();
    ui->lineEditEmer->clear();
    ui->lineEditEmerPhone->clear();
    ui->lineEditStartLearnBud->clear();
    ui->lineEditLicencePlate->clear();
    ui->lineEditEmail->clear();
    ui->lineEditStayDate->clear();
    ui->comboBoxHealth->setCurrentIndex(0);
    ui->comboBoxRace->setCurrentIndex(0);
    ui->comboBoxMariage->setCurrentIndex(0);
    ui->comboBoxDepart->setCurrentIndex(0);
    ui->comboBoxStat->setCurrentIndex(0);
    ui->comboBoxVolunteerType->setCurrentIndex(0);
    ui->plainTextEditHopeFor->clear();
    ui->plainTextEditPast->clear();
    ui->plainTextEditWhyHere->clear();
}

/* generated by Qt below */
void MainWindow::on_pushButtonDbInit_clicked()
{
    int ret;
    QString hostname = ui->lineEditDbHost->text().trimmed();
    QString username = ui->lineEditDbUsername->text().trimmed();
    QString password = ui->lineEditDbPassword->text().trimmed();

    ret = portTest(hostname, 3306);
    if (!ret) {
        QMessageBox::critical(this, "", "CAN NOT CONNECT TO PORT 3306");
        return;
    }

    ret = initDatabase(hostname, username, password);
    if (!ret) QMessageBox::critical(this, "", "INIT DATABASE ERROR");
}

void MainWindow::on_actionConnect_triggered()
{
    int ret = connDatabase(DB_HOSTNAME, DB_USERNAME, DB_PASSWORD);
    if (!ret) {
        QMessageBox::critical(this, "", "CONNECT DATABASE ERROR");
        return;
    }
    qDebug() << "func=on_actionConnect_triggered";
    ui->actionConnect->setDisabled(true);
    lineEditUserName->setDisabled(true);
    lineEditPassWord->setDisabled(true);
}

void MainWindow::on_actionSave_triggered()
{
    testIfLineEditEmpty();
    bool stat = testIfhasThisRecord(ui->lineEditPID->text().trimmed());
    if (stat) return;
    QSqlQuery query;
    query.prepare(" INSERT INTO `volunteer` ( "
                  " `name`, "
                  " `gender`, "
                  " `birthday`, "
                  " `phone`, "
                  " `home`, "
                  " `nowhome`, "
                  " `graduate`, "
                  " `academic`, "
                  " `field`, "
                  " `workplace`, "
                  " `job`, "
                  " `hobby`, "
                  " `ifguiyi`, "
                  " `code`, "
                  " `ifgroup`, "
                  " `iffzzj`, "
                  " `pid`, "
                  " `emer`, "
                  " `emerphone`, "
                  " `startlearnbud`, "
                  " `licencePlate`, "
                  " `email`, "
                  " `staydate`,"
                  " `health`,"
                  " `race`,"
                  " `marriage`,"
                  " `depart`,"
                  " `past`,"
                  " `whyhere`,"
                  " `hopefor`,"
                  " `stat`,"
                  " `voltype`"
                  ") "
                  " VALUES "
                  " (:name, "
                  " :gender, "
                  " :birthday, "
                  " :phone, "
                  " :home, "
                  " :nowhome, "
                  " :graduate, "
                  " :academic, "
                  " :field, "
                  " :workplace, "
                  " :job, "
                  " :hobby, "
                  " :ifguiyi, "
                  " :code, "
                  " :ifgroup, "
                  " :iffzzj, "
                  " :pid, "
                  " :emer, "
                  " :emerphone, "
                  " :startlearnbud, "
                  " :licenseplate, "
                  " :email, "
                  " :staydate, "
                  " :health, "
                  " :race, "
                  " :marriage, "
                  " :depart, "
                  " :past, "
                  " :whyhere, "
                  " :hopefor, "
                  " :stat,"
                  " :voltype"
                  " ) "
                  );

    query.bindValue(":name", ui->lineEditName->text().trimmed());
    query.bindValue(":gender", ui->comboBoxGender->currentText().trimmed());
    query.bindValue(":birthday", ui->lineEditBirthday->text().trimmed());
    query.bindValue(":phone", ui->lineEditPhone->text().trimmed());
    query.bindValue(":home", ui->lineEditHome->text().trimmed());
    query.bindValue(":nowhome", ui->lineEditNowHome->text().trimmed());
    query.bindValue(":graduate", ui->lineEditGraduate->text().trimmed());
    query.bindValue(":academic", ui->comboBoxAcademic->currentText().trimmed());
    query.bindValue(":field", ui->lineEditField->text().trimmed());
    query.bindValue(":workplace", ui->lineEditWorkPlace->text().trimmed());
    query.bindValue(":job", ui->lineEditJob->text().trimmed());
    query.bindValue(":hobby", ui->lineEditHobby->text().trimmed());
    query.bindValue(":ifguiyi", ui->comboBoxIfGuiyi->currentText().trimmed());
    query.bindValue(":code", ui->lineEditCode->text().trimmed());
    query.bindValue(":ifgroup", ui->comboBoxIfGroup->currentText().trimmed());
    query.bindValue(":iffzzj", ui->comboBoxIfFZZJ->currentText().trimmed());
    query.bindValue(":pid", ui->lineEditPID->text().trimmed());
    query.bindValue(":emer", ui->lineEditEmer->text().trimmed());
    query.bindValue(":emerphone", ui->lineEditEmerPhone->text().trimmed());
    query.bindValue(":startlearnbud", ui->lineEditStartLearnBud->text().trimmed());
    query.bindValue(":licenceplate", ui->lineEditLicencePlate->text().trimmed());
    query.bindValue(":email", ui->lineEditEmail->text().trimmed());
    query.bindValue(":staydate", ui->lineEditStayDate->text().trimmed());
    query.bindValue(":health", ui->comboBoxHealth->currentText().trimmed());
    query.bindValue(":race", ui->comboBoxRace->currentText().trimmed());
    query.bindValue(":marriage", ui->comboBoxMariage->currentText().trimmed());
    query.bindValue(":depart", ui->comboBoxDepart->currentText().trimmed());
    query.bindValue(":past", ui->plainTextEditPast->toPlainText());
    query.bindValue(":whyhere", ui->plainTextEditWhyHere->toPlainText());
    query.bindValue(":hopefor", ui->plainTextEditHopeFor->toPlainText());
    query.bindValue(":stat", ui->comboBoxStat->currentText().trimmed());
    query.bindValue(":voltype", ui->comboBoxVolunteerType->currentText().trimmed());
    query.exec();

    qDebug() << query.lastError().text();
    qDebug() << query.lastQuery();
    qDebug() << "func=on_actionSave_triggered";
    clearAddEdits();
}

void MainWindow::on_actionSearch_triggered()
{
    int tabIndex = ui->tabWidget->currentIndex();
    qDebug() << "tab: " << tabIndex << ui->tabWidget->tabText(tabIndex);
    switch(tabIndex) {
    case 1:
        break;
    default:
        break;
    }
    qDebug() << "func=on_actionSearch_triggered,tabIndex=" << tabIndex;
}

void MainWindow::on_toolButtonImage_clicked()
{
    // 选择需要上传的照片，先放到默认路径，然后上传到服务器，照片名称用身份证的 MD5 散列
    qDebug() << "upload image";
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "~/", tr("Image Files (*.png)"));
    qDebug() << fileName;
    ui->toolButtonImage->setIcon(QIcon(fileName));
}

void MainWindow::on_pushButtonDepartSave_clicked()
{
    QString name = ui->lineEditDepartName->text().trimmed();
    QString startupdate = ui->lineEditDepartStartupDate->text().trimmed();
    QString master = ui->lineEditDepartMaster->text().trimmed();
    QString incharge = ui->lineEditDepartIncharge->text().trimmed();
    QString phone = ui->lineEditDepartPhone->text().trimmed();
    QString tel = ui->lineEditDepartTel->text().trimmed();
    QString stat = ui->lineEditDepartStat->text().trimmed();

    QSqlQuery query;
    query.prepare("insert into `depart` (`name`, `startupdate`, `master`, `incharge`, `phone`, `tel`, `stat`) "
                  " values (:name, :startupdate, :master, :incharge, :phone, :tel, :stat)");

    query.bindValue(":name", name);
    query.bindValue(":startupdate", startupdate);
    query.bindValue(":master", master);
    query.bindValue(":incharge", incharge);
    query.bindValue(":phone", phone);
    query.bindValue(":tel", tel);
    query.bindValue(":stat", stat);
    query.exec();

    qDebug() << query.lastError().text() << query.lastQuery();
    departModel->select();
    ui->tableViewDepart->reset();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{

    switch (index) {
    case 1:
        departModel->select();
        ui->tableViewDepart->reset();
        break;
    default:
        break;
    }
}

void MainWindow::on_lineEditName_returnPressed()
{
    QString name = ui->lineEditName->text().trimmed();
    if (name.isEmpty()) return;

    QString filter = QString(" name = '%1'").arg(name);
    ui->tabWidget->setCurrentIndex(1);
    queryModel->setFilter(filter);
    queryModel->select();
    ui->tableView->reset();
}

void MainWindow::searchBy()
{
    QString filter = " 1=1 ";
    QString depart = ui->comboBoxVDepart->currentText().trimmed();
    if (depart.startsWith(">")) depart = "";
    if (!depart.isEmpty()) filter += QString(" and depart = '%1'").arg(depart);
    QString field = ui->lineEditVFieldSearch->text().trimmed();
    if (!field.isEmpty()) filter += QString(" and field like '%%1%' ").arg(field);
    QString hobby = ui->lineEditVHobbyKey->text().trimmed();
    if (!hobby.isEmpty()) filter += QString(" and hobby like '%%1%' ").arg(hobby);
    QString name = ui->lineEditVName->text().trimmed();
    if (!name.isEmpty()) filter += QString(" and name = '%1'").arg(name);

    qDebug() << filter;

    queryModel->setFilter(filter);
    queryModel->select();
    ui->tableView->reset();
}

void MainWindow::on_pushButtonVSearch_clicked()
{
    searchBy();
}

void MainWindow::on_tableView_customContextMenuRequested(const QPoint &pos)
{
    int rowNum = ui->tableView->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableView->horizontalHeader()->logicalIndexAt(pos);

    qDebug() << "row num:" << rowNum << "colNum:" << colNum;

    gRowNum = rowNum;

    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionDeleteVRow);
    popMenu->addAction(ui->actionCVOutput);
    popMenu->addAction(ui->actionShowDetail);
    popMenu->exec(QCursor::pos());

    delete popMenu;
    gRowNum = -1;
}

void MainWindow::on_actionDeleteVRow_triggered()
{
    if (gRowNum < 0) return;

    QMessageBox msgBox;
    msgBox.setText("你确定删除当前行么？如果不想删除请按 Cancel 按钮.");
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int stat = msgBox.exec();
    if (stat == QMessageBox::No) {
        return;
    } else {
        queryModel->removeRow(gRowNum);
        queryModel->submitAll();
        searchBy();
    }
    return;
}

void MainWindow::on_actionCVOutput_triggered()
{
    if (gRowNum < 0) return;
}
