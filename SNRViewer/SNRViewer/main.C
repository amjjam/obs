#include "SNRViewer.H"

#include <QApplication>
#include <amjCom/amjCom.H>
#include <amjCom/amjComUDP.H>

#include "../../shared/include/Help.H"

#include <iostream>

Help help({
            /****************************************************************************/
            "SNRViewer",
            "",
            "--nbaselines <int>",
            "  The number of baselines. The default value is 2.",
            "--receiver-tracker-snr",
            "  Communicator for receiving fringe tracker SNR"
            /****************************************************************************/
          });

void parse_args(int argc, char *argv[]);

int nBaselines=2;
std::string receiver_tracker_snr;

int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  parse_args(argc,argv);
  SNRViewer w;
  w.show();

  return a.exec();
}

void parse_args(int argc, char *argv[]){
  help.help(argc,argv);

  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--nbaselines")==0){
        i++;
        nBaselines=atoi(argv[i]);
      }
    else if(strcmp(argv[i],"--receiver-tracker-snr")==0){
        i++;
        receiver_tracker_snr=argv[i];
      }
  }
}
