#ifndef WEBPLUGINFACTORY_H
#define WEBPLUGINFACTORY_H
#include <QWebPluginFactory>
class WebPluginFactory : public QWebPluginFactory
{
public:
    WebPluginFactory(QObject * parent = 0);
};

#endif // WEBPLUGINFACTORY_H
