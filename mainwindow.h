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
#define DB_PASSWORD "amit"
#define DB_NAME "vol"
#define FTP_PATH "/home/sumeru/images"
#define FTP_USER "sumeru"
#define FTP_PASSWORD "7d1d4f8c0e"

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

private slots:
    void on_pushButtonDbInit_clicked();

    void on_actionConnect_triggered();

    void on_actionSave_triggered();

    void on_actionSearch_triggered();

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

private:
    Ui::MainWindow *ui;
    int gRowNum;
    QString gImageAbsPath;

    int gUserLevel;
    int gModifySwitch;
    int gId;
};

#endif // MAINWINDOW_H
