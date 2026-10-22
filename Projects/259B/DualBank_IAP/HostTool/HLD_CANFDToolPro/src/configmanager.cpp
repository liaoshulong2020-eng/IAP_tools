#include "configmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

ConfigManager::ConfigManager()
{
    path_ = defaultFilePath();
    if (QFile::exists(path_))
        load(path_);
}

QString ConfigManager::defaultFilePath()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/config/setdb.txt");
}

QString ConfigManager::value(const QString &key, const QString &def) const
{
    return values_.value(key, def);
}

int ConfigManager::intValue(const QString &key, int def) const
{
    const QString v = values_.value(key);
    bool ok = false;
    const int n = v.toInt(&ok);
    return ok ? n : def;
}

bool ConfigManager::boolValue(const QString &key, bool def) const
{
    const QString v = values_.value(key);
    if (v.isEmpty())
        return def;
    return v != QStringLiteral("0");
}

void ConfigManager::setValue(const QString &key, const QString &value)
{
    if (!values_.contains(key))
        keys_.append(key);
    values_[key] = value;
}

bool ConfigManager::load(const QString &path)
{
    path_ = path;
    keys_.clear();
    values_.clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    // 兼容 BOM
    QByteArray content = f.readAll();
    f.close();
    if (content.startsWith("\xEF\xBB\xBF"))
        content.remove(0, 3);

    const QString text = QString::fromUtf8(content).trimmed();
    // 格式：key,value;key,value;...
    const QStringList pairs = text.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QString &pair : pairs) {
        const int comma = pair.indexOf(QLatin1Char(','));
        if (comma <= 0)
            continue;
        const QString key = pair.left(comma).trimmed();
        const QString val = pair.mid(comma + 1);
        if (!values_.contains(key))
            keys_.append(key);
        values_[key] = val;
    }
    return true;
}

bool ConfigManager::save(const QString &path) const
{
    if (!path.isEmpty())
        const_cast<ConfigManager *>(this)->path_ = path;

    QFileInfo fi(path_);
    QDir dir = fi.dir();
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    QFile f(path_);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QStringList parts;
    for (const QString &key : keys_)
        parts << (key + QLatin1Char(',') + values_.value(key));
    const QByteArray data = parts.join(QLatin1Char(';')).toUtf8();

    f.write(data);
    f.close();
    return true;
}

QStringList ConfigManager::companyNames()
{
    QStringList names;
    const QString path = QCoreApplication::applicationDirPath()
                         + QStringLiteral("/config/company.txt");
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray content = f.readAll();
        f.close();
        if (content.startsWith("\xEF\xBB\xBF"))
            content.remove(0, 3);
        names = QString::fromUtf8(content).split(QLatin1Char(';'));
    }
    // 去掉首尾可能的空项
    while (!names.isEmpty() && names.first().trimmed().isEmpty())
        names.removeFirst();
    while (!names.isEmpty() && names.last().trimmed().isEmpty())
        names.removeLast();
    return names;
}
