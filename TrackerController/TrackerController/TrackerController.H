#ifndef TRACKERCONTROLLER_H
#define TRACKERCONTROLLER_H

#include <QMainWindow>
#include <QVector>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QUdpSocket>

#include <amjPacket.H>

#include "../../FringeTracker/include/FringeTrackerStateMachine.H"

#define SLIDER_MIN 0
#define SLIDER_MAX 1000
#define SNR_MIN 1
#define SNR_MAX 300

#define TRACKER_ADDRESS "127.0.0.1"
#define TRACKER_PORT 27010

QT_BEGIN_NAMESPACE
namespace Ui { class TrackerController; }
QT_END_NAMESPACE

class TrackerController : public QMainWindow
{
  Q_OBJECT

public:
  TrackerController(QWidget *parent = nullptr);
  ~TrackerController();
public slots:
  void slider(int);
  void ftsm_ui();
private:
  Ui::TrackerController *ui;
  QVector<QRadioButton *> baselineButtons;
  QVector<QCheckBox *> baselineCheckBoxes;
  QVector<QLineEdit *> baselineFoundSNRs;
  QVector<QLineEdit *> baselineLostSNRs;
  QUdpSocket *ftsm; // connection to fringe tracker state machines
  QHostAddress address;
  int port;
  std::vector<std::string> names;
  std::vector<FringeTrackerStateMachineConfig> smcs;
  void ftsm_write();
  void ftsm_send();
  void ftsm_receive();
  void updateNames(std::vector<std::string>);
};
#endif // TRACKERCONTROLLER_H
