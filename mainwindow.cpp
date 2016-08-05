#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIcon>
#include <QDebug>
#include <QNetworkReply>
#include <QMenu>
#include <QGridLayout>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initToolBarLineEdits();
    ui->tabWidget->setCurrentIndex(0);


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

    ui->dateEditTransfer->setDate(QDate::currentDate());
    gRowNum = -1;
    deleteRow = -1;
    gUserLevel = 1;
    gId = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

static QString md5Hash(QString str)
{
    QByteArray utf8;
    utf8.append(str.toUtf8());
    QString result = QString(QCryptographicHash::hash(utf8,QCryptographicHash::Md5).toHex());
    return result;
}



void MainWindow::initToolBarLineEdits()
{

    // UserName, PassWord
    {
        lineEditUserName = new QLineEdit;
        lineEditPassWord = new QLineEdit;
        lineEditHostIp = new QLineEdit;

        lineEditUserName->setFixedSize(100, 20);
        lineEditPassWord->setFixedSize(100, 20);
        lineEditHostIp->setFixedSize(100, 20);
        lineEditHostIp->setText("127.0.0.1");

        lineEditUserName->setPlaceholderText(" 用户名：");
        lineEditPassWord->setPlaceholderText(" 密码：");
        lineEditHostIp->setPlaceholderText(" 服务器IP: ");
        lineEditPassWord->setEchoMode(QLineEdit::PasswordEchoOnEdit);

        ui->mainToolBar->addWidget(lineEditUserName);
        ui->mainToolBar->addSeparator();

        ui->mainToolBar->addWidget(lineEditPassWord);
        ui->mainToolBar->addSeparator();
        ui->mainToolBar->addWidget(lineEditHostIp);
    }

    /*
    {
        lineEditSearch = new QLineEdit;
        lineEditSearch->setFixedSize(250, 20);
        lineEditSearch->setPlaceholderText(" 查询 <仅支持姓名/手机号/身份证号> ");
        ui->mainToolBar->addWidget(lineEditSearch);
    }
    */

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
    setTable("family", familyModel, ui->tableViewFamily, QSqlTableModel::OnFieldChange);
    setTable("edujob", eduJobModel, ui->tableViewEduJob, QSqlTableModel::OnFieldChange);
    setTable("transfer", transferModel, ui->tableViewTransfer, QSqlTableModel::OnFieldChange);


    // test mac address
    if (!verify()) {
        QMessageBox::information(this, "", "重新打开后，请输入用户名和密码。");
        qApp->closeAllWindows();
        return false;
    }

    return true;
}

bool MainWindow::portTest(QString ip, int port)
{
    QTcpSocket tsock;
    tsock.connectToHost(ip, port);
    bool ret = tsock.waitForConnected(1000);
    if (ret) tsock.close();
    return ret;
}

bool MainWindow::testIfLineEditEmpty()
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

    for(int i = 0; i < 14; i++) {
        qDebug() << stat[i];
        if (stat[i]) {
            QMessageBox::critical(this, "", "所有表格不允许为空，选择框请下拉选择。");
            return true;
        }
    }

    return false;
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
    ui->comboBoxDepartSecond->setCurrentIndex(0);
    ui->comboBoxStat->setCurrentIndex(0);
    ui->comboBoxVolunteerType->setCurrentIndex(0);
    ui->plainTextEditHopeFor->clear();
    ui->plainTextEditPast->clear();
    ui->plainTextEditWhyHere->clear();

    ui->toolButtonImage->setIcon(QIcon(""));
}

/* generated by Qt below */
void MainWindow::on_pushButtonDbInit_clicked()
{
    int ret;
    QString hostname = lineEditHostIp->text().trimmed();
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
        QMessageBox::critical(this, "", "连接服务器错误，即将退出系统。");
        qApp->closeAllWindows();
        return;
    }
    qDebug() << "func=on_actionConnect_triggered";
    ui->actionConnect->setDisabled(true);
    lineEditUserName->setDisabled(true);
    lineEditPassWord->setDisabled(true);

    addEditDepartItemsLevelOne();
}

