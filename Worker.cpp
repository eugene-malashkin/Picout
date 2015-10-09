#include "Worker.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QPainter>

static const int g_spacing(16);

std::tuple<Input,QString> Input::readFromFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    return std::make_tuple(Input(), qApp->translate("Input", "Cannot read XML file"));
  }
  QDomDocument document;
  if (!document.setContent(file.readAll()))
  {
    return std::make_tuple(Input(), qApp->translate("Input", "Cannot read XML content in the file"));
  }
  Input input;
  auto root = document.documentElement();
  auto frameRectElement = root.firstChildElement("frameRect");
  input.frameRect.setLeft(frameRectElement.attribute("left").toInt());
  input.frameRect.setTop(frameRectElement.attribute("top").toInt());
  input.frameRect.setWidth(frameRectElement.attribute("width").toInt());
  input.frameRect.setHeight(frameRectElement.attribute("height").toInt());
  auto dir = QFileInfo(fileName).absoluteDir();
  auto picsElement = root.firstChildElement("pics");
  for (auto picElement = picsElement.firstChildElement("pic"); !picElement.isNull(); picElement = picElement.nextSiblingElement("pic"))
  {
    auto fileName = picElement.text();
    auto picAbsoluteFileName = dir.absoluteFilePath(fileName);
    auto clickType = ClickType::None;
    auto clickTypeStr = picElement.attribute("type");
    if (clickTypeStr == "left")  clickType = ClickType::Left;
    if (clickTypeStr == "right") clickType = ClickType::Right;
    auto clickPoint = QPoint(picElement.attribute("clickX").toInt(), picElement.attribute("clickY").toInt());
    SourceItem item {QImage(picAbsoluteFileName), clickType, clickPoint};
    input.sources << item;
  }

  auto errorString = input.errorString();
  if (!errorString.isEmpty())
  {
    return std::make_tuple(input, errorString);
  }
  return std::make_tuple(input, QString());
}

QString Input::errorString() const
{
  if (sources.isEmpty())
  {
    return qApp->translate("Input", "There is no source images");
  }
  auto sourceImageSize = sources.first().image.size();
  for (const auto &item : sources)
  {
    auto sourceImage = item.image;
    if (sourceImage.size() != sourceImageSize)
    {
      return qApp->translate("Input", "Source image size mismatch");
    }
  }
  if (sourceImageSize.isEmpty())
  {
    return qApp->translate("Input", "Source image size is empty");
  }
  auto realFrameRect = frameRect.intersected(QRect(0, 0, sourceImageSize.width(), sourceImageSize.height()));
  if (realFrameRect.isEmpty())
  {
    return qApp->translate("Input", "Real frame rect is empty");
  }
  return QString();
}

void paintFrame(QPainter &painter, const QRect &rect)
{
  painter.setBrush(Qt::NoBrush);
  painter.setPen(Qt::black);
  painter.drawRect(rect);
  painter.setPen(Qt::white);
  painter.drawRect(rect.adjusted(1, 1, -1, -1));
  painter.drawRect(rect.adjusted(2, 2, -2, -2));
}

void paintClickPoint(QPainter &painter, const QRect &rect, ClickType clickType, const QPoint &clickPoint)
{
  const QPoint corrective(5, 5);
  if (clickType == ClickType::Left)
  {
    static const QImage icon(":/LeftClick.png");
    painter.drawImage(clickPoint + rect.topLeft() - corrective, icon);
  }
  else if (clickType == ClickType::Right)
  {
    static const QImage icon(":/RightClick.png");
    painter.drawImage(clickPoint + rect.topLeft() - corrective, icon);
  }
}

QImage Input::resultingImage() const
{
  if (!errorString().isEmpty())
  {
    return QImage();
  }

  auto sourceSize = sources.first().image.size();
  auto realFrameRect = frameRect.intersected(QRect(0, 0, sourceSize.width(), sourceSize.height()));
  auto resultingSize = QSize(
        std::max(sourceSize.width(), realFrameRect.width()*sources.count() + g_spacing*(sources.count()-1)) + 1,
        sourceSize.height() + g_spacing + realFrameRect.height() + 1
        );
  QImage result(resultingSize, QImage::Format_ARGB32);
  result.fill(0);

  QPainter painter(&result);
  auto fullImageX = (resultingSize.width() - sourceSize.width())/2;
  painter.drawImage(fullImageX, 0, sources.first().image);

  QPoint currentPoint(0, sourceSize.height() + g_spacing);
  for (const auto &item : sources)
  {
    QRect targetRect(currentPoint, realFrameRect.size());
    painter.drawImage(targetRect, item.image, realFrameRect);
    paintClickPoint(painter, targetRect, item.clickType, item.clickPoint);
    paintFrame(painter, targetRect);
    currentPoint.setX(currentPoint.x() + g_spacing + realFrameRect.width());
  }

  paintFrame(painter, realFrameRect.adjusted(fullImageX, 0, fullImageX, 0));


  return result;
}
