/*
  Diana - A Free Meteorological Visualisation Tool

  $Id$

  Copyright (C) 2013 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <GL/gl.h>
#include "drawingpolyline.h"
#include "editpolyline.h"
#include <QDomDocument>
#include <QFileDialog>
#include <diTesselation.h>
#include <diDrawingManager.h>
#include <EditItems/drawingstylemanager.h>

namespace EditItem_PolyLine {

PolyLine::PolyLine()
  : addPoint_act_(new QAction(tr("Add point"), this))
  , removePoint_act_(new QAction(tr("Remove point"), this))
  , hitPointIndex_(-1)
  , hitLineIndex_(-1)
{
    init();
    updateControlPoints();
    color_.setRed(0);
    color_.setGreen(0);
    color_.setBlue(0);

    QObject::connect(addPoint_act_, SIGNAL(triggered()), SLOT(addPoint()));
    QObject::connect(removePoint_act_, SIGNAL(triggered()), SLOT(removePoint()));
}

PolyLine::~PolyLine()
{
}

DrawingItemBase *PolyLine::cloneSpecial() const
{
  PolyLine *item = new PolyLine;
  copyBaseData(item);
  return item;
}

bool PolyLine::hit(const QPointF &pos, bool selected) const
{
    // Have we hit a control point?
    if (selected && (hitControlPoint(pos) >= 0))
      return true;

    // Have we hit the edge?
    if (points_.size() >= 2 && hitLine(pos) != -1)
        return true;

    const QVariantMap style = DrawingStyleManager::instance()->getStyle(this);

    bool closed = style.value("closed").toBool();
    if (closed) {
      if (style.value("linesmooth").toBool()) {
          const QPainterPath path = DrawingStyleManager::interpolateToPath(points_, closed);
          if (path.contains(pos))
              return true;
      } else {
          const QPolygonF polygon(points_.toVector());
          if (polygon.containsPoint(pos, Qt::OddEvenFill))
              return true;
      }
    }
    return false;
}

bool PolyLine::hit(const QRectF &rect) const
{
    Q_UNUSED(rect);
    return false; // for now
}

/**
 * Returns the index of the line close to the position specified, or -1 if no
 * line was close enough.
 */
int PolyLine::hitLine(const QPointF &position) const
{
    if (points_.size() < 2)
        return -1;

    int minIndex = 0;
    const qreal proximityTolerance = 3.0;

    const QVariantMap style = DrawingStyleManager::instance()->getStyle(this);

    if (style.value("linesmooth").toBool()) {

      bool closed = style.value("closed").toBool();
      const QPainterPath path = DrawingStyleManager::interpolateToPath(points_, closed);

      // Examine each element in the path in turn, checking for an intersection
      // between a rectangle around the given position and the element. Note that
      // this approach relies on the order of the elements for each curve to match
      // the order of the points passed to the cubicTo() method.

      QRectF testRect(position.x() - proximityTolerance/2, position.y() - proximityTolerance/2,
                      proximityTolerance, proximityTolerance);

      QPointF p(path.elementAt(0));
      int i = 1;                    // The index into the list of elements.
      int n = 0;                    // The index of the point (not including control points).

      while (i < path.elementCount()) {
        int next;                   // The index of the next element.

        QPainterPath subpath;
        subpath.moveTo(p);

        QPainterPath::Element e = path.elementAt(i);

        switch (e.type) {
          case QPainterPath::LineToElement:
            p = QPointF(e.x, e.y);
            subpath.lineTo(p);
            next = i + 1;
            break;
          case QPainterPath::CurveToElement: {
            QPointF c1 = QPointF(e);
            // The two following elements are the control points.
            QPointF c2(path.elementAt(i + 1));
            p = QPointF(path.elementAt(i + 2));
            subpath.cubicTo(c1, c2, p);
            next = i + 3;
            break;
          }
          default:
            p = QPointF(e); // Unhandled element type - just use the point.
            next = i + 1;
            break;
        }

        if (subpath.intersects(testRect))
          return n;

        i = next;
        n += 1;
      }

      // Return -1 to indicate that no intersection occurred.
      return -1;

    } else {
      qreal minDist = distance2(position, points_[0], points_[1]);
      int n = points_.size();

      for (int i = 1; i < n; ++i) {
          const qreal dist = distance2(QPointF(position), QPointF(points_.at(i)), QPointF(points_.at((i + 1) % n)));
          if (dist < minDist) {
              minDist = dist;
              minIndex = i;
          }
      }
      if (minDist > proximityTolerance)
          return -1;
    }
    return minIndex;
}

QList<QAction *> PolyLine::actions(const QPoint &pos) const
{
  hitPointIndex_ = hitControlPoint(pos);
  hitLineIndex_ = hitLine(pos);
  hitLinePos_ = pos;

  QList<QAction *> acts;
  if (hitPointIndex_ != -1) {
    const QVariantMap style = DrawingStyleManager::instance()->getStyle(this);
    if (points_.size() > 3 || (!style.value("closed").toBool() && points_.size() > 2))
      acts.append(removePoint_act_);
  } else if (hitLineIndex_ != -1) {
    acts.append(addPoint_act_);
  }

  return acts;
}

