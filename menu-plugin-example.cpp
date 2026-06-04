/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xshrim <xshrim@yeah.net>
 *
 * Reference: https://github.com/ukui/peony-extensions/blob/Debian/peony-menu-plugin-mate-terminal/mate-terminal-menu-plugin.cpp
 */

#include "menu-plugin-example.h"

#include <QAction>
#include <QMenu>

#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

using namespace Peony;

MenuPluginExample::MenuPluginExample(QObject* parent) : QObject(parent) {}

QString MenuPluginExample::testPlugin() {
  qDebug() << "menu test plugin1";
  return QString("MenuPluginExample");
}

QList<QAction*> MenuPluginExample::menuActions(Types types, const QString& uri, const QStringList& selectionUris) {
  // return QList<QAction *>();
  Q_UNUSED(types);
  // Q_UNUSED(uri);
  // Q_UNUSED(selectionUris);
  QList<QAction*> actions;

  if (!selectionUris.isEmpty()) {
    QUrl url = selectionUris.first();
    QFileInfo fileInfo(url.toLocalFile());
    
    QString menuText = tr("Xarchiver压缩");
    bool isArchive = false;
    if (fileInfo.exists() && fileInfo.isFile()) {
      QStringList archiveExtensions;
      archiveExtensions << "zip" << "tar" << "gz" << "bz2" << "xz" << "7z" << "rar" << "tgz";
      QString ext = fileInfo.suffix().toLower();
      if (archiveExtensions.contains(ext)) {
        isArchive = true;
        menuText = tr("Xarchiver解压");
      }
    }
    QAction* action0 = new QAction(QIcon::fromTheme("package-x-generic"), menuText);
    connect(action0, &QAction::triggered, [=]() {
      if (url.path().isEmpty()) return;
        QString command = "xarchiver";
        QStringList args;
      if (isArchive) {
        args << "--extract-to" << url.path() << ".";
      } else {
        args << "--add" << "--compress";
        for (const QUrl &iurl : selectionUris) {
          args << iurl.path();
        }
      }
      if (!QProcess::startDetached("xarchiver", args)) {
        qWarning() << "无法启动 xarchiver，请检查是否已安装在系统路径中。";
      }
    });
    actions << action0;
    
    if (fileInfo.exists() && fileInfo.isFile()) {
      QAction* action1 = new QAction(QIcon::fromTheme("text-edit"), tr("NotePad打开"));
      connect(action1, &QAction::triggered, [=]() {
        QStringList args;
        args << url.path();
        if (!QProcess::startDetached("notepad--", args)) {
          qWarning() << "无法启动 notepad--，请检查是否已安装在系统路径中。";
        }
      });
      actions << action1;
    }
  }

  QAction* action2 = new QAction(QIcon::fromTheme("media-eject"), tr("VSCode打开"));
  connect(action2, &QAction::triggered, [=]() {
    QProcess p(0);
    QString command = "code";
    QStringList args;
    QUrl url;
    if (selectionUris.isEmpty()) {
      url = uri;
    } else {
      url = selectionUris.first();
    }
    args.append(url.path());
    p.execute(command, args);
    qDebug() << QString::fromLocal8Bit(p.readAllStandardError());
  });
  actions << action2;

  QAction* action3 = new QAction(QIcon::fromTheme("media-playback-start"), tr("动作"));
  actions << action3;
  QMenu* menu = new QMenu(action3->parentWidget());
  connect(action3, &QAction::destroyed, [=]() {
    qDebug() << "delete sub menu";
    menu->deleteLater();
  });
  QAction* subaction1 = menu->addAction("动作1");
  menu->addSeparator();
  QAction* subaction2 = menu->addAction("动作2");
  action3->setMenu(menu);
  connect(subaction1, &QAction::triggered, [=]() {
    qDebug() << "click sub action1";
    QProcess p(0);
    QString command = "echo";
    QStringList args;
    args.append("action1");
    p.execute(command, args);
  });
  connect(subaction2, &QAction::triggered, [=]() {
    qDebug() << "click sub action1";
    QProcess p(0);
    QString command = "echo";
    QStringList args;
    args.append("action2");
    p.execute(command, args);
  });
  // QAction* action3 = new QAction(QIcon::fromTheme("document-open"), tr("打开"));
  // connect(action3, &QAction::triggered, [=]() {
  //  QProcess p(0);
  //  QString command = "code";
  //  QStringList args;
  //  QUrl url;
  //  if (selectionUris.isEmpty()) {
  //    command = "peony";
  //    url = uri;
  //  } else {
  //    command = "xdg-open";
  //    url = selectionUris.first();
  //  }
  //  args.append(url.path());
  //  p.execute(command, args);
  //  qDebug() << QString::fromLocal8Bit(p.readAllStandardError());
  // });
  // actions << action3;

  return actions;
}
