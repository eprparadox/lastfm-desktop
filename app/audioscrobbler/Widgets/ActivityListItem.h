/*
   Copyright 2010 Last.fm Ltd.
      - Primarily authored by Jono Cole and Michael Coffey

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ACTIVITY_LIST_ITEM_H
#define ACTIVITY_LIST_ITEM_H

#include <QDateTime>
#include <QDomElement>

#include "lib/unicorn/StylableWidget.h"

class ActivityListItem : public StylableWidget
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus);
    Q_PROPERTY(bool odd READ odd WRITE setOdd);

public:
    ActivityListItem( QWidget* parent = 0 );

    static ActivityListItem* fromElement( QDomElement element );
    virtual QDomElement toDomElement( QDomDocument xml ) const;

    virtual QWidget* infoWidget() const;

    bool odd() const;
    void setOdd( bool odd );

    QString status() const;
    void setStatus( QString status );

signals:
    void clicked( ActivityListItem* );
    void changed();

public slots:
    virtual void updateTimestamp();

private:
    void mousePressEvent( QMouseEvent * event );
    void resizeEvent( QResizeEvent* event );

protected:
    void setupUi();

    struct
    {
        class QLabel* as;
        class QWidget* trackTextArea;
        class QLabel* trackText;
        class QLabel* correction;
        class QLabel* love;
        class QToolButton* cog;
        class GhostWidget* ghostCog;
        class QLabel* timestamp;
    } ui;

    struct {
        class QMovie* scrobbler_as;
        class QMovie* scrobbler_paused;
    } movie;

    QAction* m_loveAction;

protected:
    QDateTime m_timestamp;
    class QTimer* m_timestampTimer;

private:
    QString m_status;
    bool m_odd;
};

#include <QMovie>

class Movie : public QMovie
{
Q_OBJECT
public:
    Movie( const QString& filename, const QByteArray& format = QByteArray(), QObject* parent = 0 )
    :QMovie( filename, format, parent )
    {
        connect( this, SIGNAL( frameChanged(int)), SLOT(onFrameChanged(int)));
    }

signals:
    void loopFinished();

private slots:
    void onFrameChanged( int f )
    {
        if( f == (frameCount() - 1)) {
            emit loopFinished();
        }
    }
};

#endif // ACTIVITY_LIST_ITEM_H