#ifndef FILEPARTMESSAGE_H
#define FILEPARTMESSAGE_H

#include "abstractmessage.h"

#include "utils/qt_compat.h"

class FilePartMessage : public AbstractMessage
{
public:
    enum class ActionType
    {
        UNDEFINED,
        CREATE_TEXT,
        APPEND_TEXT,
        CREATE_FIRMWARE,
        APPEND_FIRMWARE,
        GET_FILE
    };

    FilePartMessage(ActionType actionType, const QString& dstPath, const QString& srcPath, const QByteArray& filePart = {}, int32_t partPosition = 0);
    FilePartMessage(const QByteArray& recievedData);
    ~FilePartMessage();

    QString srcPath() const {return m_srcPath;};
    QString dstPath() const {return m_dstPath;};

    uint32_t fileSize() const {return m_fileSize;};
    uint32_t partSize() const {return m_partSize;};
    uint32_t partPosition() const {return m_partPosition;};

    QByteArray filePart() const {return m_filePart;};

    static constexpr int32_t defaultPartSize = 1024 * 8;

private:
    ActionType m_actionType{ActionType::UNDEFINED};

    QByteArray m_filePart;

    QString m_dstPath;
    QString m_srcPath;
    uint32_t m_fileSize{0};
    uint32_t m_partPosition{0};
    uint32_t m_partSize{defaultPartSize};
};

#endif // FILEPARTMESSAGE_H
