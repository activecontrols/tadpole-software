// THIS FILE IS FOR RUNNING ON A COMPUTER TO WRITE CURVES TO THE SD CARD
// (PLEASE DON'T RUN ON THE TEENSY)

#include "../Curve.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#define ENABLE_SERIAL_COMMS false

#if ENABLE_SERIAL_COMMS
#include <windows.h>
HANDLE hSerial; // com port stuff
#endif

#define MODE_THRUST 't'
#define MODE_ANGLES 'a'

curve_header header;           // file data to write
lerp_point_thrust *ltc = NULL; // lerp thrust curve
lerp_point_angle *lac = NULL;  // lerp angle curve
uint8_t *data = NULL;          // raw data to write
size_t data_len;               // length of data to write

char input_mode() {
  char letter;
  do {
    std::cout << "Enter Mode (Thrust / Angle) [t/a]: ";
    std::cin >> letter;
    if (letter != MODE_ANGLES && letter != MODE_THRUST) {
      std::cout << "\nMode must be 'a' or 't'" << std::endl;
    }
    std::cin.ignore(); // ignore newline left in buffer
  } while (letter != MODE_ANGLES && letter != MODE_THRUST);
  return letter;
}

// reads a csv cell into target
void read_csv_cell(std::stringstream *ss, float *target) {
  std::string cell = ""; // holds one CSV value
  std::getline(*ss, cell, ',');
  if (cell == "") {
    throw std::runtime_error("Malformed CSV - missing column");
  }
  *target = std::stof(cell);
}

void check_header(std::stringstream *ss, std::string header) {
  std::string cell = ""; // holds one CSV value
  std::getline(*ss, cell, ',');
  if (cell == "") {
    throw std::runtime_error("Malformed CSV - missing column");
  }
  if (cell != header) {
    std::cerr << "Expected header: " << header << " but saw: " << cell << std::endl;
    throw std::runtime_error("Malformed CSV.");
  }
}

// returns number of rows read
int read_csv(std::ifstream &csv_file, char mode) {
  int rows = 0;     // number of rows of data
  std::string line; // holds each line

  while (std::getline(csv_file, line)) {
    rows += 1;
  }
  rows--; // header

  csv_file.clear();
  csv_file.seekg(0);

  if (mode == MODE_THRUST) {
    ltc = (lerp_point_thrust *)malloc(rows * sizeof(lerp_point_thrust));

    std::getline(csv_file, line);
    std::stringstream ss(line);
    check_header(&ss, "time (s)");
    check_header(&ss, "thrust (lbf)");
    std::cout << "File header valid - reading file." << std::endl;

    for (int i = 0; i < rows; i++) {
      std::getline(csv_file, line);
      std::stringstream ss(line);
      read_csv_cell(&ss, &ltc[i].time);
      read_csv_cell(&ss, &ltc[i].thrust);

      if (i == 0 && ltc[i].time != 0) {
        std::cout << "WARNING - file should start with time = 0. Continuing anyway." << std::endl;
      }
    }

    data = (uint8_t *)ltc;
    data_len = rows * sizeof(lerp_point_thrust);
  } else {
    lac = (lerp_point_angle *)malloc(rows * sizeof(lerp_point_angle));

    std::getline(csv_file, line);
    std::stringstream ss(line);
    check_header(&ss, "time (s)");
    check_header(&ss, "lox_angle (deg)");
    check_header(&ss, "ipa_angle (deg)");
    std::cout << "File header valid - reading file." << std::endl;

    for (int i = 0; i < rows; i++) {
      std::getline(csv_file, line);
      std::stringstream ss(line);
      read_csv_cell(&ss, &lac[i].time);
      read_csv_cell(&ss, &lac[i].lox_angle);
      read_csv_cell(&ss, &lac[i].ipa_angle);

      if (i == 0 && lac[i].time != 0) {
        std::cout << "WARNING - file should start with time = 0. Continuing anyway." << std::endl;
      }
    }

    data = (uint8_t *)lac;
    data_len = rows * sizeof(lerp_point_angle);
  }
  csv_file.close();

  return rows; // header row
}

void fill_header(std::string csv_filename, char mode, int num_points) {
  csv_filename = "autogen from " + csv_filename;
  for (size_t i = 0; i < sizeof(header.curve_label); i++) {
    if (i > csv_filename.length()) {
      header.curve_label[i] = '\0';
    } else {
      header.curve_label[i] = csv_filename[i];
    }
  }

  header.ctype = curve_type::lerp; // only support lerp curves
  header.of_ratio = 1.2;           // this is only needed for sine / chirp curves
  header.is_thrust = mode == MODE_THRUST;
  header.lerp.num_points = num_points;
}

