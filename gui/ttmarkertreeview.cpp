/*----------------------------------------------------------------------------*/
/* COPYRIGHT: TriTime (c) 2003/2015 / www.tritime.org                         */
/*----------------------------------------------------------------------------*/
/* PROJEKT  : TTCUT 2008                                                      */
/* FILE     : ttmarkertreeview.cpp                                            */
/*----------------------------------------------------------------------------*/
/* AUTHOR  : b. altendorf (E-Mail: b.altendorf@tritime.de)   DATE: 12/18/2008 */
/* MODIFIED: b. altendorf                                    DATE: 01/14/2015 */
/*----------------------------------------------------------------------------*/

// ----------------------------------------------------------------------------
// TTMARKERTREEVIEW
// ----------------------------------------------------------------------------

/*----------------------------------------------------------------------------*/
/* This program is free software; you can redistribute it and/or modify it    */
/* under the terms of the GNU General Public License as published by the Free */
/* Software Foundation;                                                       */
/* either version 2 of the License, or (at your option) any later version.    */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but WITHOUT*/
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      */
/* FITNESS FOR A PARTICULAR PURPOSE.                                          */
/* See the GNU General Public License for more details.                       */
/*                                                                            */
/* You should have received a copy of the GNU General Public License along    */
/* with this program; if not, write to the Free Software Foundation,          */
/* Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.              */
/*----------------------------------------------------------------------------*/


#include "ttmarkertreeview.h"

#include "data/ttmarkerlist.h"
#include "data/ttavlist.h"
#include "avstream/ttavstream.h"

#include <QHeaderView>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QAction>

/* /////////////////////////////////////////////////////////////////////////////
 * Construct a new TTAudioFileList widget.
 */
TTMarkerTreeView::TTMarkerTreeView(QWidget* parent)
  :QWidget(parent)
{
  setupUi( this );

  markerListView->setRootIsDecorated(false);
  QHeaderView* header = markerListView->header();
  header->resizeSection(0, 260);
  header->resizeSection(1, 120);
  header->resizeSection(2, 160);
  header->resizeSection(3, 100);
}

/* /////////////////////////////////////////////////////////////////////////////
 * setAVData
 */
void TTMarkerTreeView::setAVData(TTAVData* avData)
{
  mpAVData = avData;

  connect(mpAVData, SIGNAL(markerAppended(const TTMarkerItem&)),                     SLOT(onAppendItem(const TTMarkerItem&)));
  connect(mpAVData, SIGNAL(markerRemoved(int)),                                      SLOT(onItemRemoved(int)));
  connect(mpAVData, SIGNAL(markerUpdated(const TTMarkerItem&, const TTMarkerItem&)), SLOT(onUpdateItem(const TTMarkerItem&, const TTMarkerItem&)));
  connect(mpAVData, SIGNAL(markerDataReloaded()),                                    SLOT(onReloadList()));
  connect(this,     SIGNAL(removeItem(const TTMarkerItem&)),               mpAVData, SLOT(onRemoveMarker(const TTMarkerItem&)));
  connect(this,     SIGNAL(itemOrderChanged(int, int)),                    mpAVData, SLOT(onMarkerOrderChanged(int , int)));
}

void TTMarkerTreeView::clear()
{
	markerListView->clear();
}

/* /////////////////////////////////////////////////////////////////////////////
 * Enable or disable the widget
 */
void TTMarkerTreeView::controlEnabled( bool enabled )
{
  pbEntryUp->setEnabled(enabled);
  pbEntryDelete->setEnabled(enabled);
  pbEntryDown->setEnabled(enabled);
  markerListView->setEnabled(enabled);
}

/* /////////////////////////////////////////////////////////////////////////////
 * onClearList
 */
void TTMarkerTreeView::onClearList()
{
  markerListView->clear();
}

/* /////////////////////////////////////////////////////////////////////////////
 * onAppendItem
 */
