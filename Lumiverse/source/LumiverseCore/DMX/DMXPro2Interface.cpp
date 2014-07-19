#include "DMXPro2Interface.h"

#ifdef USE_DMXPRO2
namespace Lumiverse {

DMXPro2Interface::DMXPro2Interface(string id, int proNum, int out1, int out2) : 
  m_proNum(proNum), m_connected(0), m_out1Universe(out1), m_out2Universe(out2)
{
  m_ifaceName = "ENTTEC DMX USB PRO MK2";
  setInterfaceId(id);
}


DMXPro2Interface::~DMXPro2Interface()
{
}

void DMXPro2Interface::init() {
  // TODO
  // Right now there's an assumption that you're not mixing Mark 1 and Mark 2
  // devices. Which may end up being a false assumption at some point. This
  // will be corrected in a future version, at which point this class may
  // be renamed.

  int numDevices = listDevices();

  if (numDevices == 0) {
    throw runtime_error("No DMX USB PRO interfaces found.");
  }
  
  if (m_connected) {
    // Device is already connected, don't init agian.
    return;
  }

  // Attempt to connect
  m_connected = openDevice(m_proNum);
  if (!m_connected)
    throw runtime_error("Unable to connect to DMX USB PRO interface.");

  // Activate Pro 2 features
  // In actuality it's not that different from the Mk 1 so at some point this
  // class may be refactored to reflect that. Since I'm testing on a 2,
  // this class is written specifically for the 2.
  int res;
  
  res = sendData(SET_API_KEY_LABEL, const_cast<unsigned char *>(APIKey), 4);
  this_thread::sleep_for(chrono::milliseconds(200));
  Logger::log(INFO, "PRO Mk2 ... Activated ... ");

  // Activate ports 1 and 2 in DMX mode.
  setPorts(1, 1);
}

void DMXPro2Interface::sendDMX(unsigned char* data, unsigned int universe) {
  unsigned char toSend[513];
  int res = 0;

  if (m_deviceHandle != NULL)
  {
    // Apparently the first byte of the series has to be 0.
    toSend[0] = 0;
    memcpy(&(toSend[1]), data, 512);

    // Figure out where to send this universe
    int label;
    if (universe == m_out1Universe)
      label = SEND_DMX_PORT1;
    else if (universe == m_out2Universe)
      label = SEND_DMX_PORT2;
    else {
      Logger::log(WARN, "Attempting to send to universe not assigned to this interface.");
      return;
    }
    
    // send the array here
    res = sendData(label, toSend, 513);
    if (res < 0) {
      throw runtime_error("FAILED to send DMX to DMX USB PRO MK2");
    }
  }
}

void DMXPro2Interface::reset() {
// Note that this approach only works on windows at the moment
#ifdef _MSC_VER
  WORD wVID = 0x0403;
  WORD wPID = 0x6001;
  FT_STATUS ftStatus;

  Logger::log(INFO, "Reloading devices for use with drivers ");
  ftStatus = FT_Reload(wVID, wPID);

  // Must wait a while for devices to be re-enumerated
  this_thread::sleep_for(chrono::milliseconds(3500));
  if (ftStatus != FT_OK)
    Logger::log(ERR, "Reloading Driver FAILED");
  else
    Logger::log(INFO,"Reloading Driver D2XX PASSED");
#else
  // TODO
  // Can't use FT_Reload to reload entire driver, need other approach
#endif
}

void DMXPro2Interface::closeInt() {
  if (m_deviceHandle != NULL)
    FT_Close(m_deviceHandle);
}

JSONNode DMXPro2Interface::toJSON() {
  JSONNode root;

  root.set_name(getInterfaceId());
  root.push_back(JSONNode("type", getInterfaceType()));
  root.push_back(JSONNode("proNum", m_proNum));
  root.push_back(JSONNode("out1", m_out1Universe));
  root.push_back(JSONNode("out2", m_out2Universe));

  return root;
}

int DMXPro2Interface::listDevices() {
  FT_STATUS ftStatus;
  DWORD numDevs = 0;
  ftStatus = FT_ListDevices((PVOID)&numDevs, NULL, FT_LIST_NUMBER_ONLY);
  if (ftStatus == FT_OK)
    return numDevs;
  return NO_RESPONSE;
}

int DMXPro2Interface::sendData(int label, unsigned char *data, int length)
{
  unsigned char end_code = DMX_END_CODE;
  FT_STATUS res = 0;
  DWORD bytes_written = 0;

  // Form Packet Header
  unsigned char header[DMX_HEADER_LENGTH];
  header[0] = DMX_START_CODE;
  header[1] = label;
  header[2] = length & OFFSET;
  header[3] = length >> BYTE_LENGTH;

  // Write The Header
  res = FT_Write(m_deviceHandle, (unsigned char *)header, DMX_HEADER_LENGTH, &bytes_written);
  if (bytes_written != DMX_HEADER_LENGTH)
    return NO_RESPONSE;

  // Write The Data
  res = FT_Write(m_deviceHandle, (unsigned char *)data, length, &bytes_written);
  if (bytes_written != length)
    return NO_RESPONSE;

  // Write End Code
  res = FT_Write(m_deviceHandle, (unsigned char *)&end_code, ONE_BYTE, &bytes_written);
  if (bytes_written != ONE_BYTE)
    return NO_RESPONSE;

  if (res == FT_OK)
    return TRUE;
  else
    return FALSE;
}

int DMXPro2Interface::receiveData(int label, unsigned char *data, unsigned int expected_length)
{
  FT_STATUS res = 0;
  DWORD length = 0;
  DWORD bytes_read = 0;
  unsigned char byte = 0;
  char buffer[600];

  // Check for Start Code and matching Label
  while (byte != label)
  {
    while (byte != DMX_START_CODE)
    {
      res = FT_Read(m_deviceHandle, (unsigned char *)&byte, ONE_BYTE, &bytes_read);
      if (bytes_read == NO_RESPONSE) return  NO_RESPONSE;
    }
    res = FT_Read(m_deviceHandle, (unsigned char *)&byte, ONE_BYTE, &bytes_read);
    if (bytes_read == NO_RESPONSE) return  NO_RESPONSE;
  }

  // Read the rest of the Header Byte by Byte -- Get Length
  res = FT_Read(m_deviceHandle, (unsigned char *)&byte, ONE_BYTE, &bytes_read);
  if (bytes_read == NO_RESPONSE) return  NO_RESPONSE;
  length = byte;
  res = FT_Read(m_deviceHandle, (unsigned char *)&byte, ONE_BYTE, &bytes_read);
  if (res != FT_OK) return  NO_RESPONSE;
  length += ((uint32_t)byte) << BYTE_LENGTH;

  // Check Length is not greater than allowed
  if (length > DMX_PACKET_SIZE)
    return  NO_RESPONSE;

  // Read the actual Response Data
  res = FT_Read(m_deviceHandle, buffer, length, &bytes_read);
  if (bytes_read != length) return  NO_RESPONSE;

  // Check The End Code
  res = FT_Read(m_deviceHandle, (unsigned char *)&byte, ONE_BYTE, &bytes_read);
  if (bytes_read == NO_RESPONSE) return  NO_RESPONSE;
  if (byte != DMX_END_CODE) return  NO_RESPONSE;

  // Copy The Data read to the buffer passed
  memcpy(data, buffer, expected_length);
  return TRUE;
}

bool DMXPro2Interface::openDevice(int device_num)
{
  uint8_t temp[4];
  long version;
  uint8_t major_ver, minor_ver, build_ver;
  int size = 0;
  int res = 0;
  int tries = 0;
  uint8_t latencyTimer;
  FT_STATUS ftStatus;
  int BreakTime;
  int MABTime;

  // Try at least 3 times 
  do  {
    stringstream ss;
    ss << "------ D2XX ------- Opening [Device " << device_num << "] ------ Try " << tries;
    Logger::log(INFO, ss.str());
    ftStatus = FT_Open(device_num, &m_deviceHandle);
    this_thread::sleep_for(chrono::milliseconds(500));
    tries++;
  } while ((ftStatus != FT_OK) && (tries < 3));

  if (ftStatus == FT_OK)
  {
    // D2XX Driver Version
    ftStatus = FT_GetDriverVersion(m_deviceHandle, (LPDWORD)&version);
    if (ftStatus == FT_OK)
    {
#pragma warning(push)
#pragma warning(disable : 4333)
      major_ver = (uint8_t)version >> 16;
      minor_ver = (uint8_t)version >> 8;
#pragma warning(pop)
      build_ver = (uint8_t)version & 0xFF;
      stringstream ss;
      ss << "D2XX Driver Version:: " << (int)major_ver << "." << (int)minor_ver << "." << build_ver;
      Logger::log(INFO, ss.str());
    }
    else
      Logger::log(WARN, "Unable to Get D2XX Driver Version");

    // Latency Timer
    ftStatus = FT_GetLatencyTimer(m_deviceHandle, (PUCHAR)&latencyTimer);
    if (ftStatus == FT_OK) {
      stringstream ss;
      ss << "Latency Timer:: " << latencyTimer;
      Logger::log(INFO, ss.str());
    }
    else
      Logger::log(WARN, "Unable to Get Latency Timer");

    // These are important values that can be altered to suit your needs
    // Timeout in microseconds: Too high or too low value should not be used 
    FT_SetTimeouts(m_deviceHandle, m_readTimeout, m_writeTimeout);
    // Buffer size in bytes (multiple of 4096) 
    FT_SetUSBParameters(m_deviceHandle, RX_BUFFER_SIZE, TX_BUFFER_SIZE);
    // Good idea to purge the buffer on initialize
    FT_Purge(m_deviceHandle, FT_PURGE_RX);

    // Send Get Widget Params to get Device Info
    Logger::log(INFO, "Sending GET_WIDGET_PARAMS packet... ");
    res = sendData(GET_WIDGET_PARAMS, (unsigned char *)&size, 2);
    if (res == NO_RESPONSE)
    {
      FT_Purge(m_deviceHandle, FT_PURGE_TX);
      res = sendData(GET_WIDGET_PARAMS, (unsigned char *)&size, 2);
      if (res == NO_RESPONSE)
      {
        closeInt();
        return  NO_RESPONSE;
      }
    }
    else
      Logger::log(INFO, "PRO Connected Succesfully");
    // Receive Widget Response
    Logger::log(INFO, "Waiting for GET_WIDGET_PARAMS_REPLY packet... ");
    res = receiveData(GET_WIDGET_PARAMS_REPLY, (unsigned char *)&m_PROParams, sizeof(DMXUSBPROParamsType));
    if (res == NO_RESPONSE)
    {
      res = receiveData(GET_WIDGET_PARAMS_REPLY, (unsigned char *)&m_PROParams, sizeof(DMXUSBPROParamsType));
      if (res == NO_RESPONSE)
      {
        closeInt();
        return  NO_RESPONSE;
      }
    }
    else
      Logger::log(INFO, "GET WIDGET REPLY Received ... ");
    // Firmware  Version
    m_versionMSB = m_PROParams.FirmwareMSB;
    m_versionLSB = m_PROParams.FirmwareLSB;

    // Display All Info avialable
    res = sendData(GET_WIDGET_SN, (unsigned char *)&size, 2);
    res = receiveData(GET_WIDGET_SN, (unsigned char *)&temp, 4);
    stringstream ss;
    ss << "\n-----------::USB PRO Connected [Information Follows]::------------";
    ss << "\n\t  FIRMWARE VERSION: " << m_versionMSB << "." << m_versionMSB;
    BreakTime = (int)(m_PROParams.BreakTime * 10.67) + 100;
    ss << "\n\t  BREAK TIME: " << BreakTime << " micro sec";
    MABTime = (int)(m_PROParams.MaBTime * 10.67);
    ss << "\n\t  MAB TIME: " << MABTime << " micro sec";
    ss << "\n\t  SEND REFRESH RATE: " << (int)m_PROParams.RefreshRate << " packets/sec";
    ss << "\n----------------------------------------------------------------\n";
    Logger::log(INFO, ss.str());
    return true;
  }
  else // Can't open Device 
    return false;
}

void DMXPro2Interface::purgeBuffer()
{
  FT_Purge(m_deviceHandle, FT_PURGE_TX);
  FT_Purge(m_deviceHandle, FT_PURGE_RX);
}

void DMXPro2Interface::setPorts(uint8_t port1, uint8_t port2) {
  uint8_t portSet[] = { port1, port2 };
  int res = 0;

  purgeBuffer();

  // Enable Ports to DMX on both 
  res = sendData(SET_PORT_ASSIGNMENT_LABEL, portSet, 2);
  this_thread::sleep_for(chrono::milliseconds(200));
  Logger::log(INFO, "PRO Mk2 ... Ready for DMX on both ports ... ");
}
}
#endif