void write_file() {
  std::ofstream file;
  file.open("out.hex", std::ios_base::binary);
  assert(file.is_open());
  file.write((char *)&header, sizeof(header));
  file.write((char *)data, data_len);
  file.close();
}

#if ENABLE_SERIAL_COMMS
void setup_com_port(const char *portName) {
  hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (hSerial == INVALID_HANDLE_VALUE) {
    std::cerr << "Error opening COM port\n";
    return;
  }

  // Set COM port parameters
  DCB dcbSerialParams;
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  if (!GetCommState(hSerial, &dcbSerialParams)) {
    std::cerr << "Error getting COM port state\n";
    CloseHandle(hSerial);
    return;
  }

  // Configure baud rate, byte size, stop bits, and parity
  dcbSerialParams.BaudRate = CBR_9600;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  if (!SetCommState(hSerial, &dcbSerialParams)) {
    std::cerr << "Error setting COM port state\n";
    CloseHandle(hSerial);
    return;
  }

  // Configure timeouts for reading
  COMMTIMEOUTS timeouts;
  timeouts.ReadIntervalTimeout = 50;
  timeouts.ReadTotalTimeoutConstant = 50;
  timeouts.ReadTotalTimeoutMultiplier = 10;
  timeouts.WriteTotalTimeoutConstant = 50;
  timeouts.WriteTotalTimeoutMultiplier = 10;

  if (!SetCommTimeouts(hSerial, &timeouts)) {
    std::cerr << "Error setting COM port timeouts\n";
    return;
  }
}

void read_com_to_serial() {
  uint8_t readBuffer[100];
  DWORD bytesRead = 100;

  while (bytesRead == 100) {
    if (!ReadFile(hSerial, readBuffer, sizeof(readBuffer), &bytesRead, NULL)) {
      std::cerr << "Error reading from COM port\n";
    }
    for (DWORD i = 0; i < bytesRead; i++) {
      std::cout << readBuffer[i];
    }
  }
}

bool write_curve_over_com() {
  char cmd[] = "ping\nload_curve_serial\n";

  // Write raw binary data to the COM port
  DWORD bytesWritten;
  if (!WriteFile(hSerial, (uint8_t *)cmd, sizeof(cmd) - 1, &bytesWritten, NULL)) { // don't write final null byte
    std::cerr << "Error writing to COM port\n";
    CloseHandle(hSerial);
    return false;
  }
  Sleep(1000); // wait one second for teensy to respond
  read_com_to_serial();

  if (!WriteFile(hSerial, (uint8_t *)&header, sizeof(header), &bytesWritten, NULL)) {
    std::cerr << "Error writing to COM port\n";
    CloseHandle(hSerial);
    return false;
  }
  if (!WriteFile(hSerial, data, data_len, &bytesWritten, NULL)) {
    std::cerr << "Error writing to COM port\n";
    CloseHandle(hSerial);
    return false;
  }

  Sleep(1000); // wait one second for teensy to respond
  read_com_to_serial();
  std::cout << "\nData written to COM port successfully\n";

  // Close the COM port handle
  CloseHandle(hSerial);
  return true;
}
#endif

int main() {
  char mode;                // thrust or angle
  int num_pts;              // number of points
  std::string csv_filename; // holds filename

  // USER INPUT
  std::cout << "Curve Writer" << std::endl;
  std::cout << "Running on curveh version: " << header.version << std::endl;
  mode = input_mode();
  std::cout << "Enter the CSV filename: ";
  std::getline(std::cin, csv_filename);

  std::ifstream csv_file(csv_filename);
  if (!csv_file.is_open()) {
    std::cerr << "Error opening file: " << csv_filename << std::endl;
    return 0;
  }

  num_pts = read_csv(csv_file, mode);
  fill_header(csv_filename, mode, num_pts);

  write_file();
  std::cout << "File output to out.hex" << std::endl;

#if ENABLE_SERIAL_COMMS
  char letter;
  std::cout << "Hit [y] to send data to serial: ";
  std::cin >> letter;
  std::cin.ignore(); // ignore newline left in buffer
  if (letter == 'y') {
    std::string com_port;
    std::cout << "Enter the COM port: ";
    std::getline(std::cin, com_port);
    setup_com_port(com_port.c_str());
    write_curve_over_com();
  }
#endif

  return 0;
}