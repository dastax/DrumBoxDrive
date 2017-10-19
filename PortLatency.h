#ifndef FIXLATENCY_H
#define FIXLATENCY_H

#include <QObject>
#include <QString>

/*
 * A simple class to manage latency reduction of FTDI-based serial ports. Implementation varies from
 * OS to OS. Implementation has to determine if port referred to by "portName" actually is
 * an FTDI device or not.
 *
 * Emits debug or status messages to indicate its current status
 *
 */
class PortLatency : public QObject {
  Q_OBJECT
public:
  PortLatency(QString portName);
  
  bool fixLatency();
  bool resetLatency();
  bool isSwitched() {return _isSwitched;}
  
signals:
  void debugMessage(QString message);
  void errorMessage(QString message);
private:
  QString _portName;
  bool _isSwitched;
  
  bool swapAsync(bool setAsyncMode);
};

#endif // FIXLATENCY_H
