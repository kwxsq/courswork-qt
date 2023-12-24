#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //главное окно
    ui->setupUi(this);
    this -> setGeometry(650, 250, 320, 200);
    // Установка заголовка окна
    this->setWindowTitle("Учет посещений пк клуба");

    //кнопка занять пк
    QPushButton *btnOccupy = new QPushButton("Занять пк", this);
    connect(btnOccupy, &QPushButton::clicked, this, &MainWindow::openNewWindow);

    //кнопка бронирования пк
    QPushButton *btnRegister = new QPushButton("Бронирование пк", this);
    connect(btnRegister, &QPushButton::clicked, this, &MainWindow::openNewWindowForBooking);

    //кнопка освободить пк
    QPushButton *btnRelease = new QPushButton("Освободить пк", this);
    connect(btnRelease, &QPushButton::clicked, this, &MainWindow::releaseComputer);

    // Инициализация таймера и времени
    timer = new QTimer(this);
    time = new QTime(0, 0);

    // Инициализация QTimeEdit для отображения времени
    timeEdit = new QTimeEdit(this);
    timeEdit->setDisplayFormat("hh:mm:ss"); // Установите формат отображения времени
    timeEdit->setReadOnly(true); // Сделайте QTimeEdit неизменяемым

    // Подключение сигнала таймера к слоту для обновления времени
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);

    // Запуск таймера с интервалом в 1 секунду
    timer->start(1000);

    //кнопка отчета пк клуба
    QPushButton *btnReport = new QPushButton("Отчет пк клуба", this);
    connect(btnReport, &QPushButton::clicked, this, &MainWindow::createReport);

    // Создание вертикального менеджера компановки
    QVBoxLayout *vLayout = new QVBoxLayout;

    // Добавление кнопок в менеджер компановки
    vLayout->addWidget(btnOccupy);
    vLayout->addWidget(btnRegister);
    vLayout->addWidget(btnRelease);
    vLayout->addWidget(timeEdit);
    vLayout->addWidget(btnReport); // Добавьте эту строку

    // Установка менеджера компановки для главного окна
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(vLayout);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime() {
    *time = time->addSecs(1);
    timeEdit->setTime(*time); // Обновите время в QTimeEdit
}
void MainWindow::openNewWindow()
{
    bool ok;
    QString login = QInputDialog::getText(this, "Занять компьютер", "Введите ваш логин:", QLineEdit::Normal, "", &ok);
    if (ok && !login.isEmpty()) {
        nameLineEdit = new QLineEdit;
        nameLineEdit->setText(login);

        // Получите текущую дату
        QString currentDate = QDate::currentDate().toString("dd.MM.yyyy");

        // Получите все номера компьютеров из базы данных, которые заняты на текущую дату
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("database_pk_club.db");

        if (!db.open())
        {
            qDebug() << "Error: connection with database failed";
        }
        else
        {
            qDebug() << "Database: connection ok";
        }

        QSqlQuery query;
        query.prepare("SELECT DISTINCT computer_number FROM database_pk_club WHERE date = :date");
        query.bindValue(":date", currentDate);

        QStringList occupiedComputerNumbers;
        if(query.exec())
        {
            while (query.next()) {
                occupiedComputerNumbers << query.value(0).toString();
            }
        }

        // Создайте список доступных номеров компьютеров
        QStringList availableComputerNumbers;
        for (int i = 1; i <= 9; ++i) {
            QString number = QString::number(i);
            if (!occupiedComputerNumbers.contains(number)) {
                availableComputerNumbers << number;
            }
        }

        QString text = "Сегодняшняя дата: " + currentDate + "\nВыберите номер компьютера:";

        QString computerNumber = QInputDialog::getItem(this, "Введите номер компьютера", text, availableComputerNumbers, 0, false, &ok);
        if (ok && !computerNumber.isEmpty()) {
            computerNumberLineEdit = new QLineEdit;
            computerNumberLineEdit->setText(computerNumber);

            QStringList items;
            items << "на час" << "на два часа" << "на три часа";

            QString text = "На сколько хотите занять пк:";

            QString item = QInputDialog::getItem(this, "На сколько хотите занять пк", text, items, 0, false, &ok);
            if (ok && !item.isEmpty()) {
                // Здесь вы можете обработать выбранное время
                QString time;
                int hoursToAdd = 0;
                if (item == "на час") {
                    time = "1h";
                    hoursToAdd = 1;
                } else if (item == "на два часа") {
                    time = "2h";
                    hoursToAdd = 2;
                } else if (item == "на три часа") {
                    time = "3h";
                    hoursToAdd = 3;
                }

                // Получите текущее время и округлите его до ближайшего часа
                QTime currentTime = QTime::currentTime();
                QTime startTime = QTime(currentTime.hour(), 0);
                QTime endTime = startTime.addSecs(hoursToAdd * 3600);

                // Сохраните время в базу данных
                writeToDatabaseWithComputerNumberAndTime(currentDate, time, startTime, endTime);
            }
        }
    }
}
void MainWindow::openNewWindowForComputerNumber()
{
    bool ok;
    QString computerNumber = QInputDialog::getText(this, "Введите номер компьютера", "Введите номер компьютера:", QLineEdit::Normal, "", &ok);
    if (ok && !computerNumber.isEmpty()) {
        computerNumberLineEdit = new QLineEdit;
        computerNumberLineEdit->setText(computerNumber);
        openNewWindowForTimeSelection();
    }
}

