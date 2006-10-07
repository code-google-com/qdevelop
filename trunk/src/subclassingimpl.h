/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@qtfr.org>
* Program URL   : http://qtfr.org
*
*/
#ifndef SUBCLASSINGIMPL_H
#define SUBCLASSINGIMPL_H

#include "ui_subclassing.h"
#include "ui_newimplementation.h"
//
class ProjectManager;
//
class SubclassingImpl : public QDialog, public Ui::Subclassing
{
Q_OBJECT
public:
	SubclassingImpl(ProjectManager * parent, QString dirProject, QString uiName, QStringList headers);
	QString newFile();
private:
	ProjectManager *m_parent;
	QString m_uiName;
	QString m_projectDirectory;
	void implementations(QStringList);
	static QStringList signatures(QString header) ;
	QString objectName();
	QString className();
	QStringList templateHeaderImpl();
	QStringList templateSourceImpl();
	Ui::NewImplementation uiNewImplementation;
private slots:
	void slotAccept();
	void slotNewImplementation();
	void slotParseForm();
	void slotLocation();
	void slotEnableokButton(QString);
signals:
};

#endif
