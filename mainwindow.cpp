#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QListWidget>
#include <QColor> // Include QColor for color manipulation

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), totalIncome(0), totalExpenses(0), totalBudget(0)
{
    setWindowTitle("Budget Planner");
    resize(800, 500);

    // Initialize widgets
    transactionListWidget = new QListWidget(this);
    totalBudgetLabel = new QLabel("Total Budget: $0.00", this);

    // Initialize UI elements
    amountLineEdit = new QLineEdit(this);
    categoryComboBox = new QComboBox(this);
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    addTransactionButton = new QPushButton("Add Transaction", this);
    searchLineEdit = new QLineEdit(this);
    filterButton = new QPushButton("Filter", this);
    startDateEdit = new QDateEdit(QDate::currentDate(), this);
    endDateEdit = new QDateEdit(QDate::currentDate(), this);
    clearButton = new QPushButton("Clear", this);
    manageCategoriesButton = new QPushButton("Manage Categories", this);
    setBudgetGoalButton = new QPushButton("Set Budget Goal", this);

    setupUI();
    setupStatusBar();
    loadData();
    loadCategories();
}

MainWindow::~MainWindow()
{
    saveData();
    saveCategories();
}

QString MainWindow::getLineEditStyle()
{
    return R"(
        QLineEdit {
            padding: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 2px solid #4CAF50;
        }
    )";
}

QString MainWindow::getComboBoxStyle()
{
    return R"(
        QComboBox {
            padding: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-size: 14px;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #ccc;
        }
    )";
}

QString MainWindow::getDateEditStyle()
{
    return R"(
        QDateEdit {
            padding: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-size: 14px;
        }
    )";
}

QString MainWindow::getLabelStyle()
{
    return R"(
        QLabel {
            font-size: 14px;
            font-weight: bold;
            color: #333;
        }
    )";
}

QString MainWindow::getButtonStyle(const QString &color)
{
    QColor qColor(color); // Convert QString to QColor
    return QString(R"(
        QPushButton {
            background-color: %1;
            border: none;
            color: white;
            padding: 10px 20px;
            text-align: center;
            text-decoration: none;
            font-size: 14px;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %3;
        }
    )").arg(color) // Original color
        .arg(qColor.lighter(110).name()) // Lighter color
        .arg(qColor.darker(110).name()); // Darker color
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Add predefined categories
    QStringList predefinedCategories = {
        "Salary", "Rent", "Food", "Transport", "Entertainment", "Utilities", "Healthcare", "Other"
    };
    categoryComboBox->addItems(predefinedCategories);

    // Apply styles to input fields
    amountLineEdit->setStyleSheet(getLineEditStyle());
    searchLineEdit->setStyleSheet(getLineEditStyle());
    categoryComboBox->setStyleSheet(getComboBoxStyle());
    dateEdit->setStyleSheet(getDateEditStyle());
    startDateEdit->setStyleSheet(getDateEditStyle());
    endDateEdit->setStyleSheet(getDateEditStyle());

    // Apply styles to buttons
    addTransactionButton->setStyleSheet(getButtonStyle("#4CAF50")); // Green
    filterButton->setStyleSheet(getButtonStyle("#2196F3")); // Blue
    clearButton->setStyleSheet(getButtonStyle("#F44336")); // Red
    manageCategoriesButton->setStyleSheet(getButtonStyle("#FF9800")); // Orange
    setBudgetGoalButton->setStyleSheet(getButtonStyle("#9C27B0")); // Purple

    // Apply styles to labels
    QList<QLabel*> labels = centralWidget->findChildren<QLabel*>();
    for (QLabel *label : labels) {
        label->setStyleSheet(getLabelStyle());
    }

    // Layout for input fields
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Amount:"));
    inputLayout->addWidget(amountLineEdit);
    inputLayout->addWidget(new QLabel("Category:"));
    inputLayout->addWidget(categoryComboBox);
    inputLayout->addWidget(new QLabel("Date:"));
    inputLayout->addWidget(dateEdit);
    inputLayout->addWidget(addTransactionButton);

    // Layout for filter fields
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Search:"));
    filterLayout->addWidget(searchLineEdit);
    filterLayout->addWidget(new QLabel("Start Date:"));
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(new QLabel("End Date:"));
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(filterButton);
    filterLayout->addWidget(clearButton);

    // Layout for additional buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(manageCategoriesButton);
    buttonLayout->addWidget(setBudgetGoalButton);

    // Add widgets to the main layout
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(filterLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(transactionListWidget);
    mainLayout->addWidget(totalBudgetLabel);

    setCentralWidget(centralWidget);

    // Connect signals and slots
    connect(addTransactionButton, &QPushButton::clicked, this, &MainWindow::onAddTransactionClicked);
    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(filterButton, &QPushButton::clicked, this, &MainWindow::onFilterButtonClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearButtonClicked);
    connect(manageCategoriesButton, &QPushButton::clicked, this, &MainWindow::onManageCategoriesClicked);
    connect(setBudgetGoalButton, &QPushButton::clicked, this, &MainWindow::onSetBudgetGoalClicked);
}

void MainWindow::setupStatusBar()
{
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
}

void MainWindow::onAddTransactionClicked()
{
    bool ok;
    double amount = amountLineEdit->text().toDouble(&ok);
    if (ok && amount > 0) {
        QString category = categoryComboBox->currentText();
        QString date = dateEdit->date().toString("yyyy-MM-dd");

        if (category == "Salary") {
            totalIncome += amount;
            transactionListWidget->addItem(QString("[%1] Income (%2): +$%3").arg(date, category).arg(amount));
        } else {
            totalExpenses += amount;
            transactionListWidget->addItem(QString("[%1] Expense (%2): -$%3").arg(date, category).arg(amount));
        }

        calculateBudget();
        statusBar->showMessage("Transaction added successfully", 3000);
    } else {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid amount.");
    }
    amountLineEdit->clear();
}

void MainWindow::calculateBudget()
{
    totalBudget = totalIncome - totalExpenses;
    totalBudgetLabel->setText(QString("Total Budget: $%1").arg(totalBudget));
}

void MainWindow::saveData()
{
    QFile file("transactions.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < transactionListWidget->count(); ++i) {
            out << transactionListWidget->item(i)->text() << "\n";
        }
        file.close();
    }
}

