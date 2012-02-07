#include <QtGui>
 #include <QtNetwork>

 #include "ftpwindow.h"

 FtpWindow::FtpWindow(QWidget *parent)
     : QDialog(parent), ftp(0), networkSession(0)
 {
     ftpServerLabel = new QLabel(tr("Ftp &server:"));
     ftpServerLineEdit = new QLineEdit("");
     ftpServerLabel->setBuddy(ftpServerLineEdit);

     statusLabel = new QLabel(tr("Please enter the name of an FTP server."));


     fileList = new QTreeWidget;
     fileList->setEnabled(false);
     fileList->setRootIsDecorated(false);
     fileList->setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
     fileList->header()->setStretchLastSection(false);

     connectButton = new QPushButton(tr("Connect"));
     connectButton->setDefault(true);

     cdToParentButton = new QPushButton;
     cdToParentButton->setIcon(QPixmap(":/image/cdtoparent.png"));
     cdToParentButton->setEnabled(false);

     downloadButton = new QPushButton(tr("Download"));
     downloadButton->setEnabled(false);



     buttonBox = new QDialogButtonBox;
     buttonBox->addButton(downloadButton, QDialogButtonBox::ActionRole);

     progressDialog = new QProgressDialog(this);

     connect(fileList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
             this, SLOT(processItem(QTreeWidgetItem*,int)));
     connect(fileList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(enableDownloadButton()));
     connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
     connect(connectButton, SIGNAL(clicked()), this, SLOT(connectOrDisconnect()));
     connect(cdToParentButton, SIGNAL(clicked()), this, SLOT(cdToParent()));
     connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadFile()));


     QHBoxLayout *topLayout = new QHBoxLayout;
     topLayout->addWidget(ftpServerLabel);
     topLayout->addWidget(ftpServerLineEdit);
 #ifndef Q_OS_SYMBIAN
     topLayout->addWidget(cdToParentButton);
     topLayout->addWidget(connectButton);
 #else
     // Make app better lookin on small screen
     QHBoxLayout *topLayout2 = new QHBoxLayout;
     topLayout2->addWidget(cdToParentButton);
     topLayout2->addWidget(connectButton);
 #endif

     QVBoxLayout *mainLayout = new QVBoxLayout;
     mainLayout->addLayout(topLayout);
 #ifdef Q_OS_SYMBIAN
     // Make app better lookin on small screen
     mainLayout->addLayout(topLayout2);
 #endif
     mainLayout->addWidget(fileList);
     mainLayout->addWidget(statusLabel);
     mainLayout->addWidget(buttonBox);
     setLayout(mainLayout);

     setWindowTitle(tr("FTP"));
 }

 QSize FtpWindow::sizeHint() const
 {
     return QSize(500, 300);
 }

 void FtpWindow::connectOrDisconnect()
 {
     if (ftp) {
         ftp->abort();
         ftp->deleteLater();
         ftp = 0;
         fileList->setEnabled(false);
         cdToParentButton->setEnabled(false);
         downloadButton->setEnabled(false);
         connectButton->setEnabled(true);
         connectButton->setText(tr("Connect"));
 #ifndef QT_NO_CURSOR
         setCursor(Qt::ArrowCursor);
 #endif
         statusLabel->setText(tr("Please enter the name of an FTP server."));
         return;
     }

 #ifndef QT_NO_CURSOR
     setCursor(Qt::WaitCursor);
 #endif

     if (!networkSession || !networkSession->isOpen()) {
         if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
             if (!networkSession) {
                 // Get saved network configuration
                 QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
                 settings.beginGroup(QLatin1String("QtNetwork"));
                 const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
                 settings.endGroup();

                 // If the saved network configuration is not currently discovered use the system default
                 QNetworkConfiguration config = manager.configurationFromIdentifier(id);
                 if ((config.state() & QNetworkConfiguration::Discovered) !=
                     QNetworkConfiguration::Discovered) {
                     config = manager.defaultConfiguration();
                 }

                 networkSession = new QNetworkSession(config, this);
                 connect(networkSession, SIGNAL(opened()), this, SLOT(connectToFtp()));
                 connect(networkSession, SIGNAL(error(QNetworkSession::SessionError)), this, SLOT(enableConnectButton()));
             }
             connectButton->setEnabled(false);
             statusLabel->setText(tr("Opening network session."));
             networkSession->open();
             return;
         }
     }
     connectToFtp();
 }

 void FtpWindow::connectToFtp()
 {
     ftp = new QFtp(this);
     connect(ftp, SIGNAL(commandFinished(int,bool)),
             this, SLOT(ftpCommandFinished(int,bool)));
     connect(ftp, SIGNAL(listInfo(QUrlInfo)),
             this, SLOT(addToList(QUrlInfo)));
     connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
             this, SLOT(updateDataTransferProgress(qint64,qint64)));

     fileList->clear();
     currentPath.clear();
     isDirectory.clear();

     QUrl url(ftpServerLineEdit->text());
     if (!url.isValid() || url.scheme().toLower() != QLatin1String("ftp")) {
         ftp->connectToHost(ftpServerLineEdit->text(), 21);
         ftp->login();
     } else {
         ftp->connectToHost(url.host(), url.port(21));

         if (!url.userName().isEmpty())
             ftp->login(QUrl::fromPercentEncoding(url.userName().toLatin1()), url.password());
         else
             ftp->login();
         if (!url.path().isEmpty())
             ftp->cd(url.path());
     }

     fileList->setEnabled(true);
     connectButton->setEnabled(false);
     connectButton->setText(tr("Disconnect"));
     statusLabel->setText(tr("Connecting to FTP server %1...")
                          .arg(ftpServerLineEdit->text()));
 }

 void FtpWindow::downloadFile()
 {
     QString fileName = fileList->currentItem()->text(0);
 //
     if (QFile::exists(fileName)) {
         QMessageBox::information(this, tr("FTP"),
                                  tr("There already exists a file called %1 in "
                                     "the current directory.")
                                  .arg(fileName));
         return;
     }

     file = new QFile(fileName);
     if (!file->open(QIODevice::WriteOnly)) {
         QMessageBox::information(this, tr("FTP"),
                                  tr("Unable to save the file %1: %2.")
                                  .arg(fileName).arg(file->errorString()));
         delete file;
         return;
     }

     ftp->get(fileList->currentItem()->text(0), file);

     progressDialog->setLabelText(tr("Downloading %1...").arg(fileName));
     downloadButton->setEnabled(false);
     progressDialog->exec();
 }

 void FtpWindow::cancelDownload()
 {
     ftp->abort();

     if (file->exists()) {
         file->close();
         file->remove();
     }
     delete file;
 }

 void FtpWindow::ftpCommandFinished(int, bool error)
 {
 #ifndef QT_NO_CURSOR
     setCursor(Qt::ArrowCursor);
 #endif

     if (ftp->currentCommand() == QFtp::ConnectToHost) {
         if (error) {
             QMessageBox::information(this, tr("FTP"),
                                      tr("Unable to connect to the FTP server "
                                         "at %1. Please check that the host "
                                         "name is correct.")
                                      .arg(ftpServerLineEdit->text()));
             connectOrDisconnect();
             return;
         }
         statusLabel->setText(tr("Logged onto %1.")
                              .arg(ftpServerLineEdit->text()));
         fileList->setFocus();
         downloadButton->setDefault(true);
         connectButton->setEnabled(true);
         return;
     }

     if (ftp->currentCommand() == QFtp::Login)
         ftp->list();

     if (ftp->currentCommand() == QFtp::Get) {
         if (error) {
             statusLabel->setText(tr("Canceled download of %1.")
                                  .arg(file->fileName()));
             file->close();
             file->remove();
         } else {
             statusLabel->setText(tr("Downloaded %1 to current directory.")
                                  .arg(file->fileName()));
             file->close();
         }
         delete file;
         enableDownloadButton();
         progressDialog->hide();
     } else if (ftp->currentCommand() == QFtp::List) {
         if (isDirectory.isEmpty()) {
             fileList->addTopLevelItem(new QTreeWidgetItem(QStringList() << tr("<empty>")));
             fileList->setEnabled(false);
         }
     }
 }

 void FtpWindow::addToList(const QUrlInfo &urlInfo)
 {
     QTreeWidgetItem *item = new QTreeWidgetItem;
     item->setText(0, urlInfo.name());
     item->setText(1, QString::number(urlInfo.size()));
     item->setText(2, urlInfo.owner());
     item->setText(3, urlInfo.group());
     item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));

     QPixmap pixmap(urlInfo.isDir() ? ":/image/dir.png" : ":/image/file.png");
     item->setIcon(0, pixmap);

     isDirectory[urlInfo.name()] = urlInfo.isDir();
     fileList->addTopLevelItem(item);
     if (!fileList->currentItem()) {
         fileList->setCurrentItem(fileList->topLevelItem(0));
         fileList->setEnabled(true);
     }
 }

 void FtpWindow::processItem(QTreeWidgetItem *item, int /*column*/)
 {
     QString name = item->text(0);
     if (isDirectory.value(name)) {
         fileList->clear();
         isDirectory.clear();
         currentPath += '/';
         currentPath += name;
         ftp->cd(name);
         ftp->list();
         cdToParentButton->setEnabled(true);
 #ifndef QT_NO_CURSOR
         setCursor(Qt::WaitCursor);
 #endif
         return;
     }
 }

 void FtpWindow::cdToParent()
 {
 #ifndef QT_NO_CURSOR
     setCursor(Qt::WaitCursor);
 #endif
     fileList->clear();
     isDirectory.clear();
     currentPath = currentPath.left(currentPath.lastIndexOf('/'));
     if (currentPath.isEmpty()) {
         cdToParentButton->setEnabled(false);
         ftp->cd("/");
     } else {
         ftp->cd(currentPath);
     }
     ftp->list();
 }

 void FtpWindow::updateDataTransferProgress(qint64 readBytes,
                                            qint64 totalBytes)
 {
     progressDialog->setMaximum(totalBytes);
     progressDialog->setValue(readBytes);
 }

 void FtpWindow::enableDownloadButton()
 {
     QTreeWidgetItem *current = fileList->currentItem();
     if (current) {
         QString currentFile = current->text(0);
         downloadButton->setEnabled(!isDirectory.value(currentFile));
     } else {
         downloadButton->setEnabled(false);
     }
 }

 void FtpWindow::enableConnectButton()
 {
     // Save the used configuration
     QNetworkConfiguration config = networkSession->configuration();
     QString id;
     if (config.type() == QNetworkConfiguration::UserChoice)
         id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
     else
         id = config.identifier();

     QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
     settings.beginGroup(QLatin1String("QtNetwork"));
     settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
     settings.endGroup();

     connectButton->setEnabled(true);
     statusLabel->setText(tr("Please enter the name of an FTP server."));
 }