bool MainWindow::saveImageToDb(QString pid, QString filePath)
{
    QByteArray bytes;
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly)) {
        bytes = file.readAll();
        file.close();
    }

    QSqlQuery query;
    query.prepare("insert into images (pid, image) values (:pid, :image)");
    query.bindValue(":pid", pid);
    query.bindValue(":image", bytes);

    if (query.exec()) return true;
    return false;
}

void MainWindow::updateInfo(int id)
{
    QSqlQuery query;
    query.prepare(
                "UPDATE `volunteer` "
                " SET "
                " `name` = :name , "
                " `gender` = :gender , "
                " `birthday` = :birthday , "
                " `phone` = :phone , "
                " `home` = :home , "
                " `nowhome` = :nowhome , "
                " `graduate` = :graduate , "
                " `academic` = :academic , "
                " `field` = :field , "
                " `workplace` = :workplace , "
                " `job` = :job , "
                " `hobby` = :hobby , "
                " `ifguiyi` = :ifguiyi , "
                " `code` = :code , "
                " `ifgroup` = :ifgroup , "
                " `iffzzj` = :iffzzj , "
                " `pid` = :pid , "
                " `emer` = :emer , "
                " `emerphone` = :emerphone , "
                " `startlearnbud` = :startlearnbud , "
                " `licenceplate` = :licenceplate , "
                " `email` = :email , "
                " `staydate` = :staydate , "
                " `health` = :health , "
                " `race` = :race , "
                " `marriage` = :marriage , "
                " `depart` = :depart , "
                " `departsecond` = :departsecond , "
                " `past` = :past , "
                " `whyhere` = :whyhere , "
                " `hopefor` = :hopefor , "
                " `stat` = :stat , "
                " `voltype` = :voltype "
                " WHERE `id` = :id");

    query.bindValue(":id", id);
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
    query.bindValue(":pid", ui->lineEditPID->text().trimmed().toUpper());
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
    query.bindValue(":departsecond", ui->comboBoxDepartSecond->currentText().trimmed());
    query.bindValue(":past", ui->plainTextEditPast->toPlainText());
    query.bindValue(":whyhere", ui->plainTextEditWhyHere->toPlainText());
    query.bindValue(":hopefor", ui->plainTextEditHopeFor->toPlainText());
    query.bindValue(":stat", ui->comboBoxStat->currentText().trimmed());
    query.bindValue(":voltype", ui->comboBoxVolunteerType->currentText().trimmed());
    query.exec();

    if (!gImageAbsPath.isEmpty()) {
        QFile::copy(gImageAbsPath, gDocumentPath + QDir::separator() + ui->lineEditPID->text().trimmed() + ".png");
        saveImageToDb(ui->lineEditPID->text().trimmed(), gImageAbsPath);
        gImageAbsPath = "";
    }

    clearAddEdits();
}

void MainWindow::on_actionSave_triggered()
{
    qDebug() << "gModifySwitch" << gModifySwitch;
    if (gModifySwitch) {
        updateInfo(gId);
        gId = 0;
        gModifySwitch = 0;
        return;
    }

    bool st;
    /*
    st = testIfLineEditEmpty();
    if (st) return;
    */

    st = testIfhasThisRecord(ui->lineEditPID->text().trimmed());
    if (st) return;

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
                  " `departsecond`,"
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
                  " :departsecond, "
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
    query.bindValue(":pid", ui->lineEditPID->text().trimmed().toUpper());
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
    query.bindValue(":departsecond", ui->comboBoxDepartSecond->currentText().trimmed());
    query.bindValue(":past", ui->plainTextEditPast->toPlainText());
    query.bindValue(":whyhere", ui->plainTextEditWhyHere->toPlainText());
    query.bindValue(":hopefor", ui->plainTextEditHopeFor->toPlainText());
    query.bindValue(":stat", ui->comboBoxStat->currentText().trimmed());
    query.bindValue(":voltype", ui->comboBoxVolunteerType->currentText().trimmed());
    query.exec();

    qDebug() << query.lastError().text();
    qDebug() << query.lastQuery();

    if (!gImageAbsPath.isEmpty()) {
        QFile::copy(gImageAbsPath, gDocumentPath + QDir::separator() + ui->lineEditPID->text().trimmed() + ".png");
        saveImageToDb(ui->lineEditPID->text().trimmed(), gImageAbsPath);
        gImageAbsPath = "";
    }

    clearAddEdits();
}