void PolyLine::mousePress(
    QMouseEvent *event, bool &repaintNeeded, QList<QUndoCommand *> *undoCommands,
    QSet<QSharedPointer<DrawingItemBase> > *items, const QSet<QSharedPointer<DrawingItemBase> > *selItems, bool *multiItemOp)
{
  Q_ASSERT(undoCommands);
  Q_UNUSED(repaintNeeded);
  Q_UNUSED(undoCommands);
  Q_UNUSED(items);
  Q_UNUSED(selItems);

  if (event->button() == Qt::LeftButton) {
    pressedCtrlPointIndex_ = hitControlPoint(event->pos());
    resizing_ = (pressedCtrlPointIndex_ >= 0);
    moving_ = !resizing_;
    basePoints_ = points_;
    baseMousePos_ = event->pos();

    if (multiItemOp)
      *multiItemOp = moving_; // i.e. a move operation would apply to all selected items
  }
}

void PolyLine::incompleteMousePress(QMouseEvent *event, bool &repaintNeeded, bool &complete, bool &aborted)
{
  if (event->button() == Qt::LeftButton) {
    if (points_.size() == 0)
      points_.append(QPointF(event->pos()));
    points_.append(QPointF(event->pos()));
    repaintNeeded = true;
  } else if (event->button() == Qt::RightButton) {
    if (points_.size() >= 2) {
      complete = true; // causes repaint
    } else {
      aborted = true; // not a complete multiline
      repaintNeeded = true;
    }
  }
}

void PolyLine::incompleteMouseHover(QMouseEvent *event, bool &repaintNeeded)
{
  if (points_.size() > 0) {
    points_.last() = QPointF(event->pos());
    repaintNeeded = true;
  }
}

void PolyLine::incompleteKeyPress(QKeyEvent *event, bool &repaintNeeded, bool &complete, bool &aborted)
{
  if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
    if (points_.size() >= 2) {
      complete = true; // causes repaint
    } else {
      aborted = true; // not a complete multiline
      repaintNeeded = true;
    }
  } else if (event->key() == Qt::Key_Escape) {
    aborted = true;
    if (points_.size() > 0)
      repaintNeeded = true;
  }
}

void PolyLine::resize(const QPointF &pos)
{
    const QPointF delta = pos - baseMousePos_;
    Q_ASSERT(pressedCtrlPointIndex_ >= 0);
    Q_ASSERT(pressedCtrlPointIndex_ < controlPoints_.size());
    Q_ASSERT(basePoints_.size() == points_.size());
    points_[pressedCtrlPointIndex_] = basePoints_.at(pressedCtrlPointIndex_) + delta;
    updateControlPoints();
}

void PolyLine::updateControlPoints()
{
    controlPoints_.clear();
    const int size = controlPointSize(), size_2 = size / 2;
    foreach (QPointF p, points_)
        controlPoints_.append(QRectF(p.x() - size_2, p.y() - size_2, size, size));
}

void PolyLine::setPoints(const QList<QPointF> &points)
{
  setGeometry(points);
}

void PolyLine::addPoint()
{
    points_.insert(hitLineIndex_ + 1, hitLinePos_);
    setLatLonPoints(DrawingManager::instance()->getLatLonPoints(*this));

    updateControlPoints();
}

void PolyLine::removePoint()
{
    if ((hitPointIndex_ >= 0) && (hitPointIndex_ < points_.size())) {
        points_.removeAt(hitPointIndex_);
        latLonPoints_.removeAt(hitPointIndex_);
        hoveredCtrlPointIndex_ = -1;
    }

    updateControlPoints();
}

qreal PolyLine::dist2(const QPointF &v, const QPointF &w)
{
  return sqr(v.x() - w.x()) + sqr(v.y() - w.y());
}

// Returns the distance between \a p and the line between \a v and \a w.
qreal PolyLine::distance2(const QPointF &p, const QPointF &v, const QPointF &w)
{
    const qreal l2 = dist2(v, w);
    if (l2 == 0) return sqrt(dist2(p, v));
    Q_ASSERT(l2 > 0);
    const qreal t = ((p.x() - v.x()) * (w.x() - v.x()) + (p.y() - v.y()) * (w.y() - v.y())) / l2;
    if (t < 0) return sqrt(dist2(p, v));
    if (t > 1) return sqrt(dist2(p, w));
    QPointF p2(v.x() + t * (w.x() - v.x()), v.y() + t * (w.y() - v.y()));
    return sqrt(dist2(p, p2));
}

/**
 * Returns the distance between \a p and the multiline (i.e. the minimum
 * distance between \a p and any of the line segments).
 * If the multiline contains fewer than two points, the function returns -1.
 */
qreal PolyLine::distance(const QPointF &p) const
{
    if (points_.size() < 2)
        return -1;

    int n = points_.size();

    qreal minDist = -1;
    for (int i = 1; i <= n; ++i) {
        const qreal dist = distance2(QPointF(p), QPointF(points_.at(i - 1)), QPointF(points_.at(i % n)));
        minDist = (i == 1) ? dist : qMin(minDist, dist);
    }
    Q_ASSERT(minDist >= 0);
    return minDist;
}

void PolyLine::drawIncomplete() const
{
}

void PolyLine::drawHoverHighlighting(bool incomplete) const
{
  if (incomplete)
    return;

  glColor3ub(255, 0, 0);

  if (hoveredCtrlPointIndex_ >= 0) {
    EditItemBase::drawHoveredControlPoint();
  } else {
    DrawingStyleManager *styleManager = DrawingStyleManager::instance();

    // highlight the polyline
    glPushAttrib(GL_LINE_BIT);
    bool ok = false;
    const int lineWidth = properties().value("style:lineWidth").toInt(&ok);
    glLineWidth(ok ? lineWidth : 2);
    styleManager->drawLines(this, points_, 1);
    glPopAttrib();
  }
}

} // namespace EditItem_PolyLine
