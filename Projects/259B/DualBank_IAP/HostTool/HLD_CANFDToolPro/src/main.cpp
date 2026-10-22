#include "mainwindow.h"
#include "modelitem.h"
#include "configmanager.h"

#include <QApplication>
#include <QMetaType>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QDir>

// 语言文件名 -> 程序内语言标识
static QString languageFileFromSetting(const QString &setting)
{
    // setdb.txt 的 Language 项存的是 "CN.qm" / "EN.qm" / "CNF.qm" 之类的文件名
    if (setting.isEmpty())
        return QString();
    if (setting.contains("CNF", Qt::CaseInsensitive))
        return QStringLiteral("CNF.qm");
    if (setting.contains("EN", Qt::CaseInsensitive))
        return QStringLiteral("EN.qm");
    if (setting.contains("CN", Qt::CaseInsensitive))
        return QStringLiteral("CN.qm");
    return setting;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("HLD_CANFDToolPro"));
    app.setApplicationDisplayName(QStringLiteral("HLD_CANFDToolPro v1.1 双Bank版"));
    app.setApplicationVersion(QStringLiteral("1.0"));
    app.setOrganizationName(QStringLiteral("HLD"));
    app.setOrganizationName(QStringLiteral("Zhuhai ChuangXin Tec.co"));

    // 读取语言设置，加载对应翻译
    ConfigManager cfg;
    QString langFile = languageFileFromSetting(cfg.value(QStringLiteral("Language")));

    static QTranslator translator;
    static QTranslator qtTranslator;
    if (!langFile.isEmpty()) {
        const QString langDir = QCoreApplication::applicationDirPath() + QStringLiteral("/language/");
        if (translator.load(langDir + langFile))
            app.installTranslator(&translator);
        // 加载 Qt 自带翻译（可选）
        if (qtTranslator.load(QLocale::system(), QStringLiteral("qt"), QStringLiteral("_"),
                              langDir, QStringLiteral(".qm")))
            app.installTranslator(&qtTranslator);
    }

    qRegisterMetaType<QList<ModelItem>>("QList<ModelItem>");

    MainWindow w;
    w.show();
    return app.exec();
}
