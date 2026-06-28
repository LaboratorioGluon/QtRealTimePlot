#ifndef RTPCURSOR_H__
#define RTPCURSOR_H__

#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QWheelEvent>

#include "PlotSeries.h"

class RtpCursor {
 public:
  enum MarkerStyle { MARKER_FULL = 0, MARKER_SIMPLE };

  RtpCursor(int index, QColor color,
            MarkerStyle style = MarkerStyle::MARKER_FULL);

  void setSeriesSource(const std::vector<std::shared_ptr<PlotSeries>>* series) {
    m_seriesRef = series;
  };

  void setFont(const QFont& font);
  void setTextColor(const QColor& color);
  void setBackgroundColor(const QColor& bgColor);
  void setMargin(const int marginPixels);

  void draw(QPainter& painter, const QRect& plotArea, int calculatedPixelX,
            const QString& mainLabel, bool alignLeft = true);

  /***** INPUT Handling *****/
  bool mousePressEvent(QMouseEvent* event, int currentPixelX);
  bool mouseMoveEvent(QMouseEvent* event, const QRect& plotArea,
                      int& outNewPixelX);
  void mouseReleaseEvent(QMouseEvent* event);

  QRect lastRenderedRect() const { return m_lastRect; }

  void   setPos(double pos) { m_xPos = pos; }
  double getPos() const { return m_xPos; }

  bool enabled() const { return m_isEnabled; }
  void enable(bool e) { m_isEnabled = e; }

  bool isVisible() { return m_isVisible; }

  bool isDragging() const { return m_isDragging; }

  bool contains(const QPoint& pos) const;

 private:
  bool   m_isEnabled = true;
  double m_xPos      = 0;

  bool m_isDragging = false;
  int  m_dragOffset = 0;

  int    m_cursorIndex;
  bool   m_isVisible = false;
  QFont  m_font;
  QColor m_color;
  QColor m_textColor;
  QColor m_bgColor;
  QRect  m_lastRect;

  QRect m_labelRect;
  QRect m_lineHitbox;

  MarkerStyle m_style;

  const std::vector<std::shared_ptr<PlotSeries>>* m_seriesRef = nullptr;

  void drawFullMarker(QPainter& painter, const QRect& plotArea,
                      int calculatedPixelX, bool alignLeft);
  void drawSimpleMarker(QPainter& painter, const QRect& plotArea,
                        int calculatedPixelX, bool alignLeft);
};

#endif  // RTPCURSOR_H__