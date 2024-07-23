#include "QString.h"

#include <stdio.h>
#include <string.h>

QString::QString()
{

}

QString::QString(const QByteArray& srcArray)
{
    assign(srcArray.begin(), srcArray.end());
}

QString::QString(const std::string& srcString)
{
    assign(srcString.begin(), srcString.end());    
}

QStringList QString::split(const char* separator)
{
    char* str_ptr = data();
    char* foundName;

    QStringList answerList;

    while((foundName = strsep(&str_ptr, separator)) != NULL)
    {  
        std::string entry(foundName);
 
        if(entry.find(0xFF) == std::string::npos)
        {
            answerList.push_back(entry);
        }
    }

    return answerList;
}