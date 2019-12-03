#ifndef LASER_MESSAGE_H
#define LASER_MESSAGE_H
#include "laser_parse_define.h"
#include <string>
#include <vector>
using namespace std;
class laser_message
{
public:
    struct {
    long int seq;
    string frame;
    }Header ;
    struct{
    float angle_min;
    float angle_max;
    float angle_increment;
    float range_min;
    float range_max;
    int sample_number;
    vector<float> ranges;
    vector<float> intensities;
    }Data;
};

#endif // LASER_MESSAGE_H
