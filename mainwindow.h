#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QHostAddress>
#include <QNetworkInterface>
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
#include <QChart>
#define DB_HOSTNAME "127.0.0.1"
#define DB_USERNAME "test"
#define DB_PASSWORD "test"
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
    QLineEdit *lineEditSearch;

    void initToolBarLineEdits();
    void testIfLineEditEmpty();
    bool testIfhasThisRecord(QString pid);
    bool initDatabase(QString hostname, QString username, QString password);
    bool connDatabase(QString hostname, QString username, QString password);
    bool portTest(QString ip, int port);
    void setQueryModel();
    void setTable(QString tableName, QSqlTableModel *&queryModel,
                  QTableView *&tableView, QSqlTableModel::EditStrategy editStrategy);
    void clearAddEdits();

    void searchBy();

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

private:
    Ui::MainWindow *ui;
    int gRowNum;
};

#endif // MAINWINDOW_H
