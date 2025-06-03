#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QSslSocket>
#include <QColorDialog>
#include <QSettings>
#include <QFile>
#include <QDesktopServices>
#include <QScrollBar>
#include <QUrlQuery>

QString country_input = "";
QString language_input = "en";
QString keywords_input = "";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    loadFavorites();

    originalListHeight = ui->listWidget->height();

    connect(ui->listWidget->horizontalScrollBar(), &QScrollBar::rangeChanged,
            this, &MainWindow::onHorizontalScrollRangeChanged);
    connect(ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::onListItemClicked);
    connect(ui->choose_color_btn, &QPushButton::clicked, this, &MainWindow::chooseColor);
    connect(ui->add_fav_label, &QLabel::linkActivated, this, &MainWindow::onAddFavorite);

    ui->add_fav_label->setText("<a href='#' style='color: gray; text-decoration: none;'>Add to Favorites</a>");
    ui->listWidget->setFocusPolicy(Qt::NoFocus);

    QSettings settings;

    QColor savedColor = settings.value("themeColor", QColor(233, 94, 130)).value<QColor>();
    applyThemeColor(savedColor);

    ui->yaboku_label_link->installEventFilter(this);
    ui->listWidget->setFlow(QListView::LeftToRight);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    ui->country_combo_box->addItem("All world", "");
    ui->country_combo_box->addItem("Ukraine", "ua");
    ui->country_combo_box->addItem("United States", "us");
    ui->country_combo_box->addItem("Europe", "eu");
    ui->country_combo_box->addItem("United Kingdom", "gb");

    ui->language_combo_box->addItem("English", "en");
    ui->language_combo_box->addItem("German", "de");
    ui->language_combo_box->addItem("French", "fr");
    ui->language_combo_box->addItem("Spanish", "es");
    ui->language_combo_box->addItem("Chinese", "zh");

    country_input = ui->country_combo_box->currentData().toString();
    language_input = ui->language_combo_box->currentData().toString();

    ui->country_combo_box->setCurrentIndex(0); //set default item

    connect(ui->filter_apply_btn, &QPushButton::clicked, this, &MainWindow::fetchNews);

    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::handleNetworkReply);
    ui->textBrowser->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->textBrowser->setOpenExternalLinks(true);

    //fetchNews(); // load news on startup
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fetchNews() {
    QString apiKey = "577d2a75c0bc583993fe659cf5500666";

    country_input = ui->country_combo_box->currentData().toString();
    language_input = ui->language_combo_box->currentData().toString();
    keywords_input = ui->keywords_line_edit->text().trimmed();

    QUrl url("http://api.mediastack.com/v1/news");
    QUrlQuery query;
    query.addQueryItem("access_key", apiKey);

    if (!country_input.isEmpty()) {
        query.addQueryItem("country", country_input);
    }
    if (!language_input.isEmpty()) {
        query.addQueryItem("languages", language_input);
    }
    if (!keywords_input.isEmpty()) {
        query.addQueryItem("keywords", keywords_input);
    }

    query.addQueryItem("sort", "published_desc");

    url.setQuery(query);

    QNetworkRequest request(url);
    networkManager->get(request);
}

void MainWindow::handleNetworkReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
    if (jsonDoc.isObject()) {
        QJsonObject obj = jsonDoc.object();
        QJsonArray data = obj["data"].toArray();;

        QSet<QString> seenTitles;
        QString newsHtml;

        for (int i = 0; i < data.size(); ++i) {
            QJsonObject newsItem = data.at(i).toObject();
            QString title = newsItem["title"].toString();

            if (seenTitles.contains(title)) {
                continue;
            }
            seenTitles.insert(title);

            QString description = newsItem["description"].toString();
            QString url = newsItem["url"].toString();

            newsHtml += "<b style='font-size:16pt; color: gray;'>" + title + "</b>"; // title
            newsHtml += "<br><br>";

            newsHtml += "<span style='font-size:10pt;'>" + description + "</span>"; // main text
            newsHtml += "<br>";

            if (!url.isEmpty()) { // link
                newsHtml += "<a href='" + url + "' style='color:gray; font-size:10pt;text-decoration: none;'>Read more...</a>";
            }

            newsHtml += "<br><hr>";
        }
        ui->textBrowser->setHtml(newsHtml);
        ui->textBrowser->setOpenExternalLinks(true);
    }

    reply->deleteLater();
}


