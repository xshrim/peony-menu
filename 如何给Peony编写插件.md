# 如何给 Peony 写插件

Peony是优麒麟Kylin操作系统的文件管理器, 可以通过编写插件扩展其功能.

## 参考资料

[如何编写 Peony 插件](https://zhuanlan.zhihu.com/p/373677523)

[Peony-Qt的开发者手册（中文版）](https://github.com/Yue-Lan/peony-qt_development_document)

[Peony 的 wiki](https://github.com/ukui/peony/wiki)

## 下载编译

### peony

https://github.com/ukui/peony

```
sudo apt build-dep peony
```

~~这个装依赖我记得是不全的，编译时缺什么补什么吧~~

```cpp
git clone https://github.com/ukui/peony.git
cd peony && mkdir build && cd build
qmake .. && make
sudo make install
```

在peony项目中，有一些关于插件测试的子项目，是项目早期为了进行验证的时候加上的，比如 https://github.com/ukui/peony/tree/master/peony-qt-plugin-test。可作为参考。

在 libpeony-qt / plugin-iface 中，定义了插件接口，比如 MenuPluginInterface，PreviewPagePluginIface，在实现插件时需要继承这些接口。

在 controls / preview-page 中实现的默认预览窗口就是继承 PreviewPagePluginIface 实现的。

plugin-manager.cpp 可以看到插件是如何加载的。

### 插件

https://github.com/ukui/peony-extensions

同样下载编译好，不感兴趣的插件依赖没装好等原因不好编译的，在 [peony-extensions.pro](http://peony-extensions.pro/) 注释它就行。

再次打开 peony，应该已经可以加载插件了。

已经写好的插件是很好的参考。

## 简单示例

MenuPluginInterface，可以在右键菜单上添加功能项，应该是最常用的接口。

下面以设置壁纸的插件为例，当选中一个图片并右键呼出菜单时，会添加一个设置壁纸的选项。

1. 在 [peony-extensions.pro](http://peony-extensions.pro/) 中添加一个 SUBDIRS
2. .pro 文件

```haskell
QT       += widgets

TARGET = peony-set-wallpaper
TEMPLATE = lib

DEFINES += PEONYSETWALLPAPER_LIBRARY

include(../common.pri)

CONFIG += debug link_pkgconfig plugin

PKGCONFIG += peony gsettings-qt

TRANSLATIONS = translations/peony-set-wallpaper-extension_zh_CN.ts \
               translations/peony-set-wallpaper-extension_tr.ts

SOURCES += \
    set-wallpaper-plugin.cpp

HEADERS += \
        set-wallpaper-plugin.h \
        peony-set-wallpaper_global.h

unix {
    target.path = $$[QT_INSTALL_LIBS]/peony-extensions
    INSTALLS += target
}

RESOURCES += \
    peony-set-wallpaper.qrc
```

可以看到 include(../common.pri) 是规定了插件版本，如果 peony 版本与插件不一致，就会拒绝加载。这个和其他项目一致就行，不需要改。

PKGCONFIG += peony，可以使用 libpeony-dev 中的函数了。注意外部只能调用有PEONYCORESHARED_EXPORT 声明的类

1. peony-set-wallpaper_global.h

```cpp
#ifndef PEONYQTSETWALLPAPER_GLOBAL_H
#define PEONYQTSETWALLPAPER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PEONYSETWALLPAPER_LIBRARY)
#  define PEONYQTSETWALLPAPERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PEONYQTSETWALLPAPERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PEONYQTSETWALLPAPER_GLOBAL_H
```

即使没用过 Qt 插件体系，这些东西可以先照着抄，改改名字就行。

Q_DECL_EXPORT  // 共享库项目

Q_DECL_IMPORT  // 使用共享库的客户项目

这个 PEONYSETWALLPAPER_LIBRARY 在 .pro 中定义了。

1. set-wallpaper-plugin.h

```cpp
#ifndef SETWALLPAPERPLUGIN_H
#define SETWALLPAPERPLUGIN_H

#include "peony-set-wallpaper_global.h"
#include <menu-plugin-iface.h>

#include <QGSettings>

namespace Peony {

class PEONYQTSETWALLPAPERSHARED_EXPORT SetWallPaperPlugin: public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit SetWallPaperPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {return PluginInterface::MenuPlugin;}
    const QString name() override {return tr("Peony-Qt set wallpaper Extension");}
    const QString description() override {return tr("set wallpaper Extension.");}
    const QIcon icon() override {return QIcon::fromTheme("");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}

    QString testPlugin() override {return "test set wallpaper";}
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;

    bool is_picture_file(QString file_name);

private:
    bool m_enable;
    QStringList m_picture_type_list = {"png", "jpg", "jpeg", "bmp", "dib", "jfif", "jpe", "gif", "tif", "tiff", "wdp"};

    QGSettings *m_bg_settings = nullptr;
};

}

#endif // SETWALLPAPERPLUGIN_H
```

这里面插件机制相关的代码，就是换个名字的事，各个插件都是类似的。

重载自 MenuPluginInterface 的 QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) 是实现功能的关键。

```cpp
QList<QAction*> SetWallPaperPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction*> actions;
    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow)
    {
        if (selectionUris.count() == 1 && is_picture_file(selectionUris.first())) {
            if (selectionUris.first().contains("trash:///"))
                return actions;
            QAction *set_action = new QAction(tr("Set as wallpaper"), nullptr);
            actions<<set_action;
            connect(set_action, &QAction::triggered, [=](){
                if (QGSettings::isSchemaInstalled(BACKGROUND_SETTINGS))
                {
                   m_bg_settings = new QGSettings(BACKGROUND_SETTINGS, QByteArray(), this);
                   QUrl url= selectionUris.first();
                   bool success = m_bg_settings->trySet("pictureFilename", url.path());
                   qDebug() << "set as wallpaper result:" <<success <<url.path();
                }
            });
        }
    }
    return actions;
}
```

其他类型的插件也是类似的，实现相应接口即可。

## 附录笔记

https://zany-yoke-295.notion.site/Peony-07be443c7e314f17b021056c380ccaaa