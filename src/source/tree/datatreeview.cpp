/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "datatreeview.h"
#include "treeheaderview.h"
#include "datamodel.h"
#include "uitools.h"

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>

#include <QMouseEvent>
#include <QPainter>
#include <QHeaderView>
#include <QMimeData>
#include <QDebug>
#include <QScrollBar>
#include <QPainterPath>

StyleTreeViewDelegate::StyleTreeViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

StyleTreeViewDelegate::~StyleTreeViewDelegate()
{

}

QSize StyleTreeViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    return QSize(option.rect.width(), 36);
}

void StyleTreeViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);
    //?????????????????????
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            forground.setColor(palette.color(cg, DPalette::HighlightedText));
        }
    }

    painter->setPen(forground);
    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QPainterPath path, clipPath;
    QRect textRect = rect;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        // ?????????
        rect.setX(rect.x() + margin);
    } break;
    case QStyleOptionViewItem::Middle: {
    } break;
    case QStyleOptionViewItem::End: {
        // ?????????
        rect.setWidth(rect.width() - margin);
    } break;
    case QStyleOptionViewItem::OnlyOne: {
        // ?????????
        rect.setX(rect.x() + margin);
        // ?????????
        rect.setWidth(rect.width() - margin);
    } break;
    default: {
        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }

    //????????????
    if (QStyleOptionViewItem::Beginning == opt.viewItemPosition &&
            index.data(Qt::DecorationRole).isValid()) {
        // icon size
        auto iconSize = style->pixelMetric(DStyle::PM_ListViewIconSize, &option);
        QRect iconRect = rect;
        iconRect.setX(rect.x() + margin);
        iconRect.setWidth(iconSize);
        // ?????????????????????
        auto diff = (iconRect.height() - iconSize) / 2;
        opt.icon.paint(painter, iconRect.adjusted(0, diff, 0, -diff));
    }

    //????????????
    textRect = rect;
    if (0 == index.column()) {
        textRect.setX(textRect.x() + margin + 32 - 2);
    } else {
        textRect.setX(textRect.x() + margin - 2);
    }

    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

DataTreeView::DataTreeView(QWidget *parent)
    : DTreeView(parent)
{
    initUI();
    initConnections();
}

DataTreeView::~DataTreeView()
{

}

void DataTreeView::resetLevel()
{
    m_iLevel = 0;
    m_strCurrentPath = QDir::separator();
    setPreLblVisible(false);
}

void DataTreeView::initUI()
{
    setAttribute(Qt::WA_AcceptTouchEvents); // ???????????????????????????
    setObjectName("TableViewFile");
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setIconSize(QSize(24, 24));
    setViewportMargins(10, 10, 10, 0);
    setIndentation(0);
    setFrameShape(QFrame::NoFrame);     // ???????????????
    resizeColumnWidth();
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFocusPolicy(Qt::StrongFocus);

    // ??????????????????
    StyleTreeViewDelegate *pDelegate = new StyleTreeViewDelegate(this);
    setItemDelegate(pDelegate);

    // ????????????
    m_pHeaderView = new TreeHeaderView(Qt::Horizontal, this);
    m_pHeaderView->setStretchLastSection(true);
    setHeader(m_pHeaderView);

    m_pModel = new DataModel(this);
    setModel(m_pModel);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);    // ???????????????????????????
    setAcceptDrops(true);

    m_selectionModel = selectionModel();
}

void DataTreeView::initConnections()
{
    connect(m_pHeaderView->getpreLbl(), &PreviousLabel::doubleClickedSignal, this, &DataTreeView::slotPreClicked);
}

void DataTreeView::resizeColumnWidth()
{
    setColumnWidth(0, width() * 25 / 58);
    setColumnWidth(1, width() * 17 / 58);
    setColumnWidth(2, width() * 8 / 58);
    setColumnWidth(3, width() * 8 / 58);
}

TreeHeaderView *DataTreeView::getHeaderView() const
{
    return m_pHeaderView;
}

void DataTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
#endif
    }

    auto *style = dynamic_cast<DStyle *>(DApplication::style());

    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
    auto margin = style->pixelMetric(DStyle::PM_ContentsMargins, &options);
    //???????????????????????????????????????????????????
    auto palette = options.palette;
    QBrush background;
    if (!(index.row() & 1)) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }
    if (options.state & DStyle::State_Enabled) {
        if (m_selectionModel->isSelected(index)) {
            background = palette.color(cg, DPalette::Highlight);
        }
    }

    // ???????????????????????????-2???????????????????????????????????????????????????????????????????????????????????????delegate???????????????????????????????????????????????????????????????????????????qrect?????????????????????????????????????????????????????????
    QPainterPath path;
    QRect rowRect { options.rect.x() - header()->offset(),
                    options.rect.y(),
                    header()->length() - header()->sectionPosition(0),
                    options.rect.height() - 2 };
    rowRect.setX(rowRect.x() + margin);
    rowRect.setWidth(rowRect.width() - margin);

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    QTreeView::drawRow(painter, options, index);
    // draw focus
    if (hasFocus() && currentIndex().row() == index.row() && (Qt::TabFocusReason == m_reson || Qt::BacktabFocusReason == m_reson)) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(options);
        o.state |= QStyle::State_KeyboardFocusChange | QStyle::State_HasFocus;
        o.rect = style->visualRect(layoutDirection(), viewport()->rect(), rowRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, painter);
    }

    painter->restore();
}