void MainWindow::loadData()
{
    QFile file("transactions.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            transactionListWidget->addItem(line);

            // Parse the line to update totals
            if (line.contains("Income")) {
                totalIncome += line.section(':', -1).toDouble();
            } else if (line.contains("Expense")) {
                totalExpenses += line.section(':', -1).toDouble();
            }
        }
        file.close();
    }
    calculateBudget();
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text); // Suppress unused parameter warning
    filterTransactions();
}

void MainWindow::onFilterButtonClicked()
{
    filterTransactions();
}

void MainWindow::filterTransactions()
{
    QString searchText = searchLineEdit->text().toLower();
    QDate startDate = startDateEdit->date();
    QDate endDate = endDateEdit->date();

    for (int i = 0; i < transactionListWidget->count(); ++i) {
        QListWidgetItem *item = transactionListWidget->item(i);
        QString itemText = item->text().toLower();
        QDate itemDate = QDate::fromString(itemText.section(']', 0, 0).mid(1), "yyyy-MM-dd");

        bool matchesSearch = itemText.contains(searchText);
        bool matchesDate = (itemDate >= startDate && itemDate <= endDate);

        item->setHidden(!(matchesSearch && matchesDate));
    }
}

void MainWindow::onManageCategoriesClicked()
{
    bool ok;
    QString newCategory = QInputDialog::getText(this, "Manage Categories", "Enter a new category:", QLineEdit::Normal, "", &ok);
    if (ok && !newCategory.isEmpty()) {
        categoryComboBox->addItem(newCategory);
        saveCategories();
    }
}

void MainWindow::onSetBudgetGoalClicked()
{
    QString category = categoryComboBox->currentText();
    bool ok;
    double goal = QInputDialog::getDouble(this, "Set Budget Goal", QString("Enter budget goal for %1:").arg(category), 0, 0, 1000000, 2, &ok);
    if (ok) {
        budgetGoals[category] = goal;
        statusBar->showMessage(QString("Budget goal set for %1: $%2").arg(category).arg(goal), 3000);
    }
}

void MainWindow::onClearButtonClicked()
{
    transactionListWidget->clear();

    totalIncome = 0;
    totalExpenses = 0;
    totalBudget = 0;

    totalBudgetLabel->setText("Total Budget: $0.00");

    statusBar->showMessage("Transaction list cleared", 3000);
}

void MainWindow::loadCategories()
{
    QFile file("categories.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString category = in.readLine();
            categoryComboBox->addItem(category);
        }
        file.close();
    }
}

void MainWindow::saveCategories()
{
    QFile file("categories.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < categoryComboBox->count(); ++i) {
            out << categoryComboBox->itemText(i) << "\n";
        }
        file.close();
    }
}
