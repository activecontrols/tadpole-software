// THIS FILE IS FOR RUNNING ON A COMPUTER TO WRITE CURVES TO THE SD CARD
// (PLEASE DON'T RUN ON THE TEENSY)

#include "../Curve.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#define MODE_THRUST 't'
#define MODE_ANGLES 'a'

curve_header header;           // file data to write
lerp_point_thrust *ltc = NULL; // lerp thrust curve
lerp_point_angle *lac = NULL;  // lerp angle curve

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

// returns number of rows read
int read_csv(std::ifstream &csv_file, char mode) {
  int rows = 0;     // number of rows of data
  std::string line; // holds each line

  while (std::getline(csv_file, line)) {
    rows += 1;
  }

  csv_file.clear();
  csv_file.seekg(0);

  if (mode == MODE_THRUST) {
    std::cout << "Reading file - expecting columns time (s), thrust (lbf) - don't include header row" << std::endl;
    ltc = (lerp_point_thrust *)malloc(rows * sizeof(lerp_point_thrust));
    for (int i = 0; i < rows; i++) {
      std::getline(csv_file, line);
      std::stringstream ss(line);
      read_csv_cell(&ss, &ltc[i].time);
      read_csv_cell(&ss, &ltc[i].thrust);
    }
  } else {
    std::cout << "Reading file - expecting columns time (s), lox_angle (deg), ipa_angle (deg) - don't include header row" << std::endl;
    lac = (lerp_point_angle *)malloc(rows * sizeof(lerp_point_angle));
    for (int i = 0; i < rows; i++) {
      std::getline(csv_file, line);
      std::stringstream ss(line);
      read_csv_cell(&ss, &lac[i].time);
      read_csv_cell(&ss, &lac[i].lox_angle);
      read_csv_cell(&ss, &lac[i].ipa_angle);
    }
  }
  csv_file.close();

  return rows;
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

void write_file(char mode) {
  std::ofstream file;
  file.open("out.hex", std::ios_base::binary);
  assert(file.is_open());
  file.write((char *)&header, sizeof(header));
  if (mode == MODE_THRUST) {
    file.write((char *)ltc, sizeof(lerp_point_thrust) * header.lerp.num_points);
  } else {
    file.write((char *)lac, sizeof(lerp_point_angle) * header.lerp.num_points);
  }
  file.close();
}

int main() {
  char mode;                // thrust or angle
  int num_pts;              // number of points
  std::string csv_filename; // holds filename

  // USER INPUT
  std::cout << "Curve Writer" << std::endl;
  std::cout << "Running on curve version: " << header.version << std::endl;
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

  write_file(mode);
  std::cout << "File output to out.hex" << std::endl;

  return 0;
}