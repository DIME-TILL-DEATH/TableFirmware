#include "foldercontentmessage.h"

FolderContentMessage::FolderContentMessage(const QString& dstPath, const QStringList& folderContent)
    : AbstractMessage(FrameType::FILE_ACTIONS, (uint8_t)Requests::File::GET_FOLDER_CONTENT),
      m_files(folderContent)
{
    m_messageType = AbstractMessage::FOLDER_CONTENT;

    QByteArray contentArray;
    for(auto it = folderContent.begin(); it != folderContent.end(); ++it)
    {
        contentArray.append((*it).c_str(), (*it).size());
        contentArray.append("\r", 1);
    }    

    m_frameHeader.structData.frameSize = sizeof(FrameHeader) + dstPath.size() + contentArray.size();
    m_frameHeader.structData.frameParameters = dstPath.size();

    m_rawData.append(m_frameHeader.rawData, sizeof(FrameHeader));
    m_rawData.append(dstPath.c_str(), dstPath.size()); //m_rawData.append(dstPath.toLocal8Bit());
    m_rawData.append(contentArray);
}

FolderContentMessage::FolderContentMessage(const QByteArray &recievedData)
    : AbstractMessage(recievedData)
{
    m_messageType = AbstractMessage::FOLDER_CONTENT;

    m_path =  m_rawData.mid(sizeof(FrameHeader), m_frameHeader.structData.frameParameters);

    uint32_t contentSize = m_frameHeader.structData.frameSize - sizeof(FrameHeader) - m_frameHeader.structData.frameParameters;
    QString content = m_rawData.mid(sizeof(FrameHeader) + m_frameHeader.structData.frameParameters, contentSize);

    m_files = content.split("\r");
}

FolderContentMessage::~FolderContentMessage()
{

}


