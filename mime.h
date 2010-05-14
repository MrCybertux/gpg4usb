/*
 *      mime.h
 *
 *      Copyright 2008 gpg4usb-team <gpg4usb@cpunk.de>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __MIME_H__
#define __MIME_H__

#include <QByteArray>
#include <QHash>
#include <QDebug>


class HeadElem {
public:
    QString name;
    QString value;
    QHash<QString, QString> params;

/*    QDataStream & operator<<(QDataStream& Stream, const HeadElem& H)
    {
        return Stream << H.name << " : " << H.value;
    }*/

};

class MimePart {
public:
    QList<HeadElem> header;
    QByteArray body;

    QString getValue(QString key){
        foreach(HeadElem tmp, header) {
            //qDebug() << "gv: " << tmp.name << ":" << tmp.value;
            if(tmp.name == key)
                return tmp.value;
        }
        return "";
    }

    QHash<QString, QString> getParams(QString key){
        foreach(HeadElem tmp, header) {
            //qDebug() << "gv: " << tmp.name << ":" << tmp.value;
            if(tmp.name == key)
                //return tmp.value;
                return tmp.params;
        }
        return *(new QHash<QString, QString>());
    }

     QString getParam(QString key, QString pKey){
        foreach(HeadElem tmp, header) {
            //qDebug() << "gv: " << tmp.name << ":" << tmp.value;
            if(tmp.name == key)
                return tmp.params.value(pKey);
        }
        return "";
    }

/*    QDataStream & operator<<(QDataStream& Stream, const Part& P)
    {
        foreach(HeadElem tmp, header) {
            Stream << tmp << "\n";
        }
        return Stream;
    }*/
};

class Mime
{

public:
    Mime(QByteArray *message); // Constructor
    ~Mime(); // Destructor
    static bool isMultipart(QByteArray *message);
    QList<MimePart> parts()
      { return mPartList; }
    void splitParts(QByteArray *message);
    QList<HeadElem> parseHeader(QByteArray *header);
    static void quotedPrintableDecode(const QByteArray& in, QByteArray& out);

private:
    QByteArray *mMessage;
    QByteArray *mBoundary;
    QList<MimePart> mPartList;

};

#endif  // __MIME_H__
