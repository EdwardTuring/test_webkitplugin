#ifndef WEBPLUGINFACTORY_H
#define WEBPLUGINFACTORY_H
#include <QtCore>
#include <QWebPluginFactory>
class WebPluginFactory : public QWebPluginFactory
{
public:
    WebPluginFactory(QObject * parent = 0);
    virtual QObject *	create ( const QString & mimeType,
                                 const QUrl & url,
                                 const QStringList & argumentNames,
                                 const QStringList & argumentValues ) const;

    virtual QList<WebPluginFactory::Plugin>	plugins () const ;

};

#endif // WEBPLUGINFACTORY_H
