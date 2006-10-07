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
* Program URL   : http://qdevelop.org
*
*/
#include "subclassingimpl.h"
#include "projectmanager.h"
#include "ui_newimplementation.h"
#include <QFile>
#include <QList>
#include <QMetaMethod>
#include <QUiLoader>
#include <QFileDialog>
#include <QDebug>

//
SubclassingImpl::SubclassingImpl(ProjectManager * parent, QString dirProject, QString uiName, QStringList headers) 
	: QDialog(0), m_projectDirectory(dirProject), m_parent(parent), m_uiName(uiName)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	setupUi(this); 
	connect(okButton, SIGNAL(clicked()), this, SLOT(slotAccept()) );
	connect(newImplementation, SIGNAL(clicked()), this, SLOT(slotNewImplementation()) );
	connect(comboClassName, SIGNAL(activated(int)), this, SLOT(slotParseForm()) );
	implementations(headers);
	if( comboClassName->count() )
	{
		comboClassName->setCurrentIndex( 0 );
		slotParseForm();
	}
    QApplication::restoreOverrideCursor();
}
//
//
QString SubclassingImpl::newFile()
{
	QString filename = comboClassName->itemData( comboClassName->currentIndex() ).toString();
	return filename;
}
//
void SubclassingImpl::slotAccept()
{
	QString filename = comboClassName->itemData( comboClassName->currentIndex() ).toString();
	QStringList headerImpl;
	QFile file( filename+".h" );
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		headerImpl = QString(file.readAll()).split("\n");
		file.close();
	}
	else
		headerImpl = templateHeaderImpl();
	int index = headerImpl.indexOf( "private slots:" );
	if( index == -1 )
	{
		int last = headerImpl.lastIndexOf("};");
		if( last == -1 )
			last = headerImpl.lastIndexOf("}");
		headerImpl.insert(last, "private slots:" );
		index = headerImpl.indexOf( "private slots:" );
	}
	//
	QStringList sourceImpl;
	file.setFileName( filename+".cpp" );
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		sourceImpl = QString(file.readAll()).split("\n");
		file.close();
	}
	else
		sourceImpl = templateSourceImpl();
	//
	for(int i=0; i<treeSlots->topLevelItemCount(); i++)
	{
		QTreeWidgetItem *topLevelItem = treeSlots->topLevelItem( i );
		for(int j=0; j< topLevelItem->childCount(); j++)
		{
			QTreeWidgetItem *item = topLevelItem->child( j );
			if( item->checkState( 0 ) && item->textColor(0)!= Qt::blue )
			{
				QString s = item->data(0, Qt::UserRole).toString();
				headerImpl.insert(++index, "\tvoid "+s+";");
				sourceImpl << "void "+ comboClassName->currentText() + "::" + s;
				sourceImpl << "{";
				sourceImpl << "}";				
				sourceImpl << "//";

			}
		}
	}
	//
	file.setFileName( filename+".h" );
	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		foreach(QString line, headerImpl)
			file.write( line.toLatin1()+"\n" );
	}
	file.close();
	//
	file.setFileName( filename+".cpp" );
	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		foreach(QString line, sourceImpl)
			file.write( line.toLatin1()+"\n" );
	}
	file.close();
	
	//qDebug()<<headerImpl;
	//qDebug()<<sourceImpl;
	//
}
//
void SubclassingImpl::slotLocation()
{
	QString s = QFileDialog::getExistingDirectory(
		this,
		tr("Choose the file location"),
		m_projectDirectory,
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	if( s.isEmpty() )
	{
		// Cancel is clicked
		return;
	}
	uiNewImplementation.location->setText( s );	
}
//
void SubclassingImpl::slotNewImplementation()
{
	QDialog *dial = new QDialog;
	uiNewImplementation.setupUi(dial);
	connect(uiNewImplementation.locationButton, SIGNAL(clicked()), this, SLOT(slotLocation()) );
	connect(uiNewImplementation.className, SIGNAL(textChanged(QString)), this, SLOT(slotEnableokButton(QString)) );
	connect(uiNewImplementation.location, SIGNAL(textChanged(QString)), this, SLOT(slotEnableokButton(QString)) );
	connect(uiNewImplementation.filename, SIGNAL(textChanged(QString)), this, SLOT(slotEnableokButton(QString)) );
	if( !comboClassName->count() )
	{
		uiNewImplementation.className->setText(objectName()+"Impl");
		uiNewImplementation.location->setText(m_projectDirectory+"/");
		uiNewImplementation.filename->setText(QString(objectName()+"Impl").toLower());
	}
	if( dial->exec() == QDialog::Accepted )
	{
		if( uiNewImplementation.filename->text().right(2).toLower() == ".h" || uiNewImplementation.filename->text().right(4).toLower() == ".cpp" )
			uiNewImplementation.filename->setText( uiNewImplementation.filename->text().left( uiNewImplementation.filename->text().lastIndexOf(".") ) );
	   	comboClassName->addItem(uiNewImplementation.className->text(), QVariant(uiNewImplementation.location->text()+"/"+uiNewImplementation.filename->text()));
		comboClassName->setCurrentIndex( comboClassName->count()-1 );
		slotParseForm();
	}
	delete dial;
}
void SubclassingImpl::slotEnableokButton(QString)
{
	if( uiNewImplementation.location->text().isEmpty() || uiNewImplementation.className->text().isEmpty() || uiNewImplementation.filename->text().isEmpty() )
		uiNewImplementation.okButton->setDisabled( true );
	else
		uiNewImplementation.okButton->setEnabled( true );
}
//
void SubclassingImpl::slotParseForm()
{
	//
	QString data = comboClassName->itemData( comboClassName->currentIndex() ).toString();
	if( QFile::exists( data + ".h"  ) )
	{
		okButton->setText( tr("&Update") );
	}
	else
	{
		okButton->setText( tr("C&reate") );
	}
	okButton->setEnabled( true );
	filename->setText( data.section("/", -1).section(".", 0, 0) + " "+tr("(.h and .cpp)"));
	location->setText( data.left( data.lastIndexOf("/")+1 ) );
	//
	QString headerContent;
	QFile fileHeader( comboClassName->itemData( comboClassName->currentIndex() ).toString() + ".h");
	if (fileHeader.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		headerContent = fileHeader.readAll();
		fileHeader.close();
	}
	treeSlots->clear();
	QStringList listeSignatures = signatures(headerContent);
	//
	QTreeWidgetItem *item;
	//
    QUiLoader loader;
    QFile file(m_uiName);
    file.open(QFile::ReadOnly);
    QWidget *dial = loader.load(&file, this);
    file.close();
    //
	QList<QWidget *> widgets;
	widgets += dial->findChildren<QWidget *>();
	foreach(QWidget *w, widgets)
	{
       	QString name = w->objectName();
       	if( name.isEmpty() )
       		continue;
       	QString className = w->metaObject()->className();        	
		item = new QTreeWidgetItem(QStringList(name+" ("+className+")"));
		treeSlots->addTopLevelItem ( item );
		for(int i=0; i<w->metaObject()->methodCount(); i++)
		{
			QMetaMethod meta = w->metaObject()->method( i );
			if( meta.methodType() == QMetaMethod::Signal )
			{
				QTreeWidgetItem *item2 = new QTreeWidgetItem(item, QStringList()<<"on_"+name+"_"+meta.signature());
				QString method = QString("on_"+name+"_"+meta.signature()).section("(", 0, 0) + "(";
				for(int param=0; param<meta.parameterTypes().count(); param++)
					method += meta.parameterTypes().at(param)+" "+meta.parameterNames().at(param)+", ";
				if( method.right(2) == ", " )
					method = method.left( method.length() - 2 );
				method += ")";
				item2->setData(0, Qt::UserRole, QVariant(method));
				if( listeSignatures.contains( "on_"+name+"_"+meta.signature() ) )
				{
					item2->setFlags( Qt::ItemIsEnabled );
					item2->setCheckState(0, Qt::Checked );
					item2->setTextColor(0, Qt::blue);
				}
				else
				{
					item2->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
					item2->setCheckState(0, Qt::Unchecked );
				}
			}
		}
		treeSlots->expandItem( item );
	}
	delete dial;
	return;
	//
}
// 
QStringList SubclassingImpl::signatures(QString header)
{
	QStringList sign;
	bool slot = false;
	QString formattedParams, name;
	foreach(QString line, header.split("\n") )
	{
		if( line.simplified().right(1) == ":" )
		{
			if( line.simplified().contains("slots") )
				slot = true;
			else
				slot = false;
			continue;
		}
		if( slot )
		{
			line = line.simplified();
			line = line.section(" ", 1).simplified();
			name = line.section("(", 0, 0).simplified();
			QString params = line.section("(", 1).section(")", 0, 0).simplified();
			formattedParams.clear();
			foreach(QString param, params.split(",") )
			{
				if( param.contains("=") )
					param = param.simplified().left( param.simplified().lastIndexOf("=") );
				if( param.contains("&") )
					param = param.simplified().left( param.simplified().lastIndexOf("&")+1 );
				else if( param.contains("*") )
					param = param.simplified().left( param.simplified().lastIndexOf("*")+1 );
				else if( param.simplified().contains(" ") )
					param = param.simplified().left( param.simplified().lastIndexOf(" ")+1 );
				formattedParams += param + ",";
				//qDebug()<<param;
			}
			formattedParams = formattedParams.simplified().left( formattedParams.lastIndexOf(",") );
			QString s = name + "(" + formattedParams + ")";
			sign << QMetaObject::normalizedSignature( s.toLatin1() );
		}
	}
	return sign;
}
//
void SubclassingImpl::implementations(QStringList headers)
{
	QString name = objectName();
	foreach(QString header, headers)
	{
	    QFile file(header);
	    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	        return;
	
	    QTextStream in(&file);
	    QString className;
	    while (!in.atEnd()) 
	    {
	        QString line = in.readLine();
	        if( line.simplified().indexOf("class ") == 0 && !line.contains(";") )
	        	className = line.simplified().section("class ", 1, 1).section(":", 0, 0).simplified().section(" ", -1, -1).simplified();
	        if( line.contains("Ui::"+name) )
	        {
	        	header = header.left( header.lastIndexOf(".") );
	        	comboClassName->addItem(className, QVariant(header));
	        	break;
        	}
	    }
	    file.close();
    }
	return;
}
//
QString SubclassingImpl::objectName()
{
    QUiLoader loader;
    QFile file(m_uiName);
    file.open(QFile::ReadOnly);
    QWidget *dial = loader.load(&file, this);
    file.close();
   	QString name = dial->objectName();        	
    delete dial;
    return name;
}
//
QString SubclassingImpl::className()
{
    QUiLoader loader;
    QFile file(m_uiName);
    file.open(QFile::ReadOnly);
    QWidget *dial = loader.load(&file, this);
    file.close();
   	QString className = dial->metaObject()->className();        	
    delete dial;
    return className;
}
//
QStringList SubclassingImpl::templateHeaderImpl()
{
	QString filename = comboClassName->itemData( comboClassName->currentIndex() ).toString().section("/", -1).section(".", 0);
	QString classImpl = comboClassName->currentText();
	QFile file(":/templates/templates/impl.h");
	file.open(QIODevice::ReadOnly);
	QString data = file.readAll();
	file.close();
	data.replace("$IMPL_H", filename.toUpper()+"_H");
	data.replace("$UIHEADERNAME", "\"ui_"+m_uiName.section("/", -1, -1).section(".", 0, 0)+".h\"");
	data.replace("$CLASSNAME", classImpl);
	data.replace("$PARENTNAME", className());
	data.replace("$OBJECTNAME", objectName());
	QStringList impl = data.split("\n");
	/*impl << "#ifndef "+filename.toUpper()+"_H";
	impl << "#define "+filename.toUpper()+"_H";
	impl << "//";
	impl << "#include \"ui_"+m_uiName.section("/", -1, -1).section(".", 0, 0)+".h\"";
	impl << "//";
	impl << "class "+classImpl+ " : public "+className()+", public Ui::"+objectName() ;
	impl << "{";
	impl << "Q_OBJECT";
	impl << "public:";
	impl << "\t"+classImpl+"( QWidget * parent = 0, Qt::WFlags f = 0 );";
	impl << "private slots:";
	impl << "};";
	impl << "#endif";*/
	return impl;
}
//
QStringList SubclassingImpl::templateSourceImpl()
{
	QString filename = comboClassName->itemData( comboClassName->currentIndex() ).toString().section("/", -1);
	QString classImpl = comboClassName->currentText();
	QFile file(":/templates/templates/impl.cpp");
	file.open(QIODevice::ReadOnly);
	QString data = file.readAll();
	file.close();
	data.replace("$HEADERNAME", "\""+filename+".h\"");
	data.replace("$CLASSNAME", classImpl);
	data.replace("$PARENTNAME", className());
	QStringList impl = data.split("\n");
	/*impl << "#include \""+filename+".h\"";
	impl << "//";
	impl << classImpl + "::" + classImpl+"( QWidget * parent, Qt::WFlags f) : "+className()+"(parent, f)";
	impl << "{";
	impl << "\tsetupUi(this);";
	impl << "}";
	impl << "//";*/
	return impl;
}
//