void TTMarkerTreeView::onAppendItem(const TTMarkerItem& item)
{
  QTreeWidgetItem* treeItem = new QTreeWidgetItem(markerListView);

  treeItem->setText(0, item.fileName());
  treeItem->setText(1, item.markerPosString());
  treeItem->setText(2, item.markerTimeString());
  treeItem->setText(3, item.ID().toString());
}

/*!
 * onUpdateItem
 */
void TTMarkerTreeView::onUpdateItem(const TTMarkerItem& mItem, const TTMarkerItem& uItem)
{
  QTreeWidgetItem* treeItem = findItem(mItem);

  if (treeItem == 0) {
    qDebug("TTMarkerTreeView::item not found!");
   	return;
  }

  treeItem->setText(0, uItem.fileName());
  treeItem->setText(1, uItem.markerPosString());
  treeItem->setText(2, uItem.markerTimeString());
  treeItem->setText(3, uItem.ID().toString());

  emit itemUpdated(mItem);
  emit refreshDisplay();
}


/* //////////////////////////////////////////////////////////////////////////////
 * Swap two items in list
 */
void TTMarkerTreeView::onSwapItems(int oldIndex, int newIndex)
{
    QTreeWidgetItem* listItem = markerListView->takeTopLevelItem(oldIndex);

    markerListView->insertTopLevelItem(newIndex, listItem);
    markerListView->setCurrentItem(listItem);

    emit itemOrderChanged(oldIndex, newIndex);
}

/* /////////////////////////////////////////////////////////////////////////////
 * Event handler for item up button
 */
void TTMarkerTreeView::onItemUp()
{
  if (markerListView->currentItem() == 0)  return;

    int index = markerListView->indexOfTopLevelItem(markerListView->currentItem());

    if (index <= 0) return;

    onSwapItems(index, index-1);
}

/* /////////////////////////////////////////////////////////////////////////////
 * Event handler for item down button
 */
void TTMarkerTreeView::onItemDown()
{
  if (markerListView->currentItem() == 0)  return;

  int index = markerListView->indexOfTopLevelItem(markerListView->currentItem());

  if (index >= markerListView->topLevelItemCount()-1) return;

  onSwapItems(index, index+1);
}

/* /////////////////////////////////////////////////////////////////////////////
 * Event handler for remove item button
 */
void TTMarkerTreeView::onRemoveItem()
{
  if (markerListView->currentItem() == 0) return;

  int index = markerListView->indexOfTopLevelItem(markerListView->currentItem());

  emit removeItem(mpAVData->markerAt(index));
}

/* //////////////////////////////////////////////////////////////////////////////
 *
 */
void TTMarkerTreeView::onItemRemoved(int index)
{
  delete markerListView->takeTopLevelItem(index);
}

void TTMarkerTreeView::onActivateMarker()
{
  if (markerListView->currentItem() == 0) return;

  int index = markerListView->indexOfTopLevelItem(markerListView->currentItem());

  emit gotoMarker(mpAVData->markerAt(index));
}

/* /////////////////////////////////////////////////////////////////////////////
 * onContextMenuRequest
 * User requested a context menu
 */
void TTMarkerTreeView::onContextMenuRequest(const QPoint& point)
{
  if (markerListView->currentItem() == 0)
    return;

  QMenu contextMenu(this);
  contextMenu.addAction(actionItemUp);
  contextMenu.addAction(actionItemDelete);
  contextMenu.addAction(actionItemDown);

  contextMenu.exec(markerListView->mapToGlobal(point));
}

void TTMarkerTreeView::onReloadList()
{
  onClearList();

  for (int i = 0; i < mpAVData->markerCount(); i++) {
    onAppendItem(mpAVData->markerAt(i));
  }
}

QTreeWidgetItem* TTMarkerTreeView::findItem(const TTMarkerItem& markerItem)
{
	for (int i = 0; i < markerListView->topLevelItemCount(); i++) {
		QTreeWidgetItem* item = markerListView->topLevelItem(i);
		if (item->text(3) == QString("%1").arg(markerItem.ID().toString()))
			return item;
	}
	return 0;
}