void MainWindow::on_toolButtonImage_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "~/", tr("Image Files (*.png)"));
    ui->toolButtonImage->setIcon(QIcon(fileName));
    gImageAbsPath = fileName;
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
    QString level = ui->comboBoxDepartLevel->currentText().trimmed();
    QString topper = ui->comboBoxDepartUpStairs->currentText().trimmed();

    QSqlQuery query;
    query.prepare("insert into `depart` (`name`, `startupdate`, `master`, `incharge`, `phone`, `tel`, `stat`, `level`, `topper`) "
                  " values (:name, :startupdate, :master, :incharge, :phone, :tel, :stat, :level, :topper)");

    query.bindValue(":name", name);
    query.bindValue(":startupdate", startupdate);
    query.bindValue(":master", master);
    query.bindValue(":incharge", incharge);
    query.bindValue(":phone", phone);
    query.bindValue(":tel", tel);
    query.bindValue(":stat", stat);
    query.bindValue(":level", level);
    query.bindValue(":topper", topper);
    query.exec();

    qDebug() << query.lastError().text() << query.lastQuery();
    departModel->select();
    ui->tableViewDepart->reset();

    addEditDepartItemsLevelOne();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{

    switch (index) {
    case 4:
        addComboBoxValues(ui->comboBoxQDepart, " level = 1");
        break;
    case 5:
        departModel->select();
        ui->tableViewDepart->reset();

        break;
    case 3:
        addComboBoxValues(ui->comboBoxTransferDepartOne, " level = 1");
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
    QString field = ui->lineEditVFieldSearch->text().trimmed();
    if (!field.isEmpty()) filter += QString(" and field like '%%1%' ").arg(field);
    QString hobby = ui->lineEditVHobbyKey->text().trimmed();
    if (!hobby.isEmpty()) filter += QString(" and hobby like '%%1%' ").arg(hobby);

    QString name = ui->lineEditVName->text().trimmed();
    if (!name.isEmpty()) filter += QString(" and name = '%1'").arg(name);

    QString home = ui->lineEditQHome->text().trimmed();
    if (!home.isEmpty()) filter += QString(" and home like '%1'").arg(home);

    QString phone = ui->lineEditQPhone->text().trimmed();
    if (!phone.isEmpty()) filter += QString(" and phone = '%1'").arg(phone);

    QString iffzzj = ui->comboBoxQFZZJ->currentText().trimmed();
    if (iffzzj.startsWith(">")) iffzzj = "";
    if (!iffzzj.isEmpty()) filter += QString(" and iffzzj = '%1' ").arg(iffzzj);
    qDebug() << "iffzzj" << iffzzj;

    QString ifgroup = ui->comboBoxQIfGroup->currentText().trimmed();
    if (ifgroup.startsWith(">")) ifgroup = "";
    if (!ifgroup.isEmpty()) filter += QString(" and ifgroup = '%1' ").arg(ifgroup);
    qDebug() << "ifgroup" << ifgroup;

    QString gender = ui->comboBoxQGender->currentText().trimmed();
    if (gender.startsWith(">")) gender = "";
    if (!gender.isEmpty()) filter += QString(" and gender = '%1' ").arg(gender);

    QString voltype = ui->comboBoxQVolunteerType->currentText().trimmed();
    if (voltype.startsWith(">")) voltype = "";
    if (!voltype.isEmpty()) filter += QString(" and voltype = '%1' ").arg(voltype);

    QString depart = ui->comboBoxQDepart->currentText().trimmed();
    if (depart.startsWith(">")) depart = "";
    if (!depart.isEmpty()) filter += QString(" and depart = '%1' ").arg(depart);

    QString departsecond = ui->comboBoxQSecondDepart->currentText().trimmed();
    if (departsecond.startsWith(">")) departsecond = "";
    if (!departsecond.isEmpty()) filter += QString(" and departsecond = '%1' ").arg(departsecond);

    QString age = ui->comboBoxQAge->currentText().trimmed();
    if (age.startsWith(">")) age = "";

    int s;
    int e;

    QString ageW = " and YEAR(CURDATE()) - YEAR(STR_TO_DATE(birthday, '%Y')) > %1 "
                " AND YEAR(CURDATE()) - YEAR(STR_TO_DATE(birthday, '%Y')) <= %2 "
                " AND STR_TO_DATE(birthday, '%Y-%m-%d') > '1900' ";

    if (age == "童年") {
        s = 0, e = 6;
    } else if (age == "少年") {
        s = 7; e = 17;
    } else if (age == "青年") {
        s = 18; e = 40;
    } else if (age == "中年") {
        s = 41; e = 65;
    } else if (age == "老年") {
        s = 66; e = 200;
    }

    if (!age.isEmpty()) {
        filter += QString(ageW).arg(s).arg(e);
    }

    qDebug() << filter;

    queryModel->setFilter(filter);
    queryModel->select();
    ui->tableView->reset();

    ui->lineEditVName->clear();
    ui->lineEditVFieldSearch->clear();
    ui->lineEditVHobbyKey->clear();
    ui->lineEditQHome->clear();
    ui->lineEditQPhone->clear();
    ui->comboBoxQAge->setCurrentIndex(0);
    ui->comboBoxQDepart->setCurrentIndex(0);
    ui->comboBoxQSecondDepart->setCurrentIndex(0);
    ui->comboBoxQFZZJ->setCurrentIndex(0);
    ui->comboBoxQGender->setCurrentIndex(0);
    ui->comboBoxQIfGroup->setCurrentIndex(0);
    ui->comboBoxQVolunteerType->setCurrentIndex(0);
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
    popMenu->addAction(ui->actionRecordHistory);
    popMenu->addAction(ui->actionTransferHistory);
    popMenu->addAction(ui->actionShowPersonImage);
    popMenu->addAction(ui->actionFamilyInfo);
    popMenu->addAction(ui->actionEduJob);
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

void MainWindow::showDetail()
{
    gId = queryModel->index(gRowNum, 0).data().toInt();
    QString name = queryModel->index(gRowNum, 1).data().toString();
    ui->lineEditName->setText(name);

    QString gender = queryModel->index(gRowNum, 2).data().toString();
    ui->comboBoxGender->setCurrentText(gender);

    QString birthday = queryModel->index(gRowNum, 3).data().toString();
    ui->lineEditBirthday->setText(birthday);

    QString phone = queryModel->index(gRowNum, 4).data().toString();
    ui->lineEditPhone->setText(phone);

    QString home = queryModel->index(gRowNum, 5).data().toString();
    ui->lineEditHome->setText(home);

    QString nowhome = queryModel->index(gRowNum, 6).data().toString();
    ui->lineEditNowHome->setText(nowhome);

    QString graduate = queryModel->index(gRowNum, 7).data().toString();
    ui->lineEditGraduate->setText(graduate);

    QString academic = queryModel->index(gRowNum, 8).data().toString();
    ui->comboBoxAcademic->setCurrentText(academic);

    QString field = queryModel->index(gRowNum, 9).data().toString();
    ui->lineEditField->setText(field);

    QString workplace = queryModel->index(gRowNum, 10).data().toString();
    ui->lineEditWorkPlace->setText(workplace);

    QString job = queryModel->index(gRowNum, 11).data().toString();
    ui->lineEditJob->setText(job);

    QString hobby = queryModel->index(gRowNum, 12).data().toString();
    ui->lineEditHobby->setText(hobby);

    QString ifguiyi = queryModel->index(gRowNum, 13).data().toString();
    ui->comboBoxIfGuiyi->setCurrentText(ifguiyi);

    QString code = queryModel->index(gRowNum, 14).data().toString();
    ui->lineEditCode->setText(code);

    QString ifgroup = queryModel->index(gRowNum, 15).data().toString();
    ui->comboBoxIfGroup->setCurrentText(ifgroup);

    QString iffzzj = queryModel->index(gRowNum, 16).data().toString();
    ui->comboBoxIfFZZJ->setCurrentText(iffzzj);

    QString pid = queryModel->index(gRowNum, 17).data().toString();
    ui->lineEditPID->setText(pid);

    QString emer = queryModel->index(gRowNum, 18).data().toString();
    ui->lineEditEmer->setText(emer);

    QString emerphone = queryModel->index(gRowNum, 19).data().toString();
    ui->lineEditEmerPhone->setText(emerphone);

    QString startlearnbud = queryModel->index(gRowNum, 20).data().toString();
    ui->lineEditStartLearnBud->setText(startlearnbud);

    QString licenceplace = queryModel->index(gRowNum, 21).data().toString();
    ui->lineEditLicencePlate->setText(licenceplace);

    QString email = queryModel->index(gRowNum, 22).data().toString();
    ui->lineEditEmail->setText(email);

    QString staydate = queryModel->index(gRowNum, 23).data().toString().left(10);
    ui->lineEditStayDate->setText(staydate);

    QString health = queryModel->index(gRowNum, 24).data().toString();
    ui->comboBoxHealth->setCurrentText(health);

    QString race = queryModel->index(gRowNum, 25).data().toString();
    ui->comboBoxRace->setCurrentText(race);

    QString marriage = queryModel->index(gRowNum, 26).data().toString();
    ui->comboBoxMariage->setCurrentText(marriage);

    QString depart = queryModel->index(gRowNum, 27).data().toString();
    ui->comboBoxDepart->setCurrentText(depart);

    QString departsecond = queryModel->index(gRowNum, 28).data().toString();
    ui->comboBoxDepartSecond->setCurrentText(departsecond);

    QString past = queryModel->index(gRowNum, 29).data().toString();
    ui->plainTextEditPast->clear();
    ui->plainTextEditPast->appendPlainText(past);

    QString whyhere = queryModel->index(gRowNum, 30).data().toString();
    ui->plainTextEditWhyHere->clear();
    ui->plainTextEditWhyHere->appendPlainText(whyhere);

    QString hopefor = queryModel->index(gRowNum, 31).data().toString();
    ui->plainTextEditHopeFor->clear();
    ui->plainTextEditHopeFor->appendPlainText(hopefor);

    QString stat = queryModel->index(gRowNum, 32).data().toString();
    ui->comboBoxStat->setCurrentText(stat);
    QString voltype = queryModel->index(gRowNum, 33).data().toString();
    ui->comboBoxVolunteerType->setCurrentText(voltype);

    QPixmap photo;
    QByteArray bytes = getImageFromDb(pid);
    photo.loadFromData(bytes);

    QString saveFileAbsPath = gDocumentPath + QDir::separator() + pid + ".png";
    photo.toImage().save(saveFileAbsPath, "PNG");
    ui->toolButtonImage->setIcon(QIcon(saveFileAbsPath));
    ui->tabWidget->setCurrentIndex(0);
    gModifySwitch = 1;
    qDebug() << "gId" << gId;
}

void MainWindow::on_actionShowDetail_triggered()
{
    showDetail();
}

void MainWindow::on_tableViewDepart_customContextMenuRequested(const QPoint &pos)
{
    int rowNum = ui->tableViewDepart->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableViewDepart->horizontalHeader()->logicalIndexAt(pos);

    qDebug() << "row num:" << rowNum << "colNum:" << colNum;

    gRowNum = rowNum;

    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionDepartShowSecond);
    popMenu->exec(QCursor::pos());

    delete popMenu;
    gRowNum = -1;

}

void MainWindow::addEditDepartItemsLevelOne()
{
    QSqlQuery query;
    int i = 1;

    ui->comboBoxDepart->clear();
    ui->comboBoxDepart->insertItem(0, "> 请选择部门");
    query.exec("select name from depart where level = 1");
    while(query.next()) {
        QString name = query.value(0).toString();
        qDebug() << i << name;
        ui->comboBoxDepart->insertItem(i, name);
        i++;
    }
}

void MainWindow::addComboBoxValues(QComboBox *&combox, QString filter)
{
    int i = 1;
    QSqlQuery query;
    combox->clear();
    combox->insertItem(0, "> 请选择部组名称");
    query.exec(QString("select name from depart where %1").arg(filter));
    while(query.next()) {
        QString name = query.value(0).toString();
        combox->insertItem(i, name);
        i++;
    }
}

void MainWindow::addEditDepartItemsLevelTwo()
{
    QString name = ui->comboBoxDepart->currentText();
    if (name.startsWith(">")) return;

    QSqlQuery query;
    int i = 1;
    ui->comboBoxDepartSecond->clear();
    ui->comboBoxDepartSecond->insertItem(0, "> 请选择二级部门");
    QString sql =  QString("select name from depart where level = 2 and topper = '%1'").arg(name);
    qDebug() << sql;
    query.exec(sql);
    while(query.next()) {
        QString name = query.value(0).toString();
        qDebug() << i << name;
        ui->comboBoxDepartSecond->insertItem(i, name);
        i++;
    }
}

void MainWindow::addDepartItems()
{
    QSqlQuery query;
    int i = 1;
    ui->comboBoxDepartUpStairs->clear();
    ui->comboBoxDepartUpStairs->insertItem(0, "> 请选择一级部门");
    query.exec("select name from depart where level = 1");
    while(query.next()) {
        QString name = query.value(0).toString();
        qDebug() << i << name;
        //ui->comboBoxDepartUpStairs->setItemText(i, name);
        ui->comboBoxDepartUpStairs->insertItem(i, name);
        i++;
    }
}

void MainWindow::on_comboBoxDepartLevel_currentIndexChanged(int index)
{
    qDebug() << "index:" << index;
    switch(index) {
    case 0:
        break;
    case 1:
        // 只有两个级别的部组，顶级部组肯定没有上级部组，所以设置为空。
        ui->comboBoxDepartUpStairs->clear();
        ui->comboBoxDepartUpStairs->insertItem(0, "");
        break;
    case 2:
        // add comboxBoxDepartUpStairs items here
        addDepartItems();
        break;
    default:
        break;
    }
}

void MainWindow::on_comboBoxDepart_currentIndexChanged(int index)
{
    if (index < 1) return;
    addEditDepartItemsLevelTwo();
}

QByteArray MainWindow::getImageFromDb(QString pid)
{
    QSqlQuery query;
    QByteArray imageByteArray;
    query.prepare("select image from images where pid = :pid");
    query.bindValue(":pid", pid);
    query.exec();
    while(query.next()) {
        imageByteArray = query.value(0).toByteArray();
    }

    return imageByteArray;
}

void MainWindow::on_actionShowPersonImage_triggered()
{

    QString pid = queryModel->index(gRowNum, 17).data().toString();
    qDebug() << pid;

    QPixmap photo;
    QByteArray bytes = getImageFromDb(pid);
    photo.loadFromData(bytes);
    QDialog *dialog = new QDialog();
    QGridLayout *layout = new QGridLayout();
    QLabel *label = new QLabel(this);
    label->setPixmap(photo);
    layout->addWidget(label);
    dialog->setLayout(layout);
    dialog->exec();

    delete label;
    delete layout;
    delete dialog;
}


bool MainWindow::verify()
{
    QSqlQuery query;
    QStringList macAddresses;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface i, list) {
        if (i.hardwareAddress().isEmpty()) continue;
            QString mac = QString("'%1'").arg(i.hardwareAddress().toUpper());
            macAddresses.append(mac);
    }

    QString where = macAddresses.join(",");
    QString sql = QString("select username, mac, password from sumeru_user where mac in (%1)").arg(where);
    query.exec(sql);

    QString name;
    QString mac;
    QString password;
    while(query.next()) {
        name = query.value(0).toString();
        mac = query.value(1).toString();
        password = query.value(2).toString();
    }

    QString inputUsername = lineEditUserName->text().trimmed();
    QString inputPassword = lineEditPassWord->text().trimmed();

    QString inputHashPassword = md5Hash(inputPassword);

    if (name.isEmpty() || inputUsername != name || inputHashPassword != password) {
        qDebug() << "name:" << name << "inputUsername:" << inputUsername;
        qDebug() << "password:" << password << "inputHashPassword:" << inputHashPassword;
        return false;
    }

    return true;
}

