/***************************************************************************
 *  	   Copyright (C) 2011  Nicola Parisi <nparisi89@gmail.com>	   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "wattrate.h"
#include <QPainter>
#include <QFontMetrics>
#include <QSizeF>
#include <KLocale>
#include <KDebug>

#include <QtDBus>

#include <plasma/svg.h>
#include <plasma/theme.h>

#define UPOWER_SERVICE "org.freedesktop.UPower"
#define UPOWER_PATH "/org/freedesktop/UPower"

WattRate::WattRate(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
    m_svg(this),
    m_icon("preferences-system-power-management")
{
  
    qDBusRegisterMetaType<Data>();
    qDBusRegisterMetaType<DataList>();
    qDBusRegisterMetaType<StatsList>();
    qDBusRegisterMetaType<Stats>();
  
    // m_svg.setImagePath( patrh immagine);
    setHasConfigurationInterface(false);
    setAspectRatioMode( Plasma::IgnoreAspectRatio );
    
    resize(250, 50);
    
    //Ti butto tutto quello che ti interessa in var
    upower=new OrgFreedesktopUPowerInterface(UPOWER_SERVICE,UPOWER_PATH,QDBusConnection::systemBus());
    devices=new QList<OrgFreedesktopUPowerDeviceInterface*>;
    QList<QDBusObjectPath> pl= upower->EnumerateDevices();
    foreach (QDBusObjectPath p ,pl){
      devices->append(new OrgFreedesktopUPowerDeviceInterface(UPOWER_SERVICE,p.path(),QDBusConnection::systemBus()));
    }
    
}


WattRate::~WattRate()
{
    if (hasFailedToLaunch()) {
        // Do some cleanup here
    } else {
        // Save settings
    }
}

void WattRate::init()
{
    // A small demonstration of the setFailedToLaunch function
    if (m_icon.isNull()) {
        setFailedToLaunch(true, i18n("Errore avvio plasmoide"));
    }

    updateWattUsage();
    
    // kick off a refresh timer
    QTimer* m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateWattUsage()));
    m_timer->start(10000);
}


void WattRate::paintInterface(QPainter *p,
        const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
  Q_UNUSED(option)
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    // Now we draw the applet, starting with our svg
    m_svg.resize((int)contentsRect.width(), (int)contentsRect.height());
    m_svg.paint(p, (int)contentsRect.left(), (int)contentsRect.top());

    p->setPen(Qt::white);
    p->setFont(QFont("Tahoma", 8, QFont::Bold));
    p->drawText(contentsRect,
                Qt::AlignCenter | Qt::AlignLeft,
                wattString);
    
    // imposta un icona di sistema di una batteria a lato del valore.
    
    p->restore();
}

void WattRate::updateWattUsage()
{
  //Prendiamo la prima batteria ignorandone altre
    OrgFreedesktopUPowerDeviceInterface *d=0;
    foreach(OrgFreedesktopUPowerDeviceInterface *dev, *devices)
    {
      if(!d)
      {
	if(dev->type()==2)
	{ //Batteria
	  d=dev;
	}
	
      }
    }

    
    wattUsage	=	d->energyRate();
    
    if( wattUsage > 0 && d->timeToEmpty() > 0 )
    {
      //wattString	=	"Consumo attuale: " + QString::number( wattUsage , 'd' , 2 ) + "W\n"
	//		+	"Carica rimanente: " + QString::number( d->percentage() , 'i', 0 ) + "%";
		int hours = (d->timeToEmpty() / 60 / 60) % 24;
		int minutes = (d->timeToEmpty() / 60) % 60;
		wattString = QString::number( hours , 'i' , 0 ) + "h" + QString::number( minutes , 'i' , 0 ) + "m";
    }
    else
    {
		int hours = (d->timeToFull() / 60 / 60) % 24;
		int minutes = (d->timeToFull() / 60) % 60;
		wattString = "(" + QString::number( hours , 'i' , 0 ) + "h" + QString::number( minutes , 'i' , 0 ) + "m" + ")";
      //wattString	=	"Fonte AC in uso\nPercentuale ricarica: " + QString::number( d->percentage() , 'i' , 0 ) + "%";
	 // wattString = QString::number( d->timeToFull() , 'i' , 0 ) ;
    }   

    update();
}

# include "wattrate.moc"
