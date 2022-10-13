#include "guiutils.hpp"
#include "sme/logger.hpp"
#include "sme/tiff.hpp"
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

void selectMatchingOrFirstItem(QListWidget *list, const QString &text) {
  if (list->count() == 0) {
    return;
  }
  if (auto l = list->findItems(text, Qt::MatchExactly); !l.isEmpty()) {
    list->setCurrentItem(l[0]);
  } else {
    list->setCurrentRow(0);
  }
}

void selectFirstChild(QTreeWidget *tree) {
  for (int i = 0; i < tree->topLevelItemCount(); ++i) {
    if (const auto *p = tree->topLevelItem(i);
        (p != nullptr) && (p->childCount() > 0)) {
      tree->setCurrentItem(p->child(0));
      return;
    }
  }
}

void selectMatchingOrFirstChild(QTreeWidget *list, const QString &text) {
  // look for a child widget with the desired text
  if (!text.isEmpty()) {
    for (auto *item :
         list->findItems(text, Qt::MatchExactly | Qt::MatchRecursive, 0)) {
      if (item->parent() != nullptr) {
        list->setCurrentItem(item);
        return;
      }
    }
  }
  // if we didn't find a match, then just select the first child item
  selectFirstChild(list);
}

sme::common::ImageStack getImageFromUser(QWidget *parent,
                                         const QString &title) {
  QString filename = QFileDialog::getOpenFileName(
      parent, title, "",
      "Image Files (*.tif *.tiff *.gif *.jpg *.jpeg *.png *.bmp);; All files "
      "(*.*)");
  if (filename.isEmpty()) {
    return {};
  }
  SPDLOG_DEBUG("  - import file {}", filename.toStdString());
  // first try using tiffReader
  sme::common::TiffReader tiffReader(filename.toStdString());
  if (tiffReader.size() > 0) {
    return tiffReader.getImages();
  } else {
    SPDLOG_DEBUG(
        "    -> tiffReader could not read file, trying QImage::load()");
    QImage img;
    bool success = img.load(filename);
    if (success) {
      return sme::common::ImageStack({img});
    }
  }
  SPDLOG_DEBUG("    -> QImage::load() could not read file - giving up");
  QMessageBox::warning(
      parent, "Could not open image file",
      QString("Failed to open image file '%1'\nError message: '%2'")
          .arg(filename)
          .arg(tiffReader.getErrorMessage()));
  return {};
}

static QSize getZoomedInSize(const QSize &originalSize, int zoomFactor) {
  QSize newSize{originalSize};
  for (int i = 0; i < zoomFactor; ++i) {
    // 20% increase in size for each step
    newSize *= 6;
    newSize /= 5;
  }
  return newSize;
}

static void adjustScrollbars(QScrollArea *scrollArea, const QSizeF &sizeChange,
                             const QPointF &relativePos) {
  auto *hBar{scrollArea->horizontalScrollBar()};
  int hOffset{hBar->value() +
              static_cast<int>(relativePos.x() * sizeChange.width())};
  hBar->setValue(std::clamp(hOffset, 0, hBar->maximum()));
  auto *vBar{scrollArea->verticalScrollBar()};
  int vOffset{vBar->value() +
              static_cast<int>(relativePos.y() * sizeChange.height())};
  vBar->setValue(std::clamp(vOffset, 0, vBar->maximum()));
}

void zoomScrollArea(QScrollArea *scrollArea, int zoomFactor,
                    const QPointF &relativePos) {
  auto sizeOld{scrollArea->widget()->size()};
  scrollArea->setWidgetResizable(false);
  auto sizeNew{getZoomedInSize(scrollArea->size(), zoomFactor)};
  scrollArea->widget()->resize(sizeNew);
  QSizeF sizeChange(sizeNew - sizeOld);
  adjustScrollbars(scrollArea, sizeChange, relativePos);
}