void MainWindow::openNewWindowForTimeSelection()
{
    bool ok;
    QStringList items;
    items << "на час" << "на два часа" << "на три часа";

    // Получите текущую дату и добавьте ее в текстовую строку
    QString currentDate = QDate::currentDate().toString("dd.MM.yyyy");
    QString text = "Сегодняшняя дата: " + currentDate + "\nНа сколько садитесь за пк:";

    QString item = QInputDialog::getItem(this, "На сколько хотите занять пк", text, items, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        // Здесь вы можете обработать выбранное время
        QString time;
        int hoursToAdd = 0;
        if (item == "на час") {
            time = "1h";
            hoursToAdd = 1;
        } else if (item == "на два часа") {
            time = "2h";
            hoursToAdd = 2;
        } else if (item == "на три часа") {
            time = "3h";
            hoursToAdd = 3;
        }

        // Получите текущее время и округлите его до ближайшего часа
        QTime currentTime = QTime::currentTime();
        QTime startTime = QTime(currentTime.hour(), 0);
        QTime endTime = startTime.addSecs(hoursToAdd * 3600);

        // Сохраните время в базу данных
        writeToDatabaseWithComputerNumberAndTime(currentDate, time, startTime, endTime);
    }
}
void MainWindow::writeToDatabaseWithComputerNumberAndTime(const QString &date, const QString &time, const QTime &startTime, const QTime &endTime)
{
    // Проверка на существование записи
    QString error = isComputerNumberExists(computerNumberLineEdit->text(), date, startTime, endTime);
    if (!error.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Компьютер уже занят на выбранную дату и время.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.prepare("INSERT INTO database_pk_club (login, computer_number, time, date, start, end) VALUES (:login, :computer_number, :time, :date, :start, :end)");
    query.bindValue(":login", nameLineEdit->text());
    query.bindValue(":computer_number", computerNumberLineEdit->text());
    query.bindValue(":time", time); // Добавьте время
    query.bindValue(":date", date); // Используйте выбранную дату
    query.bindValue(":start", startTime.toString("hh:mm:ss"));
    query.bindValue(":end", endTime.toString("hh:mm:ss"));

    if(!query.exec())
    {
        qDebug() << "Error inserting into the table.";
    } else {
        writeToFile("Логин: " + nameLineEdit->text() + ", Номер пк: " + computerNumberLineEdit->text() + ", Время: " + time + ", Дата: " + date);
    }
}
void MainWindow::releaseComputer()
{
    // Получите все логины из базы данных
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.prepare("SELECT DISTINCT login FROM database_pk_club");

    QStringList logins;
    if(query.exec())
    {
        while (query.next()) {
            logins << query.value(0).toString();
        }
    }
    // Предложите выбрать логин из базы данных
    bool ok;
    QString login = QInputDialog::getItem(this, "Освободить пк", "Выберите ваш логин:", logins, 0, false, &ok);
    if (ok && !login.isEmpty()) {
        // Получите все даты для выбранного логина
        query.prepare("SELECT DISTINCT date FROM database_pk_club WHERE login = :login");
        query.bindValue(":login", login);

        QStringList dates;
        if(query.exec())
        {
            while (query.next()) {
                dates << query.value(0).toString();
            }
        }

        // Предложите выбрать дату
        QString date = QInputDialog::getItem(this, "Освободить пк", "Выберите дату:", dates, 0, false, &ok);
        if (ok && !date.isEmpty()) {
            // Получите все времена для выбранного логина и даты
            query.prepare("SELECT start, end FROM database_pk_club WHERE login = :login AND date = :date");
            query.bindValue(":login", login);
            query.bindValue(":date", date);

            QStringList times;
            if(query.exec())
            {
                while (query.next()) {
                    times << query.value(0).toString() + " - " + query.value(1).toString();
                }
            }

            // Предложите выбрать время
            QString time = QInputDialog::getItem(this, "Освободить пк", "Выберите время:", times, 0, false, &ok);
            if (ok && !time.isEmpty()) {
                QStringList timeParts = time.split(" - ");
                QTime startTime = QTime::fromString(timeParts[0], "hh:mm:ss");
                QTime endTime = QTime::fromString(timeParts[1], "hh:mm:ss");

                removeFromDatabaseWithTime(login, date, startTime, endTime);
            }
        }
    }
}
void MainWindow::openNewWindowForBooking()
{
    bool ok;
    QString login = QInputDialog::getText(this, "Бронирование пк", "Введите ваш логин:", QLineEdit::Normal, "", &ok);
    if (ok && !login.isEmpty()) {
        nameLineEdit = new QLineEdit;
        nameLineEdit->setText(login);

        QStringList dates;
        for (int i = 0; i < 16; ++i) {
            dates << QDate::currentDate().addDays(i).toString("dd.MM.yyyy");
        }

        QString selectedDate = QInputDialog::getItem(this, "Бронирование пк", "Выберите дату:", dates, 0, false, &ok);
        if (ok && !selectedDate.isEmpty()) {
            QStringList allComputerNumbers;
            for (int i = 1; i <= 9; ++i) {
                allComputerNumbers << QString::number(i);
            }

            QString computerNumber = QInputDialog::getItem(this, "Введите номер компьютера", "Введите номер компьютера:", allComputerNumbers, 0, false, &ok);
            if (ok && !computerNumber.isEmpty()) {
                computerNumberLineEdit = new QLineEdit;
                computerNumberLineEdit->setText(computerNumber);

                QStringList availableTimes;
                for (int i = 0; i < 24; ++i) {
                    availableTimes << QTime(i, 0).toString("hh:mm");
                }

                QString selectedTime = QInputDialog::getItem(this, "Выберите время", "Выберите время:", availableTimes, 0, false, &ok);
                if (ok && !selectedTime.isEmpty()) {
                    QTime startTime = QTime::fromString(selectedTime, "hh:mm");

                    QStringList items;
                    items << "на час" << "на два часа" << "на три часа";

                    QString text = "На сколько хотите забронировать пк:";

                    QString item = QInputDialog::getItem(this, "На сколько хотите забронировать пк", text, items, 0, false, &ok);
                    if (ok && !item.isEmpty()) {
                        QString time;
                        int hoursToAdd = 0;
                        if (item == "на час") {
                            time = "1h";
                            hoursToAdd = 1;
                        } else if (item == "на два часа") {
                            time = "2h";
                            hoursToAdd = 2;
                        } else if (item == "на три часа") {
                            time = "3h";
                            hoursToAdd = 3;
                        }

                        QTime endTime = startTime.addSecs(hoursToAdd * 3600);
                        if (endTime > QTime(23, 59, 59)) {
                            QMessageBox::critical(this, "Ошибка", "Бронирование не может продолжаться после полуночи.");
                            return;
                        }

                        QTime checkStartTime = QTime(0, 0); // Ваше начальное время 00:00
                        QTime checkEndTime = QTime(23, 0); // Ваше конечное время 23:59:59
                        QString error = isComputerNumberExists(computerNumber, selectedDate, checkStartTime, checkEndTime);
                        if (!error.isEmpty()) {
                            QMessageBox::critical(this, "Ошибка", "Компьютер уже занят на выбранную дату.");
                            return;
                        }

                        writeToDatabaseWithComputerNumberAndTime(selectedDate, time, startTime, endTime);
                    }
                }
            }
        }
    }
}
void MainWindow::writeToFile(const QString &data)
{
    QFile file("output.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << data << "\n";
        file.close();
    } else {
        qDebug() << "Error opening the file.";
    }
}

void MainWindow::writeToDatabaseWithComputerNumber(const QString &date)
{
    // Проверка на существование записи
    QTime startTime = QTime(0, 0); // Ваше начальное время 00:00
    QTime endTime = QTime(23, 0); // Ваше конечное время 23:59:59
    QString error = isComputerNumberExists(computerNumberLineEdit->text(), date, startTime, endTime);
    if (!error.isEmpty()) {
    QMessageBox::critical(this, "Ошибка", "Компьютер уже занят на выбранную дату.");
    return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.prepare("INSERT INTO database_pk_club (login, computer_number, date) VALUES (:login, :computer_number, :date)");
    query.bindValue(":login", nameLineEdit->text());
    query.bindValue(":computer_number", computerNumberLineEdit->text());
    query.bindValue(":date", date); // Используйте выбранную дату

    if(!query.exec())
    {
        qDebug() << "Error inserting into the table.";
    } else {
        writeToFile("Логин: " + nameLineEdit->text() + ", Номер пк: " + computerNumberLineEdit->text() + ", Дата: " + date);
    }
}

void MainWindow::removeFromDatabase(const QString &login, const QString &date)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.prepare("DELETE FROM database_pk_club WHERE login = :login AND date = :date");
    query.bindValue(":login", login);
    query.bindValue(":date", date);


    if(!query.exec())
    {
        qDebug() << "Error deleting from the table.";
    }
}
QString MainWindow::isComputerNumberExists(const QString &computer_number, const QString &date, const QTime &startTime, const QTime &endTime)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        return "Error: connection with database failed";
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM database_pk_club WHERE computer_number = :computer_number AND date = :date AND ((start < :end AND end > :start) OR (start <= :end AND end >= :start))");
    query.bindValue(":computer_number", computer_number);
    query.bindValue(":date", date);
    query.bindValue(":start", startTime.toString("hh:mm:ss"));
    query.bindValue(":end", endTime.toString("hh:mm:ss"));

    if(query.exec())
    {
        while (query.next())
        {
            QTime existingStartTime = QTime::fromString(query.value(3).toString(), "hh:mm:ss");
            QTime existingEndTime = QTime::fromString(query.value(4).toString(), "hh:mm:ss");

            if ((startTime >= existingStartTime && startTime < existingEndTime) ||
                (endTime > existingStartTime && endTime <= existingEndTime)) {
                return "Error: Computer number already exists.";
            }
        }
    }
    return "";
}
void MainWindow::createReport()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM database_pk_club");

    if(query.exec())
    {
        QTableWidget* table = new QTableWidget;
        table->setWindowTitle("Отчет пк клуба");
        table->setColumnCount(6); // Установите количество столбцов в соответствии с вашей базой данных
        table->setHorizontalHeaderLabels(QStringList() << "Login" << "Computer Number" << "Time"  << "Start" << "End" << "Date"); // Установите заголовки столбцов в соответствии с вашей базой данных
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        int row = 0;
        while (query.next()) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
            table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
            table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
            table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
            row++;
        }

        // Включите сортировку
        table->setSortingEnabled(true);

        // Создание нового окна для отчета
        QWidget *reportWindow = new QWidget;
        reportWindow->setWindowTitle("Отчет пк клуба");

        // Создание вертикального менеджера компановки для окна отчета
        QVBoxLayout *reportLayout = new QVBoxLayout;
        reportLayout->addWidget(table);

        // Создание кнопки поиска
        QPushButton *btnSearch = new QPushButton("Поиск", this);
        connect(btnSearch, &QPushButton::clicked, this, &MainWindow::openSearchWindow);
        reportLayout->addWidget(btnSearch); // Добавьте эту кнопку в ваш менеджер компановки

        // Установка менеджера компановки для окна отчета
        reportWindow->setLayout(reportLayout);

        // Отображение окна отчета
        reportWindow->show();
        reportWindow->resize(800, 600); // Установите размер окна подходящим для вашего случая
    }
}
void MainWindow::openSearchWindow()
{
    bool ok;
    QStringList searchTypes;
    searchTypes << "Login" << "Computer Number" << "Date"; // Добавьте сюда типы поиска

    QString searchType = QInputDialog::getItem(this, "Поиск", "Выберите тип поиска:", searchTypes, 0, false, &ok);
    if (ok && !searchType.isEmpty()) {
        // Выполните запрос к базе данных, чтобы получить все уникальные значения для выбранного типа поиска
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("database_pk_club.db");

        if (!db.open())
        {
            qDebug() << "Error: connection with database failed";
        }
        else
        {
            qDebug() << "Database: connection ok";
        }

        QSqlQuery query;
        if (searchType == "Computer Number") {
            query.prepare("SELECT DISTINCT computer_number FROM database_pk_club");
            searchType = "computer_number";
        } else {
            query.prepare("SELECT DISTINCT " + searchType + " FROM database_pk_club");
        }

        QStringList searchKeys;
        if(query.exec())
        {
            if (query.size() == 0) {
                QMessageBox::information(this, "Поиск", "Нет результатов поиска.");
                return;
            }

            while (query.next()) {
                searchKeys << query.value(0).toString();
            }
        }

        QString searchKey = QInputDialog::getItem(this, "Поиск", "Выберите ключ поиска:", searchKeys, 0, false, &ok);
        if (ok && !searchKey.isEmpty()) {
            // Выполните поиск в базе данных
            query.prepare("SELECT * FROM database_pk_club WHERE " + searchType + " = :search");
            query.bindValue(":search", searchKey);

            if(query.exec())
            {
                QTableWidget* table = new QTableWidget;
                table->setWindowTitle("Результаты поиска");
                table->setColumnCount(6); // Установите количество столбцов в соответствии с вашей базой данных
                table->setHorizontalHeaderLabels(QStringList() << "Login" << "Computer Number" << "Time"  << "Start" << "End" << "Date"); // Установите заголовки столбцов в соответствии с вашей базой данных
                table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

                int row = 0;
                while (query.next()) {
                    table->insertRow(row);
                    table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
                    table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
                    table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
                    table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
                    table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
                    table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));
                    row++;
                }

                // Включите сортировку
                table->setSortingEnabled(true);

                // Создание нового окна для отчета
                QWidget *searchWindow = new QWidget;
                searchWindow->setWindowTitle("Результаты поиска");

                // Создание вертикального менеджера компановки для окна отчета
                QVBoxLayout *searchLayout = new QVBoxLayout;
                searchLayout->addWidget(table);

                // Установка менеджера компановки для окна отчета
                searchWindow->setLayout(searchLayout);

                // Отображение окна отчета
                searchWindow->show();
                searchWindow->resize(800, 600); // Установите размер окна подходящим для вашего случая
            }
        }
    }
}
void MainWindow::removeFromDatabaseWithTime(const QString &login, const QString &date, const QTime &startTime, const QTime &endTime)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database_pk_club.db");

    if (!db.open())
    {
        qDebug() << "Error: connection with database failed";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.prepare("DELETE FROM database_pk_club WHERE login = :login AND date = :date AND start = :start AND end = :end");
    query.bindValue(":login", login);
    query.bindValue(":date", date);
    query.bindValue(":start", startTime.toString("hh:mm:ss"));
    query.bindValue(":end", endTime.toString("hh:mm:ss"));

    if(!query.exec())
    {
        qDebug() << "Error deleting from the table.";
    }
}
