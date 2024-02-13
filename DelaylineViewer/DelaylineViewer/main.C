#include "DelaylineViewer.H"

#include <QApplication>
#include <amjCom/amjCom.H>
#include <amjCom/amjComUDP.H>

#include "../../shared/include/Help.H"
Help help({
            /*************************************************************/
            "DelaylineViewer",
            "",
            "--ndelaylines <int>",
            "  The number of delay lines. The default value is 6.",
            "--receiver-delayline-delays",
            "  Communicator for receiving delayline delays (relative positions)",
            /************************************************************************/
          });

#include "../../shared/include/Delays.H"

void parse_args(int argc, char *argv[]);

int nDelaylines=6;
std::string receiver_delayline_delays;


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  DelaylineViewer w;
  w.show();

  // Parse command-line argument
  parse_args(argc,argv);

  return a.exec();
}

void parse_args(int argc, char *argv[]){
  help.help(argc,argv);

  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--ndelaylines")==0){
        i++;
        nDelaylines=atoi(argv[i]);
      }
    else if(strcmp(argv[i],"--receiver-delayline-delays")==0){
        i++;
        receiver_delayline_delays=argv[i];
      }
  }
}
