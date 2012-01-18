#include "webpluginfactory.h"
#include <QTextEdit>
#include <QtCore>
WebPluginFactory::WebPluginFactory(QObject *parent)
{

    qDebug()<<"WebPluginFactory is created";
}
//The returned object should be a QWidget
 QObject *	WebPluginFactory::create ( const QString & mimeType,
                             const QUrl & url,
                             const QStringList & argumentNames,
                             const QStringList & argumentValues ) const
{

     if(QString::compare(mimeType,"UI_App/textedit",Qt::CaseInsensitive)==0){

            qDebug()<<"works";
         return new QTextEdit(qobject_cast<QWidget *>(this->parent()));

     }
    return NULL;
}
//Currently, this function is only called when JavaScript programs access the global plugins or mimetypes objects.
  QList<WebPluginFactory::Plugin>	WebPluginFactory::plugins () const
{
      QList<WebPluginFactory::Plugin> plugins;

      return plugins;
}
