/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "xkb_helper.h"

#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QX11Info>
#include <QStandardPaths>
#include <QDebug>

#include <kprocess.h>

#include "keyboard_config.h"


static const char* SETXKBMAP_EXEC = "setxkbmap";
static const char* XMODMAP_EXEC = "xmodmap";

static bool setxkbmapNotFound = false;
static QString setxkbmapExe;

static bool xmodmapNotFound = false;
static QString xmodmapExe;

static const QString COMMAND_OPTIONS_SEPARATOR(",");

static
QString getSetxkbmapExe()
{
	if( setxkbmapNotFound )
		return "";

	if( setxkbmapExe.isEmpty() ) {
		setxkbmapExe = QStandardPaths::findExecutable(SETXKBMAP_EXEC);
		if( setxkbmapExe.isEmpty() ) {
			setxkbmapNotFound = true;
			qCritical() << "Can't find" << SETXKBMAP_EXEC << "- keyboard layouts won't be configured";
			return "";
		}
	}
	return setxkbmapExe;
}

static
void executeXmodmap(const QString& configFileName)
{
	if( xmodmapNotFound )
		return;

    if( QFile(configFileName).exists() ) {
    	if( xmodmapExe.isEmpty() ) {
    		xmodmapExe = QStandardPaths::findExecutable(XMODMAP_EXEC);
        	if( xmodmapExe.isEmpty() ) {
    			xmodmapNotFound = true;
    			qCritical() << "Can't find" << XMODMAP_EXEC << "- xmodmap files won't be run";
    			return;
        	}
    	}

    	KProcess xmodmapProcess;
    	xmodmapProcess << xmodmapExe;
    	xmodmapProcess << configFileName;
    	qDebug() << "Executing" << xmodmapProcess.program().join(" ");
    	if( xmodmapProcess.execute() != 0 ) {
    		qCritical() << "Failed to execute " << xmodmapProcess.program();
    	}
    }
}

static
void restoreXmodmap()
{
	// TODO: is just home .Xmodmap enough or should system be involved too?
	//    QString configFileName = QDir("/etc/X11/xinit").filePath(".Xmodmap");
	//    executeXmodmap(configFileName);
	QString configFileName = QDir::home().filePath(".Xmodmap");
	executeXmodmap(configFileName);
}

//TODO: make private
bool XkbHelper::runConfigLayoutCommand(const QStringList& setxkbmapCommandArguments)
{
	QTime timer;
	timer.start();

	KProcess setxkbmapProcess;
	setxkbmapProcess << getSetxkbmapExe() << setxkbmapCommandArguments;
	int res = setxkbmapProcess.execute();

	if( res == 0 ) {	// restore Xmodmap mapping reset by setxkbmap
		qDebug() << "Executed successfully in " << timer.elapsed() << "ms" << setxkbmapProcess.program().join(" ");
		restoreXmodmap();
		qDebug() << "\t and with xmodmap" << timer.elapsed() << "ms";
	    return true;
	}
	else {
		qCritical() << "Failed to run" << setxkbmapProcess.program().join(" ") << "return code:" << res;
	}
	return false;
}

bool XkbHelper::initializeKeyboardLayouts(const QList<LayoutUnit>& layoutUnits)
{
	QStringList layouts;
	QStringList variants;
	foreach (const LayoutUnit& layoutUnit, layoutUnits) {
		layouts.append(layoutUnit.layout);
		variants.append(layoutUnit.variant);
	}

	QStringList setxkbmapCommandArguments;
	setxkbmapCommandArguments.append("-layout");
	setxkbmapCommandArguments.append(layouts.join(COMMAND_OPTIONS_SEPARATOR));
	if( ! variants.join("").isEmpty() ) {
		setxkbmapCommandArguments.append("-variant");
		setxkbmapCommandArguments.append(variants.join(COMMAND_OPTIONS_SEPARATOR));
	}

	return runConfigLayoutCommand(setxkbmapCommandArguments);
}

bool XkbHelper::initializeKeyboardLayouts(KeyboardConfig& config)
{
	QStringList setxkbmapCommandArguments;
	if( ! config.keyboardModel.isEmpty() ) {
		XkbConfig xkbConfig;
		X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::MODEL_ONLY);
		if( xkbConfig.keyboardModel != config.keyboardModel ) {
			setxkbmapCommandArguments.append("-model");
			setxkbmapCommandArguments.append(config.keyboardModel);
		}
	}
	if( config.configureLayouts ) {
		QStringList layouts;
		QStringList variants;
		QList<LayoutUnit> defaultLayouts = config.getDefaultLayouts();
		foreach (const LayoutUnit& layoutUnit, defaultLayouts) {
			layouts.append(layoutUnit.layout);
			variants.append(layoutUnit.variant);
		}

		setxkbmapCommandArguments.append("-layout");
		setxkbmapCommandArguments.append(layouts.join(COMMAND_OPTIONS_SEPARATOR));
		if( ! variants.join("").isEmpty() ) {
			setxkbmapCommandArguments.append("-variant");
			setxkbmapCommandArguments.append(variants.join(COMMAND_OPTIONS_SEPARATOR));
		}
	}
	if( config.resetOldXkbOptions ) {
		setxkbmapCommandArguments.append("-option");
	}
	if( ! config.xkbOptions.isEmpty() ) {
		setxkbmapCommandArguments.append("-option");
		setxkbmapCommandArguments.append(config.xkbOptions.join(COMMAND_OPTIONS_SEPARATOR));
	}

	if( ! setxkbmapCommandArguments.isEmpty() ) {
		return runConfigLayoutCommand(setxkbmapCommandArguments);
		if( config.configureLayouts ) {
			X11Helper::setDefaultLayout();
		}
	}
	return false;
}