void MainWindow::on_actionClearEditWindow_triggered()
{
    clearAddEdits();
    ui->toolButtonImage->setIcon(QIcon(""));
    gModifySwitch = 0;
    gId = 0;
}

void MainWindow::on_comboBoxQDepart_currentIndexChanged(const QString &arg1)
{
    addComboBoxValues(ui->comboBoxQSecondDepart, QString(" level = 2 and topper = '%1'").arg(arg1));
}

void MainWindow::on_actionFamilyInfo_triggered()
{
    QString vpid = queryModel->index(gRowNum, 17).data().toString();
    ui->tabWidget->setCurrentIndex(1);
    ui->lineEditFaVolunteerPID->setText(vpid);
    familyModel->setFilter(QString(" vpid = '%1'").arg(vpid));
    familyModel->select();
    ui->tableViewFamily->reset();
}

void MainWindow::on_pushButtonFaSave_clicked()
{
    QString pid = ui->lineEditFaVolunteerPID->text().trimmed();
    QString name = ui->lineEditFaName->text().trimmed();
    QString birthday = ui->lineEditFaBirthday->text().trimmed();
    QString relation = ui->lineEditFaRelation->text().trimmed();
    QString address = ui->lineEditFaAddress->text().trimmed();
    QString phone = ui->lineEditFaPhone->text().trimmed();
    QString health = ui->comboBoxFaHealth->currentText().trimmed();

    if (pid.isEmpty() || name.isEmpty()) return;
    QSqlQuery query;

    query.prepare("insert into family(vpid, name, birthday, relation, address, phone, health) values ("
                  ":pid, :name, :birthday, :relation, :address, :phone, :health)");
    query.bindValue(":pid", pid);
    query.bindValue(":name", name);
    query.bindValue(":birthday", birthday);
    query.bindValue(":relation", relation);
    query.bindValue(":address", address);
    query.bindValue(":phone", phone);
    query.bindValue(":health", health);

    query.exec();

    familyModel->setFilter(QString(" vpid = '%1'").arg(pid));
    familyModel->select();
    ui->tableViewFamily->reset();
}