void MainWindow::applyThemeColor(const QColor& color) {
    QString colorName = color.name();

    QFile file(":/styles/styles/color_override.qss");
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "color_override error";
        return;
    }

    QTextStream stream(&file);
    QString colorStyle = stream.readAll();
    file.close();

    QString finalStyle = colorStyle.arg(colorName);

    QString combinedStyle = ui->centralwidget->styleSheet() + finalStyle; //set color overlay over Designer styles
    ui->centralwidget->setStyleSheet(combinedStyle);

}


void MainWindow::chooseColor() {
    QColor chosenColor = QColorDialog::getColor(Qt::white, this, "Choose Theme Color");
    if (chosenColor.isValid()) {
        QSettings settings("MyCompany", "MyApp");
        settings.setValue("themeColor", chosenColor);
        applyThemeColor(chosenColor);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->yaboku_label_link && event->type() == QEvent::MouseButtonPress) {
        QDesktopServices::openUrl(QUrl("https://github.com/Yabokua"));
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::saveFavorites() {
    QSettings settings("MyCompany", "MyApp");
    QStringList favorites;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        QWidget *itemWidget = ui->listWidget->itemWidget(item);
        if (itemWidget) {
            QLabel *label = itemWidget->findChild<QLabel *>();
            if (label) {
                favorites << label->text();
            }
        }
    }
    settings.setValue("favorites", favorites);
}


void MainWindow::onAddFavorite() {
    QString keyword = ui->keywords_line_edit->text().trimmed();
    if (keyword.isEmpty()) {
        return;
    }

    bool alreadyExists = false;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        if (ui->listWidget->item(i)->text() == keyword) {
            alreadyExists = true;
            break;
        }
    }
    if (alreadyExists) {
        return;
    }

    addFavoriteItem(keyword); // add item
    saveFavorites();
}

void MainWindow::loadFavorites() {
    QSettings settings("MyCompany", "MyApp");
    QStringList favorites = settings.value("favorites").toStringList();
    for (int i = 0; i < favorites.size(); ++i) {
        addFavoriteItem(favorites.at(i));
    }
}

void MainWindow::addFavoriteItem(const QString &keyword) {
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    QWidget *itemWidget = new QWidget();

    QLabel *label = new QLabel(keyword);
    QPushButton *removeButton = new QPushButton("x");
    removeButton->setObjectName("remove_button"); // add remove btn

    label->setStyleSheet("padding: 3px; padding-right: 0px; font-weight: bold;"); // favorites item style
    removeButton->setStyleSheet("background-color: transparent; border: none; padding: 0px 4px; margin-top: -2px;");

    removeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(itemWidget); // Favorites item layout
    layout->addWidget(label);
    layout->addWidget(removeButton);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->setAlignment(removeButton, Qt::AlignLeft);
    layout->setAlignment(label, Qt::AlignVCenter);

    itemWidget->setLayout(layout);

    int height = 24;
    QFontMetrics fm(label->font()); // Favorites item width with text
    int textWidth = fm.horizontalAdvance(keyword);
    int btnWidth = removeButton->sizeHint().width(); // remove btn width
    int padding = 6;
    int spacing = layout->spacing();// add extra space
    int totalWidth = textWidth + btnWidth + padding + spacing + 15;

    item->setSizeHint(QSize(totalWidth, height));

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, itemWidget);

    connect(removeButton, &QPushButton::clicked, this, [=]() {
        delete ui->listWidget->takeItem(ui->listWidget->row(item));
        saveFavorites();
    });
}

void MainWindow::onHorizontalScrollRangeChanged(int min, int max) { // adaptive scrolllist size
    if (max > 0) {
        int sbHeight = ui->listWidget->horizontalScrollBar()->height();
        ui->listWidget->setFixedHeight(originalListHeight + sbHeight);
    } else {
        ui->listWidget->setFixedHeight(originalListHeight);
    }
}

void MainWindow::onListItemClicked(QListWidgetItem *item) {
    if (!item) return;

    QWidget *widget = ui->listWidget->itemWidget(item);
    if (!widget) return;

    QLabel *label = widget->findChild<QLabel *>();
    if (label) {
        ui->keywords_line_edit->setText(label->text());
    }

    fetchNews(); //load new by clicked on "favorite item"
    ui->keywords_line_edit->setText("");
}
