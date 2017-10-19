#include <QtCore>
#include "drumboxdrive.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Q_INIT_RESOURCE(resources_qjack);

  DrumBoxDrive dlg;
  QFile qss(":/qss");
  qss.open(QFile::ReadOnly);
  QString st=qss.readAll();
  //qDebug()<<st;
  a.setStyleSheet(st);
  qss.close();

  //dlg.show();
  return a.exec();
}
/*
int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Q_INIT_RESOURCE(resources_qjack);

  QFile qss(":/qss");
  qss.open(QFile::ReadOnly);
  QString st=qss.readAll();
  //qDebug()<<st;
  a.setStyleSheet(st);
  qss.close();

  DrumBoxDrive dlg();
  //dlg.show();
  return a.exec();
}

*/