void MainWindow::on_pushButtonTransferSave_clicked()
{
    QString pid = ui->lineEditDepartPID->text().trimmed();
    if (pid.isEmpty()) return;
    QString depart = ui->comboBoxTransferDepartOne->currentText().trimmed();
    QString departSecond = ui->comboBoxTransferDepartTwo->currentText().trimmed();
    QString start = ui->dateEditTransfer->date().toString("yyyy-MM-dd");
    qDebug() << pid << depart << departSecond << start;

    ui->lineEditDepartPID->clear();

    QSqlQuery query;
    query.prepare("insert into transfer(pid, start, depart, departsecond) values (:pid, :start, :depart, :departsecond)");
    query.bindValue(":pid", pid);
    query.bindValue(":depart", depart);
    query.bindValue(":departsecond", departSecond);
    query.bindValue(":start", start);
    query.exec();

    transferModel->setFilter(QString("pid = '%1'").arg(pid));
    transferModel->select();
    ui->tableViewTransfer->reset();
}

void MainWindow::on_comboBoxTransferDepartOne_currentIndexChanged(const QString &arg1)
{
    QString departOne = arg1;
    addComboBoxValues(ui->comboBoxTransferDepartTwo, QString(" level = 2 and topper = '%1'").arg(departOne));
    qDebug() << departOne;
}

