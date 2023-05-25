#pragma once

#include "sme/voxel.hpp"
#include <QImage>

namespace sme::common {

/**
 * @brief Stack of 2d QImages
 *
 * A 3d image stored as a stack of z-slices, one QImage per slice.
 */
class ImageStack {
private:
  std::vector<QImage> imgs{};
  Volume sz{0, 0, 0};

public:
  ImageStack();
  ImageStack(const Volume &size, QImage::Format format);
  explicit ImageStack(std::vector<QImage> &&images);
  explicit ImageStack(const std::vector<QImage> &images);
  /**
   * @brief Grayscale image from array of pixel intensities
   *
   * If a positive value is supplied for `maxValue` it determines the value that
   * corresponds to white in the image, otherwise by default the maximum value
   * from `values` is used.
   */
  ImageStack(const Volume &imageSize, const std::vector<double> &values,
             double maxValue = -1.0);
  // todo: remove this constructor or make it explicit
  // temporarily allow a QImage to implicitly be converted to a ImageStack
  // to avoid refactoring a lot of test code
  ImageStack(const QImage &image);
  inline QImage &operator[](std::size_t z) { return imgs[z]; }
  [[nodiscard]] inline const QImage &operator[](std::size_t z) const {
    return imgs[z];
  }
  [[nodiscard]] inline bool empty() const { return imgs.empty(); }
  [[nodiscard]] inline const Volume &volume() const noexcept { return sz; }
  inline std::vector<QImage>::iterator begin() noexcept { return imgs.begin(); }
  [[nodiscard]] inline std::vector<QImage>::const_iterator
  cbegin() const noexcept {
    return imgs.cbegin();
  }
  inline std::vector<QImage>::iterator end() noexcept { return imgs.end(); }
  [[nodiscard]] inline std::vector<QImage>::const_iterator
  cend() const noexcept {
    return imgs.cend();
  }
  void clear();
  void fill(uint pixel);
  void rescaleXY(QSize size);
  void flipYAxis();
  void convertToIndexed();
  ImageStack scaledToWidth(int width);
  ImageStack scaledToHeight(int height);
};

} // namespace sme::common
