#include "smartnet_parser.h"

using namespace std;
SmartnetParser::SmartnetParser() {
  lastaddress = 0;
  lastcmd     = 0;
  numStacked  = 0;
  numConsumed = 0;
}

bool SmartnetParser::is_chan(int cmd) {
  if (cmd < 0x2F8)
  {
    return true;
  }

  if (cmd < 0x32F)
  {
    return false;
  }

  if (cmd < 0x340)
  {
    return true;
  }

  if (cmd == 0x3BE)
  {
    return true;
  }

  if ((cmd > 0x3C0) && (cmd <= 0x3FE))
  {
    return true;
  }
  return false;
}

double SmartnetParser::getfreq(int cmd) {
  float freq;


  /* Different Systems will have different band plans. Below is the one for
     WMATA which is a bit werid:*/
  if (cmd < 0x12e) {
    freq = float((cmd) * 0.025 + 489.0875);
  } else if (cmd < 0x2b0) {
    freq = float((cmd - 380) * 0.025 + 489.0875);
  } else {
    freq = 0;
  }

  //      cout << "LCMD: 0x" <<  hex << cmd << " Freq: " << freq << " Multi: "
  // << (cmd - 308) * 0.025 << " CMD: " << dec << cmd << endl;

  /*

          if (cmd < 0x1b8) {
                  freq = float(cmd * 0.025 + 851.0125);
          } else if (cmd < 0x230) {
                  freq = float(cmd * 0.025 + 851.0125 - 10.9875);
          } else {
                  freq = 0;
          }
   */
  return freq * 1000000;
}

std::vector<TrunkMessage>SmartnetParser::parse_message(std::string s) {
  std::vector<TrunkMessage> messages;
  TrunkMessage message;

  char tempArea[512];
  unsigned short blockNum;
  char banktype;
  unsigned short tt1, tt2;
  static unsigned int ott1, ott2;

  // print_osw(s);
  message.message_type = UNKNOWN;
  message.encrypted    = false;
  message.tdma         = false;
  message.source       = 0;
  message.sysid        = 0;
  message.emergency    = false;

  std::vector<std::string> x;
  boost::split(x, s, boost::is_any_of(","), boost::token_compress_on);

  int  full_address = atoi(x[0].c_str());
  int  status       = full_address & 0x000F;
  long address      = full_address & 0xFFF0;
  int  groupflag    = atoi(x[1].c_str());
  int  command      = atoi(x[2].c_str());

  struct osw_stru bosw;
  bosw.id           = address;
  bosw.full_address = full_address;
  bosw.address      = address;
  bosw.status       = status;
  bosw.grp          = groupflag;
  bosw.cmd          = command;

  // struct osw_stru* Inposw = &bosw;
  cout.precision(0);

  // maintain a sliding stack of 5 OSWs. If previous iteration used more than
  // one,
  // don't utilize stack until all used ones have slid past.

  switch (numStacked) // note: drop-thru is intentional!
  {
  case 5:
  case 4:
    stack[4] = stack[3];

  case 3:
    stack[3] = stack[2];

  case 2:
    stack[2] = stack[1];

  case 1:
    stack[1] = stack[0];

  case 0:
    stack[0] = bosw;
    break;

  default:
    cout << "corrupt value for nstacked" << endl;
    break;
  }

  if (numStacked < 5)
  {
    ++numStacked;
  }
/*
  if (numConsumed > 0)
  {
    if (--numConsumed > 0)
    {
      return messages;
    }
  }

  if (numStacked < 3)
  {
    return messages; // at least need a window of 3 and 5 is better.
  }
*/
  x.clear();
  vector<string>().swap(x);

  /*
          if( (stack[0].cmd < 0x2d0) && (stack[2].cmd == 0x0320) &&
             (stack[1].cmd == OSW_EXTENDED_FCN) )
          {
                                          numConsumed = 3;
                                          //idlesWithNoId = 0; // got
                                             identifying info...
                                          //if( (stack[0].id & 0xfc00) ==
                                             0x6000)
                                          //{
                                                          cout << "uhf/vhf
                                                             equivalent of
                                                             308/320/30b" <<
                                                             endl;
                                                          cout << "Freq: " <<
                                                             getfreq(stack[0].cmd)
                                                             << " 0add: " << dec
                                                             <<
                                                              stack[0].address
                                                             << " 0full_add: "
                                                             <<
                                                             stack[0].full_address
                                                              << " 1add: " <<
                                                             stack[1].address <<
                                                             " 1full_add: " <<
                                                             stack[1].full_address
                                                              << endl;
                                          //}

          }
   */


  if (((command >= 0x340) && (command <= 0x34E)) || (command == 0x350)) {
    cout << "Patching Command: " << hex << command << " Last CMD: 0x" <<  hex << lastcmd << " Freq: " << message.freq << " Talkgroup: " << dec << address << " Last Address: " << dec << lastaddress << endl;
  }

  if ((address & 0xfc00) == 0x2800) {
    message.sysid        = lastaddress;
    message.message_type = SYSID;
  } else if (command < 0x2d0) {
    message.talkgroup = address;
    message.freq      = getfreq(command);

    // cout << "Command: " << hex << command << " Last CMD: 0x" <<  hex <<
    // lastcmd << " Freq: " << message.freq << " Talkgroup: " << dec << address
    // << " Last Address: " << dec << lastaddress<< endl;


    if ((stack[2].cmd == 0x0320) && (stack[1].cmd == OSW_EXTENDED_FCN) && groupflag)
    {
      numConsumed = 3;

      cout << "uhf/vhf equivalent of 308/320/30b" << endl;
      cout << "Freq: " << fixed << getfreq(stack[0].cmd) << " 0add: " << dec <<  stack[0].address << " 0full_add: " << stack[0].full_address  << " 1add: " << stack[1].address << " 1full_add: " << stack[1].full_address  << endl;

      // Channel Grant
      message.message_type = GRANT;
      message.source       = lastaddress;

      // Check Status
      cout << "Grant Command: " << hex << command << " Last CMD: 0x" <<  hex << lastcmd << " Freq: " << fixed <<  message.freq << " Talkgroup: " << dec << address << " Source: " << dec << lastaddress << " 1st: " << (address & 0x2000) << " 2nd: " <<  (address & 0x0800) << " grp: " << groupflag << endl;

      cout << "Status: " << status << endl;

      if ((status == 2) || (status == 4) || (status == 5)) {
        message.emergency = true;
      } else if (status >= 8) { // Ignore DES Encryption
        message.message_type = UNKNOWN;
      }
    } else  {
      // Call continuation
      if (groupflag) {
        message.talkgroup    = full_address;
        message.message_type = UPDATE;
        cout << "UPDATE [ Freq: " << fixed << getfreq(command) << " TG: " << dec << address << " Full: " << dec << full_address << " CMD: " << hex << command << " Last CMD: " << hex << stack[1].cmd  << " ] Last CMD: " << hex << lastcmd << " GRoup: " << groupflag << endl;
      }
    }
  } else if (command == 0x03c0) {
    message.message_type = STATUS;

    // cout << "Status Command: " << hex << command << " Last CMD: 0x" <<  hex
    // << lastcmd << " Freq: " << message.freq << " Talkgroup: " << dec <<
    // address << " Last Address: " << dec << lastaddress<< endl;

    // parse_status(command, address,groupflag);
  }

  lastaddress = full_address;
  lastcmd     = command;
  messages.push_back(message);
  return messages;
}