void MainWindow::on_pushButton_clicked()
{
    QString pid = ui->lineEditEPID->text().trimmed();
    QString start = ui->lineEditEStart->text().trimmed();
    QString end = ui->lineEditEEnd->text().trimmed();
    QString place = ui->lineEditEPlace->text().trimmed();
    QString notes = ui->lineEditENotes->text().trimmed();

    if (pid.isEmpty() || place.isEmpty()) return;

    QSqlQuery query;
    query.prepare("insert into edujob( pid, start, end, place, notes) values (:pid, :start, :end, :place, :notes)");
    query.bindValue(":pid", pid);
    query.bindValue(":start", start);
    query.bindValue(":end", end);
    query.bindValue(":place", place);
    query.bindValue(":notes", notes);
    query.exec();

    eduJobModel->setFilter(QString("pid = '%1'").arg(pid));
    eduJobModel->select();
    ui->tableViewEduJob->reset();
}

void MainWindow::on_actionTransferHistory_triggered()
{
    QString pid = queryModel->index(gRowNum, 17).data().toString();
    ui->tabWidget->setCurrentIndex(3);
    ui->lineEditDepartPID->setText(pid);
    transferModel->setFilter(QString(" pid = '%1'").arg(pid));
    transferModel->select();
    ui->tableViewTransfer->reset();
}

