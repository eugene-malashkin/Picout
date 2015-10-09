#include "MainWindow.h"
#include <QPainter>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

/****************************************************************************************************************
 * ImageWidget
 ***************************************************************************************************************/

ImageWidget::ImageWidget(QWidget *parent)
  : QWidget(parent)
  , d_image()
{

}

void ImageWidget::setImage(const QImage &value)
{
  d_image = value;
  update();
}

QImage ImageWidget::image() const
{
  return d_image;
}

void ImageWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.drawImage(0, 0, d_image);
}


/****************************************************************************************************************
 * MainWindow
 ***************************************************************************************************************/

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , d_imageWidget(nullptr)
  , d_resultingImage()
  , d_lastOpenedFileName()
{
  resize(800, 600);

  d_imageWidget = new ImageWidget;
  setCentralWidget(d_imageWidget);

  auto fileMenu = menuBar()->addMenu(tr("File"));
  fileMenu->addAction(tr("Open..."), this, SLOT(onOpen()), QKeySequence("Ctrl+O"));
  fileMenu->addAction(tr("Refresh"), this, SLOT(onRefresh()), QKeySequence("F5"));
  fileMenu->addAction(tr("Save"), this, SLOT(onSave()), QKeySequence("Ctrl+S"));
}

void MainWindow::onOpen()
{
  auto fileName = QFileDialog::getOpenFileName(this, tr("Open XML file"), initialOpenDir(), "XML Files (*.xml))");
  if (fileName.isEmpty())
  {
    return;
  }
  d_lastOpenedFileName = fileName;
  auto s = std::move(settings());
  s->setValue("LastOpenedFileName", d_lastOpenedFileName);
  open(fileName);
}

void MainWindow::onRefresh()
{
  if (d_lastOpenedFileName.isEmpty())
  {
    QMessageBox::critical(this, tr("Error"), tr("Last opened file name is unknown"));
    return;
  }
  open(d_lastOpenedFileName);
}

void MainWindow::onSave()
{
  if (d_resultingImage.isNull())
  {
    QMessageBox::critical(this, tr("Error"), tr("No image to save"));
    return;
  }
  if (d_lastOpenedFileName.isEmpty())
  {
    QMessageBox::critical(this, tr("Error"), tr("Last opened file name is unknown"));
    return;
  }

  QFileInfo fileInfo(d_lastOpenedFileName);
  auto fileName = QString("%1.png").arg(fileInfo.baseName());
  auto absoluteFilePath = fileInfo.dir().absoluteFilePath(fileName);

  d_resultingImage.save(absoluteFilePath);
}

void MainWindow::open(const QString &fileName)
{
  Input input;
  QString errorString;
  std::tie(input, errorString) = Input::readFromFile(fileName);
  if (!errorString.isEmpty())
  {
    QMessageBox::critical(this, tr("Error"), errorString);
    return;
  }

  d_resultingImage = input.resultingImage();
  d_imageWidget->setImage(d_resultingImage);
}

std::unique_ptr<QSettings> MainWindow::settings() const
{
  return std::make_unique<QSettings>("Align Technology", "Picout");
}

QString MainWindow::initialOpenDir() const
{
  auto s = std::move(settings());
  auto lastOpenedFileName = s->value("LastOpenedFileName").toString();
  return QFileInfo(lastOpenedFileName).absolutePath();
}
