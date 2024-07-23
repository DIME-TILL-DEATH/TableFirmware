#ifndef FOLDERCONTENTMESSAGE_H
#define FOLDERCONTENTMESSAGE_H

#include "utils/qt_compat.h"

#include "abstractmessage.h"

class FolderContentMessage : public AbstractMessage
{
public:
    FolderContentMessage();

    FolderContentMessage(const QString& dstPath, const QStringList& folderContent);
    FolderContentMessage(const QByteArray& recievedData);
    ~FolderContentMessage();

    QString path() const {return m_path;};
    QStringList files() const {return m_files;};

private:
    QString m_path;
    QStringList m_files;
};

#endif // FOLDERCONTENTMESSAGE_H
