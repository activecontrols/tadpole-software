// THIS FILE IS FOR RUNNING ON A COMPUTER TO WRITE CURVES TO THE SD CARD
// (PLEASE DON'T RUN ON THE TEENSY)

#include "../Curve.h"
#include <fstream>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <cstring>

int main() {
  curve_header header;
  header.ctype = curve_type::lerp;
  header.is_open = false;
  header.of_ratio = 1.2;
  header.lerp.checksum[0] = 'H';
  header.lerp.checksum[1] = 'E';
  header.lerp.checksum[2] = 'L';
  header.lerp.checksum[3] = 'P';

  std::string csv_filename;
  std::cout << "Enter the CSV filename: ";
  std::getline(std::cin, csv_filename);

  int rows = 0;

  std::ifstream csv_file(csv_filename);
  if (!csv_file.is_open()) {
    std::cerr << "Error opening file: " << csv_filename << std::endl;
    return 0;
  }
  std::string line;
  while (std::getline(csv_file, line)) {
    rows += 1;
  }

  csv_file.clear();
  csv_file.seekg(0);

  lerp_point_closed *lcs = (lerp_point_closed *)malloc(rows * sizeof(lerp_point_closed));

  for (int i = 0; i < rows; i++) {
    std::getline(csv_file, line);
    std::stringstream ss(line);
    std::string cell;
    std::getline(ss, cell, ',');
    lcs[i].time = std::stof(cell);
    std::getline(ss, cell, ',');
    lcs[i].thrust = std::stof(cell);
  }

  csv_file.close();

  for (size_t i = 0; i < sizeof(header.curve_label); i++) {
    if (i > csv_filename.length()) {
      header.curve_label[i] = '\0';
    } else {
      header.curve_label[i] = csv_filename[i];
    }
  }

  std::strcpy(header.curve_label, csv_filename.c_str());
  header.lerp.num_points = rows;

  std::ofstream file;
  file.open("out.hex", std::ios_base::binary);
  assert(file.is_open());
  file.write((char *)&header, sizeof(header));
  file.write((char *)lcs, sizeof(lerp_point_closed) * header.lerp.num_points);

  file.close();

  std::cout << "File output to out.hex" << std::endl;

  return 0;
}