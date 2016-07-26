#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSql>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <QTableView>
#include <QFileDialog>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QByteArray>
#include <QComboBox>
#include <QCryptographicHash>

#define DB_HOSTNAME "192.168.128.10"
#define DB_USERNAME "volunteer"
#define DB_PASSWORD "b7e6bf7f5cf47c8f383131e709f50b70"
#define DB_NAME "vol"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QLineEdit *lineEditUserName;
    QLineEdit *lineEditPassWord;
    // QLineEdit *lineEditSearch;

    void initToolBarLineEdits();
    bool testIfLineEditEmpty();
    bool testIfhasThisRecord(QString pid);
    bool saveImageToDb(QString pid, QString filePath);
    bool initDatabase(QString hostname, QString username, QString password);
    bool connDatabase(QString hostname, QString username, QString password);
    bool portTest(QString ip, int port);
    void setQueryModel();
    void setTable(QString tableName, QSqlTableModel *&queryModel,
                  QTableView *&tableView, QSqlTableModel::EditStrategy editStrategy);
    void clearAddEdits();

    void searchBy();
    void addDepartItems();
    void addComboBoxValues(QComboBox *&combox, QString filter);
    void addEditDepartItemsLevelOne();
    void addEditDepartItemsLevelTwo();
    void showDetail();
    void updateInfo(int id);

    bool verify();
    QByteArray getImageFromDb(QString pid);



    QString gDocumentPath;

    // var
    QSqlDatabase initDbHandle;
    QSqlDatabase connDbHandle;

    QSqlTableModel *queryModel;
    QSqlTableModel *departModel;
    QSqlTableModel *familyModel;
    QSqlTableModel *eduJobModel;
    QSqlTableModel *transferModel;

private slots:
    void on_pushButtonDbInit_clicked();

    void on_actionConnect_triggered();

    void on_actionSave_triggered();

    void on_toolButtonImage_clicked();

    void on_pushButtonDepartSave_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_lineEditName_returnPressed();

    void on_pushButtonVSearch_clicked();

    void on_tableView_customContextMenuRequested(const QPoint &pos);

    void on_actionDeleteVRow_triggered();

    void on_actionCVOutput_triggered();

    void on_actionShowDetail_triggered();

    void on_tableViewDepart_customContextMenuRequested(const QPoint &pos);

    void on_comboBoxDepartLevel_currentIndexChanged(int index);

    void on_comboBoxDepart_currentIndexChanged(int index);

    void on_actionShowPersonImage_triggered();

    void on_actionClearEditWindow_triggered();

    void on_comboBoxQDepart_currentIndexChanged(const QString &arg1);

    void on_actionFamilyInfo_triggered();

    void on_pushButtonFaSave_clicked();

    void on_pushButtonTransferSave_clicked();

    void on_comboBoxTransferDepartOne_currentIndexChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_actionTransferHistory_triggered();

    void on_actionEduJob_triggered();

    void on_tableViewFamily_customContextMenuRequested(const QPoint &pos);

    void on_tableViewEduJob_customContextMenuRequested(const QPoint &pos);

    void on_tableViewTransfer_customContextMenuRequested(const QPoint &pos);

    void on_actionDeleteRow_triggered();

private:
    Ui::MainWindow *ui;
    int gRowNum;
    int deleteRow;
    QString gImageAbsPath;

    int gUserLevel;
    int gModifySwitch;
    int gId;
};

#endif // MAINWINDOW_H
