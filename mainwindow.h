#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextEdit>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void fetchNews();
    void handleNetworkReply(QNetworkReply *reply);
    void chooseColor();
    void onHorizontalScrollRangeChanged(int min, int max);
    void onListItemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;  // <--- добавляем менеджер сети
    void applyThemeColor(const QColor& color);
    void loadFavorites();
    void saveFavorites();
    void onAddFavorite();
    void addFavoriteItem(const QString &keyword);
    int originalListHeight;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // MAINWINDOW_H
