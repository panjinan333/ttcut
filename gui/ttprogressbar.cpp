/*----------------------------------------------------------------------------*/
/* COPYRIGHT: TriTime (c) 2003/2010 / www.tritime.org                         */
/*----------------------------------------------------------------------------*/
/* PROJEKT  : TTCUT 2009                                                      */
/* FILE     : ttprogressbar.cpp                                               */
/*----------------------------------------------------------------------------*/
/* AUTHOR  : b. altendorf (E-Mail: b.altendorf@tritime.de)   DATE: 03/11/2005 */
/*----------------------------------------------------------------------------*/

// ----------------------------------------------------------------------------
// TTPROGRESSBAR
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

#include "ttprogressbar.h"
#include "tttaskprogress.h"

#include "../common/ttexception.h"
#include "../common/istatusreporter.h"
#include "../common/ttthreadtask.h"

#include <QDebug>
#include <QApplication>

/**
 * Constructor
 */
TTProgressBar::TTProgressBar(QWidget* parent)
              : QDialog(parent)
{
  setupUi(this);

  scrollArea->hide();
  this->adjustSize();

  processForm    = 0;
  normTotalSteps = 1000;

  progressBar->setMinimum( 0 );
  progressBar->setMaximum( normTotalSteps );

  taskProgressHash = new QHash<QUuid, TTTaskProgress*>;

  connect(pbCancel,  SIGNAL(clicked()),         this,  SLOT(onBtnCancelClicked()));
  connect(cbDetails, SIGNAL(stateChanged(int)), this, SLOT(onDetailsStateChanged(int)));
}

/**
 * Destructor
 */
TTProgressBar::~TTProgressBar()
{
}

/**
 * Show the progress form
 */
void TTProgressBar::showBar()
{
  setModal(false);
  show();

  qApp->processEvents();
}

/**
 * Hide the progress form
 */
void TTProgressBar::hideBar()
{
  setModal(true);
  hide();
  
  qApp->processEvents();
}

/**
 * Set the action text
 */
void TTProgressBar::setActionText( QString action )
{
  actionString->setText( action );
}

/**
 * Set the current total progress values
 */
void TTProgressBar::setTotalProgress(int progress, QTime time)
{
    percentageString->setText(QString("%1%").arg((float)progress/10.0, 0, 'f', 0));
    progressBar->setValue(progress);
    elapsedTimeString->setText(time.toString("hh:mm:ss"));
}

/**
 * Set the task's progress value
 */
void TTProgressBar::setTaskProgress(TTThreadTask* task, const QString& msg)
{
  if (task == 0) return;
  if (!taskProgressHash->contains(task->taskID())) return;

  TTTaskProgress* tp = taskProgressHash->value(task->taskID());

  if (tp == 0) return;

  tp->onRefreshProgress(msg);
}

/**
 * Set the progress value to 100%
 */
void TTProgressBar::setComplete()
{
	progressBar->setValue(normTotalSteps);
}

/**
 * Reset the progress bar and remove all taskprogress widgets
 */
void TTProgressBar::resetProgress()
{
  progressBar->reset();

  foreach (TTTaskProgress* value, *taskProgressHash) {
    if (value == 0) continue;

    verticalLayout->removeWidget(value);
    delete value;
    value = 0;
  }
  taskProgressHash->clear();
  scrollArea->adjustSize();
  this->adjustSize();
}

/**
 * Show/hide the details view
 */
void TTProgressBar::onDetailsStateChanged(int)
{
  if (cbDetails->isChecked()) {
    scrollArea->show();
  } else {
    scrollArea->hide();
  }
  this->adjustSize();
}

/**
 * Button cancel clicked
 */
void TTProgressBar::onBtnCancelClicked()
{
  emit cancel();
}

/**
 * Set progress values
 */
void TTProgressBar::onSetProgress(TTThreadTask* task, int state, const QString& msg, int totalProgress, QTime totalTime)
{
  switch (state) {
    case StatusReportArgs::Init:
      qDebug("onSetProgress::Init");
      resetProgress();
      setActionText(msg);
      break;

    case StatusReportArgs::Start:
      qDebug("onSetProgress::Start");      
      setActionText(msg);
      addTaskProgress(task);
      break;

    case StatusReportArgs::Step:
      setTotalProgress(totalProgress, totalTime);
      setTaskProgress(task, msg);
      break;

    case StatusReportArgs::Finished:
      qDebug("onSetProgress::Finished");
      break;

    case StatusReportArgs::ShowProcessForm:
      showProcessForm();
      break;

    case StatusReportArgs::AddProcessLine:
      addProcessLine(msg);
      break;

    case StatusReportArgs::HideProcessForm:
      hideProcessForm();
      break;

    default:
      break;
  }
}

/**
 * Add progress bar for the given task
 */
void TTProgressBar::addTaskProgress(TTThreadTask* task)
{
  if (task == 0) return;
  if (taskProgressHash->contains(task->taskID())) return;

  TTTaskProgress* taskProgress = new TTTaskProgress(this, task);

  taskProgressHash->insert(task->taskID(), taskProgress);
  verticalLayout->addWidget(taskProgress);
}

/*!
 * showProcessForm
 */
void TTProgressBar::showProcessForm()
{
  if (processForm != 0) return;

  //hideBar();

	processForm = new TTProcessForm(TTCut::mainWindow);
	processForm->setModal(false);
	processForm->showCancelButton(false);
	processForm->show();
}

/*!
 * addProcessLine
 */
void TTProgressBar::addProcessLine(const QString& line)
{
	if (processForm == 0) return;

	processForm->addLine(line);
}

/*!
 * hideProcessForm
 */
void TTProgressBar::hideProcessForm()
{
	if (processForm != 0) {
		processForm->hide();
		delete processForm;
		processForm = 0;
	}

  //showBar();
}
