#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QHash>
#include <QList>
#include <QString>

// 读写 config/setdb.txt 的键值配置（单行 "key,value;key,value;..." 格式）
class ConfigManager
{
public:
    ConfigManager();

    QString value(const QString &key, const QString &def = QString()) const;
    int     intValue(const QString &key, int def = 0) const;
    bool    boolValue(const QString &key, bool def = false) const;
    void    setValue(const QString &key, const QString &value);

    bool    load(const QString &path);
    bool    save(const QString &path) const;

    QString filePath() const { return path_; }
    static QString defaultFilePath();   // <appdir>/config/setdb.txt

    // 公司名（config/company.txt，中/繁/英 三语以 ; 分隔）
    static QStringList companyNames();

private:
    QString path_;
    QList<QString> keys_;              // 保持原文件顺序
    QHash<QString, QString> values_;
};

#endif // CONFIGMANAGER_H
