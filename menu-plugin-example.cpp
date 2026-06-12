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
#include <QDir>
#include <QMessageBox>

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
    
    QDir::setCurrent(fileInfo.absolutePath());
    
    if (fileInfo.exists() && fileInfo.isDir()) {
      QAction* action0 = new QAction(QIcon::fromTheme("folder"), tr("文件管理器打开"));
      connect(action0, &QAction::triggered, [=]() {
        QStringList args;
        args << url.path();
        if (!QProcess::startDetached("nemo", args)) {
          QMessageBox::information(nullptr, "提示", "无法启动 nemo，请检查是否已安装在系统路径中。");
          qWarning() << "无法启动 nemo，请检查是否已安装在系统路径中。";
        }
      });
      actions << action0;
    }
    
    QString menuText = tr("Ark压缩");
    bool isArchive = false;
    if (selectionUris.size() == 1 && fileInfo.exists() && fileInfo.isFile()) {
      QStringList archiveExtensions;
      archiveExtensions << "zip" << "tar" << "gz" << "bz2" << "xz" << "txz" << "zst" << "tzst" << "7z" << "rar" << "ar" << "arj" << "tgz" << "cab" << "deb" << "rpm" << "apk" << "iso" << "cpio";
      QString ext = fileInfo.suffix().toLower();
      if (archiveExtensions.contains(ext)) {
        isArchive = true;
        menuText = tr("Ark解压");
      }
    }
    QAction* action1 = new QAction(QIcon::fromTheme("package-x-generic"), menuText);
    connect(action1, &QAction::triggered, [=]() {
      if (url.path().isEmpty()) return;
      QString command = "ark";
      QStringList args;

      if (isArchive) {
        if (fileInfo.completeBaseName().toLower().endsWith(".tar")) {
          command = "tar";
          // 预览 tar 包内部文件列表
          QProcess* listProcess = new QProcess();
          listProcess->start(command, QStringList() << "-tf" << url.path());
          listProcess->waitForFinished();

          QString output = QString::fromUtf8(listProcess->readAllStandardOutput().trimmed());
          listProcess->deleteLater();

          QStringList lines = output.split('\n', QString::SkipEmptyParts);
          if (lines.isEmpty()) return;

          QString targetDir = ".";
          
          if (lines.size() > 1) {
            targetDir = fileInfo.completeBaseName();
            targetDir.chop(4);
            QDir().mkpath(targetDir);
          }
        
          args << "-xf" << url.path() << "-C" << targetDir;
          if (!QProcess::startDetached(command, args)) {
            QMessageBox::information(nullptr, "提示", "无法启动 ark，请检查是否已安装在系统路径中。");
            qWarning() << "无法启动 tar，请检查是否已安装在系统路径中。";
          }
        } else {
          // args << "-b" << "-a" << "-o" << fileInfo.absolutePath() << url.path();
          args << "-b" << "-a" << url.path();
          if (!QProcess::startDetached(command, args)) {
            QMessageBox::information(nullptr, "提示", "无法启动 ark，请检查是否已安装在系统路径中。");
            qWarning() << "无法启动 ark，请检查是否已安装在系统路径中。";
          }
        }
      } else {
        args << "-b" << "-f" << "tgz" << "-c";
        for (const QUrl &iurl : selectionUris) {
          args << iurl.path();
        }
        if (!QProcess::startDetached(command, args)) {
          QMessageBox::information(nullptr, "提示", "无法启动 ark，请检查是否已安装在系统路径中。");
          qWarning() << "无法启动 ark，请检查是否已安装在系统路径中。";
        }
      }
    });
    actions << action1;
    
    if (fileInfo.exists() && fileInfo.isFile()) {
      QIcon nddIcon = QIcon::fromTheme("ndd");
      if (nddIcon.isNull()) {
        nddIcon = QIcon::fromTheme("text-editor");
      }
      QAction* action2 = new QAction(nddIcon, tr("NotePad打开"));
      connect(action2, &QAction::triggered, [=]() {
        QStringList args;
        args << url.path();
        if (!QProcess::startDetached("notepad--", args)) {
          QMessageBox::information(nullptr, "提示", "无法启动 notepad--，请检查是否已安装在系统路径中。");
          qWarning() << "无法启动 notepad--，请检查是否已安装在系统路径中。";
        }
      });
      actions << action2;
    }
  }

  QIcon codeIcon = QIcon::fromTheme("vscode");
  if (codeIcon.isNull()) {
    codeIcon = QIcon::fromTheme("accessories-text-editor");
  }
  QAction* action3 = new QAction(codeIcon, tr("VsCode打开"));
  connect(action3, &QAction::triggered, [=]() {
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
  actions << action3;

  QAction* action4 = new QAction(QIcon::fromTheme("media-playback-start"), tr("动作"));
  actions << action4;
  QMenu* menu = new QMenu(action4->parentWidget());
  connect(action4, &QAction::destroyed, [=]() {
    qDebug() << "delete sub menu";
    menu->deleteLater();
  });
  QAction* subaction2 = menu->addAction("动作1");
  menu->addSeparator();
  QAction* subaction3 = menu->addAction("动作2");
  action4->setMenu(menu);
  connect(subaction2, &QAction::triggered, [=]() {
    qDebug() << "click sub action2";
    QProcess p(0);
    QString command = "echo";
    QStringList args;
    args.append("action2");
    p.execute(command, args);
  });
  connect(subaction3, &QAction::triggered, [=]() {
    qDebug() << "click sub action2";
    QProcess p(0);
    QString command = "echo";
    QStringList args;
    args.append("action3");
    p.execute(command, args);
  });
  // QAction* action4 = new QAction(QIcon::fromTheme("document-open"), tr("打开"));
  // connect(action4, &QAction::triggered, [=]() {
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
  // actions << action4;

  return actions;
}
