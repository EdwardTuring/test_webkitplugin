#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebView>

class Webview : public QWebView
{
    Q_OBJECT
public:
    explicit Webview(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // WEBVIEW_H
