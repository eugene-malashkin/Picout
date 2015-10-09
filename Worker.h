#ifndef WORKER_H
#define WORKER_H

#include <tuple>
#include <QStringList>
#include <QImage>
#include <QRect>

enum class ClickType
{
  None,
  Left,
  Right
};

struct SourceItem
{
  QImage image;
  ClickType clickType;
  QPoint clickPoint;
  SourceItem() = default;
};

struct Input
{
  QList<SourceItem> sources;
  QRect frameRect;

  Input() = default;
  static std::tuple<Input,QString> readFromFile(const QString &fileName);
  QString errorString() const;

  QImage resultingImage() const;
};

#endif // WORKER_H