void MainWindow::on_actionEduJob_triggered()
{
    QString pid = queryModel->index(gRowNum, 17).data().toString();
    ui->tabWidget->setCurrentIndex(2);
    ui->lineEditEPID->setText(pid);
    eduJobModel->setFilter(QString(" pid = '%1'").arg(pid));
    eduJobModel->select();
    ui->tableViewEduJob->reset();
}

void MainWindow::on_tableViewFamily_customContextMenuRequested(const QPoint &pos)
{
    // 删除功能
    int rowNum = ui->tableViewFamily->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableViewFamily->horizontalHeader()->logicalIndexAt(pos);

    deleteRow = rowNum;
    if (rowNum < 0) return;
    qDebug() << rowNum << colNum;
    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionDeleteRow);
    popMenu->exec(QCursor::pos());

    delete popMenu;
}

void MainWindow::on_tableViewEduJob_customContextMenuRequested(const QPoint &pos)
{
    // 删除功能
    int rowNum = ui->tableViewEduJob->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableViewEduJob->horizontalHeader()->logicalIndexAt(pos);

    deleteRow = rowNum;
    if (rowNum < 0) return;

    qDebug() << rowNum << colNum;
    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionDeleteRow);
    popMenu->exec(QCursor::pos());

    delete popMenu;

}

