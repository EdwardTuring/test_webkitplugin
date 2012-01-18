#include <QtGui>
#include <QtCore>
#include "webview.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;
    Webview *view=new Webview(&w);
    QUrl url("http://127.0.0.1/test");
    view->load(url);
    w.setCentralWidget(view);
    w.show();

    return a.exec();
}
