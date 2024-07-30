#include "filepartmessage.h"


FilePartMessage::FilePartMessage(ActionType actionType, const QString& dstPath, const QString& srcPath, const QByteArray& filePart, int32_t partPosition)
    : AbstractMessage(FrameType::UNDEFINED, (uint8_t)Requests::File::FILE_CREATE),
    m_filePart(filePart),
    m_dstPath(dstPath),
    m_srcPath(srcPath)
{
    m_messageType = AbstractMessage::FILE_PART;

    long fileSize = -1;
    if(partPosition != -1)
    {
        FILE* reqFile = fopen(srcPath.c_str(), "r");
        if(reqFile)
        {
            fseek(reqFile, 0 , SEEK_END);
            fileSize = ftell(reqFile);
            fclose(reqFile);
        }
       

        // QFile file(srcPath);
        // fileSize = file.size();
    }
    m_frameHeader.structData.data0 = partPosition;
    m_frameHeader.structData.data1 = fileSize;

    switch(actionType)
    {
        case ActionType::UNDEFINED: break;
        case ActionType::CREATE_TEXT:
        {
            m_frameHeader.structData.frameType = FrameType::FILE_ACTIONS;
            m_frameHeader.structData.action = (uint8_t)Requests::File::FILE_CREATE;
            break;
        }
        case ActionType::APPEND_TEXT:
        {
            m_frameHeader.structData.frameType = FrameType::FILE_ACTIONS;
            m_frameHeader.structData.action = (uint8_t)Requests::File::FILE_APPEND_DATA;
            break;
        }
        case ActionType::CREATE_FIRMWARE:
        {
            m_frameHeader.structData.frameType = FrameType::FIRMWARE_ACTIONS;
            m_frameHeader.structData.action = (uint8_t)Requests::Firmware::FIRMWARE_UPLOAD_START;
            break;
        }
        case ActionType::APPEND_FIRMWARE:
        {
            m_frameHeader.structData.frameType = FrameType::FIRMWARE_ACTIONS;
            m_frameHeader.structData.action = (uint8_t)Requests::Firmware::FIRMWARE_UPLOAD_PROCEED;
            break;
        }
        case ActionType::GET_FILE:
        {
            m_frameHeader.structData.frameType = FrameType::FILE_ACTIONS;
            m_frameHeader.structData.action = (uint8_t)Requests::File::GET_FILE;
            break;            
        }
    }

    m_frameHeader.structData.frameParameters = dstPath.size();
    m_frameHeader.structData.frameSize = sizeof(FrameHeader) + dstPath.size() + filePart.size();

    m_rawData.append(m_frameHeader.rawData, sizeof(FrameHeader));
    // m_rawData.append(dstPath.toLocal8Bit());
    m_rawData.append(dstPath.c_str(), dstPath.size());
    m_rawData.append(filePart);
}

FilePartMessage::FilePartMessage(const QByteArray& recievedData)
    : AbstractMessage(recievedData)
{
    m_messageType = AbstractMessage::FILE_PART;

    m_srcPath = m_rawData.mid(sizeof(FrameHeader), m_frameHeader.structData.frameParameters);
    m_dstPath = m_srcPath;
    // m_srcPath.remove(DeviceContentModel::librarySdcardPath);

    m_partPosition = static_cast<int32_t>(m_frameHeader.structData.data0);
    m_partSize = m_frameHeader.structData.frameSize - sizeof(FrameHeader) - m_frameHeader.structData.frameParameters;

    m_fileSize = static_cast<int32_t>(m_frameHeader.structData.data1);

    m_filePart = m_rawData.mid(sizeof(FrameHeader) + m_frameHeader.structData.frameParameters, m_partSize);
}

FilePartMessage::~FilePartMessage()
{

}
