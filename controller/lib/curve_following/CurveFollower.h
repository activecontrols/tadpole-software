#ifndef CURVE_FOLLOWER_H
#define CURVE_FOLLOWER_H

namespace CurveFollower {

void begin();
void follow_curve(const char *log_file_name);
void follow_curve_cmd();
void auto_seq();

}; // namespace CurveFollower

#endif