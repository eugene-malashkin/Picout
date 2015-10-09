#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QSettings>
#include "Worker.h"

class ImageWidget : public QWidget
{
public:
  ImageWidget(QWidget *parent = nullptr);
  void setImage(const QImage &value);
  QImage image() const;

protected:
  virtual void paintEvent(QPaintEvent *) override;

private:
  QImage d_image;
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);

private slots:
  void onOpen();
  void onRefresh();
  void onSave();

private:
  ImageWidget *d_imageWidget;
  QImage d_resultingImage;
  QString d_lastOpenedFileName;
  void open(const QString &fileName);
  std::unique_ptr<QSettings> settings() const;
  QString initialOpenDir() const;
};

#endif // MAINWINDOW_H