void MainWindow::on_tableViewTransfer_customContextMenuRequested(const QPoint &pos)
{
    // 删除功能
    int rowNum = ui->tableViewTransfer->verticalHeader()->logicalIndexAt(pos);
    int colNum = ui->tableViewTransfer->horizontalHeader()->logicalIndexAt(pos);

    deleteRow = rowNum;
    if (rowNum < 0) return;
    qDebug() << rowNum << colNum;
    QMenu *popMenu = new QMenu(this);
    popMenu->addAction(ui->actionDeleteRow);
    popMenu->exec(QCursor::pos());
    delete popMenu;
}

void MainWindow::on_actionDeleteRow_triggered()
{
    int index = ui->tabWidget->currentIndex();
    qDebug() << "Current Tab Widget" << index;

    QSqlTableModel *model;
    switch(index) {
    // family
    case 1:
        model = *&familyModel;
        break;
    // edujob
    case 2:
        model = *&eduJobModel;
        break;
    // transfer
    case 3:
        model = *&transferModel;
        break;
    default:
        break;
    }

    QMessageBox msgBox;
    msgBox.setText("你确定删除当前行么？如果不想删除请按 Cancel 按钮.");
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int stat = msgBox.exec();
    if (stat == QMessageBox::No) {
        deleteRow = -1;
        return;
    } else {
        qDebug() << deleteRow;
        model->removeRow(deleteRow);
        model->submitAll();
        deleteRow = -1;
    }
}
