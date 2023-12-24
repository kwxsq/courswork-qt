#include <QMainWindow>
#include <QLineEdit>
#include <QTimer>
#include <QTime>
#include <QTimeEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QInputDialog>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    QString isComputerNumberExists(const QString &computer_number, const QString &date, const QTime &startTime, const QTime &endTime);
    ~MainWindow();

private slots:
    void openNewWindow();
    void openNewWindowForComputerNumber();
    void openNewWindowForBooking();
    void writeToDatabaseWithComputerNumber(const QString &date);
    void writeToFile(const QString &data);
    void releaseComputer();
    void removeFromDatabase(const QString &login, const QString &date);
    void updateTime();
    void createReport();
    void openNewWindowForTimeSelection();
    void writeToDatabaseWithComputerNumberAndTime(const QString &date, const QString &time, const QTime &startTime, const QTime &endTime);
    void openSearchWindow();
    void removeFromDatabaseWithTime(const QString &login, const QString &date, const QTime &startTime, const QTime &endTime);

private:
    Ui::MainWindow *ui;
    QLineEdit* nameLineEdit;
    QLineEdit* computerNumberLineEdit;
    QTimer *timer;
    QTime *time;
    QTimeEdit *timeEdit;
};