void DataTreeView::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    // qInfo() << m_reson << model()->rowCount() << currentIndex();
    if (Qt::BacktabFocusReason == m_reson || Qt::TabFocusReason == m_reson) { // ????????????????????????
        if (model()->rowCount() > 0) {
            if (currentIndex().isValid()) {
                m_selectionModel->select(currentIndex(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            } else {
                QModelIndex firstModelIndex = model()->index(0, 0);
                m_selectionModel->select(firstModelIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }

    DTreeView::focusInEvent(event);
}

void DataTreeView::dragEnterEvent(QDragEnterEvent *e)
{
    const auto *mime = e->mimeData();

    // ???????????????url
    if (!mime->hasUrls()) {
        e->ignore();
    } else {
        // ?????????????????????????????????????????? ?????? ?????? ftp smb ???
        for (const auto &url : mime->urls()) {
            if (!UiTools::isLocalDeviceFile(url.toLocalFile())) {
                e->ignore();
                return;
            }
        }

        e->accept();
    }
}

void DataTreeView::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void DataTreeView::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();

    if (false == mime->hasUrls()) {
        e->ignore();
    }

    e->accept();

    // ??????????????????
    QStringList fileList;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }

        fileList << url.toLocalFile();
    }

    if (fileList.size() == 0) {
        return;
    }

    emit signalDragFiles(fileList);
}

void DataTreeView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    resizeColumnWidth();
}

bool DataTreeView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin: {

        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);

        if (!m_isPressed && touchEvent && touchEvent->device() && touchEvent->device()->type() == QTouchDevice::TouchScreen && touchEvent->touchPointStates() == Qt::TouchPointPressed) {

            QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();
            //dell????????????????????????touchpoint ??????????????????pinchevent?????????????
            if (points.count() == 1) {
                QTouchEvent::TouchPoint p = points.at(0);
                m_lastTouchBeginPos = p.pos();
                m_lastTouchBeginPos.setY(m_lastTouchBeginPos.y() - m_pHeaderView->height());
                m_lastTouchTime = QTime::currentTime();
                m_isPressed = true;

            }
        }
        break;

    }
    default:
        break;
    }
    return  DTreeView::event(event);
}

void DataTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    DTreeView::mouseReleaseEvent(event);
}

void DataTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed) {
        //??????????????????????????????????????????,??????????????????????????????
        int touchmindistance = 2;
        //???????????????????????????????????????????????????,????????????-35~-38??????????????????,???????????????????????????????????????,???????????????30
        int touchMaxDistance = 30;
        event->accept();
        double horiDelta = event->pos().x() - m_lastTouchBeginPos.x();
        double vertDelta = event->pos().y() - m_lastTouchBeginPos.y();
        qInfo() << "event->pos()" << event->pos() << "m_pHeaderView->height()" << m_pHeaderView->height();
        if (qAbs(horiDelta) > touchmindistance && qAbs(horiDelta) < touchMaxDistance) {
            horizontalScrollBar()->setValue(static_cast<int>(horizontalScrollBar()->value() - horiDelta));
        }

        if (qAbs(vertDelta) > touchmindistance && !(qAbs(vertDelta) < m_pHeaderView->height() + 2 && qAbs(vertDelta) > m_pHeaderView->height() - 2 && m_lastTouchTime.msecsTo(QTime::currentTime()) < 100)) {
            double svalue = 1;
            if (vertDelta > 0) {
                //svalue = svalue;
            } else if (vertDelta < 0) {
                svalue = -svalue;
            } else {
                svalue = 0;
            }
            verticalScrollBar()->setValue(static_cast<int>(verticalScrollBar()->value() - vertDelta));
        }
        m_lastTouchBeginPos = event->pos();
        return;
    }
}

void DataTreeView::keyPressEvent(QKeyEvent *event)
{
    // ???????????????????????????????????????Key_Up,???????????????????????????
    if (Qt::Key_Up == event->key()) {
        // ????????????0????????????????????????????????????????????????????????????
        if (this->currentIndex().row() == 0 || model()->rowCount() == 0) {
            if (m_pHeaderView->isVisiable()) {
                m_pHeaderView->setLabelFocus(true);
                m_selectionModel->clearSelection();
            }
        }
    }

    // ?????????????????????????????????????????????Key_Down,?????????????????????
    if (Qt::Key_Down == event->key()) {
        if (m_pHeaderView->isVisiable() && m_pHeaderView->isLabelFocus() && model()->rowCount() > 0) {
            m_pHeaderView->setLabelFocus(false);
            m_selectionModel->setCurrentIndex(m_pModel->index(-1, -1), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    }

    if (Qt::Key_Delete == event->key() && m_selectionModel->selectedRows().count() > 0) { //?????????
        slotDeleteFile();
    } else if (Qt::Key_M == event->key() && Qt::AltModifier == event->modifiers()
               && m_selectionModel->selectedRows().count() > 0) { //Alt+M???????????????????????????
        int y =   36 * currentIndex().row() + 36 / 2; //???????????????y??????+??????/2,????????????36
        int x = static_cast<int>(width() * 0.618); //???????????????x??????

        slotShowRightMenu(QPoint(x, y));
    } else if ((Qt::Key_Enter == event->key() || Qt::Key_Return == event->key())
               && m_selectionModel->selectedRows().count() > 0) { //????????????????????????????????????(???)
        handleDoubleClick(currentIndex());
    } else {
        DTreeView::keyPressEvent(event);
    }
}

void DataTreeView::setPreLblVisible(bool bVisible, const QString &strPath)
{
    m_pHeaderView->getpreLbl()->setPrePath(strPath);
    m_pHeaderView->setPreLblVisible(bVisible);
}
