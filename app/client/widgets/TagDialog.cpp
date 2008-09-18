/***************************************************************************
 *   Copyright 2005-2008 Last.fm Ltd.                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "TagDialog.h"
#include "lib/types/User.h"

#include "lib/unicorn/widgets/SpinnerLabel.h"
#include "widgets/TagListWidget.h"
#include "widgets/MooseTabWidget.h"
#include "widgets/TrackWidget.h"

#include <QtCore>
#include <QtGui>


TagDialog::TagDialog( QWidget *parent )
        : QDialog( parent, Qt::Dialog )
{
    setupUi();
    setWindowTitle( tr("Tag") );

    WsReply* r = AuthenticatedUser().getTopTags();
    ui.yourTags->setTagsRequest( r );
    follow( r );
	
    QRegExp rx( "[a-zA-Z0-9\\-:,' ]{1,255}" );
    ui.edit->setValidator( new QRegExpValidator( rx, this ) );
    ui.edit->setCompleter( new QCompleter( ui.suggestedTags->model() ) );

    ui.buttons->button( QDialogButtonBox::Ok )->setText( tr("Tag") );
    ui.buttons->button( QDialogButtonBox::Ok )->setEnabled( false );

    connect( ui.edit, SIGNAL(returnPressed()), ui.add, SLOT(animateClick()) );
    connect( ui.add, SIGNAL(clicked()), SLOT(onAddClicked()) );

    connect( ui.suggestedTags, SIGNAL(itemActivated( QTreeWidgetItem*, int )), SLOT(onTagActivated( QTreeWidgetItem* )) );
    connect( ui.yourTags, SIGNAL(itemActivated( QTreeWidgetItem*, int )), SLOT(onTagActivated( QTreeWidgetItem* )) );
    
    connect( ui.buttons, SIGNAL(accepted()), SLOT(accept()) );
    connect( ui.buttons, SIGNAL(rejected()), SLOT(reject()) );
}


void
TagDialog::setupUi()
{
    QPalette p = palette();
    p.setBrush( QPalette::Window, QColor( 0x18, 0x18, 0x19 ) );
    p.setBrush( QPalette::WindowText, QColor( 0xff, 0xff, 0xff, 40 ) );
    setPalette( p );
    
    ui.tabs1 = new Unicorn::TabWidget;
    ui.tabs1->addTab( tr("Track"), ui.trackTags = new TagIconView );
    ui.tabs1->addTab( tr("Artist"), ui.artistTags = new TagIconView );
    ui.tabs1->addTab( tr("Album"), ui.albumTags = new TagIconView );
    
    ui.tabs2 = new Unicorn::TabWidget;
    ui.tabs2->addTab( tr("Suggested Tags"), ui.suggestedTags = new TagListWidget );
    ui.tabs2->addTab( tr("Your Tags"), ui.yourTags = new TagListWidget );
    
    QHBoxLayout* h2 = new QHBoxLayout;
    h2->addWidget( ui.edit = new QLineEdit );
    h2->addWidget( ui.add = new QPushButton( tr("Add") ) );

    QVBoxLayout* v = new QVBoxLayout( this );
    v->addWidget( ui.track = new TrackWidget );
    v->addLayout( h2 );
    v->addWidget( ui.tabs1 );
    v->addWidget( ui.tabs2 );
    v->addWidget( ui.buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) );
    
    ui.track->layout()->addWidget( ui.spinner = new SpinnerLabel );
    ui.spinner->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
}


void
TagDialog::follow( WsReply* r )
{
    r->setParent( this );
    connect( r, SIGNAL(finished( WsReply* )), SLOT(onWsFinished( WsReply* )) );
    m_activeRequests += r;
    ui.spinner->show();
}


void
TagDialog::setTrack( const Track& track )
{
    m_track = track;

    ui.track->setTrack( track );

    ui.tabs1->setTabEnabled( 0, !track.isNull() );
    ui.tabs1->setTabEnabled( 1, !track.artist().isNull() );
    ui.tabs1->setTabEnabled( 2, !track.album().isNull() );

    WsReply* r;
    follow( r = track.getTopTags() );
    ui.suggestedTags->setTagsRequest( r );
    follow( r = track.getTags() );
    ui.trackTags->setTagsRequest( r );
    follow( r = track.artist().getTags() );
    ui.artistTags->setTagsRequest( r );
    follow( r = track.album().getTags() );
    ui.albumTags->setTagsRequest( r );
}


void
TagDialog::accept()
{
    m_track.addTags( ui.trackTags->newTags() );
    m_track.artist().addTags( ui.artistTags->newTags() );
    m_track.album().addTags( ui.albumTags->newTags() );
    QDialog::accept();
}


void
TagDialog::onWsFinished( WsReply *r )
{
    m_activeRequests.removeAll( r );
    ui.spinner->setVisible( m_activeRequests.size() );
}


void
TagDialog::onTagActivated( QTreeWidgetItem *item )
{
    QString const newtag = item->text( 0 ).trimmed();
    currentTagListWidget()->add( newtag );
}


void
TagDialog::onAddClicked()
{
    if (currentTagListWidget()->add( ui.edit->text() ))
    {
        ui.buttons->button( QDialogButtonBox::Ok )->setEnabled( true );
        ui.edit->clear();
    }
    else
        QApplication::beep(); //TODO visually highlight the already entered one
}


TagListWidget*
TagDialog::currentTagListWidget() const
{
    return static_cast<TagListWidget*>(ui.tabs1->currentWidget());
}
