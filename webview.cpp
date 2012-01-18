#include "webview.h"
#include "webpluginfactory.h"
Webview::Webview(QWidget *parent) :
    QWebView(parent)
{
     this->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled,true);
     this->page()->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
     this->page()->setPluginFactory(new WebPluginFactory(this));
}
