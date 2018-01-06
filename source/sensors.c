#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
// compass compentation factors. The top one is made with Ellipse estimation. The bootom one with normal polynomial fit
//float cc[360] = {0.96118, 0.96234, 0.96352, 0.96472, 0.96592, 0.96715, 0.96838, 0.96962, 0.97088, 0.97215, 0.97343, 0.97471, 0.97601, 0.97732, 0.97863, 0.97995, 0.98127, 0.9826, 0.98393, 0.98527, 0.98661, 0.98795, 0.98929, 0.99063, 0.99197, 0.99331, 0.99464, 0.99598, 0.9973, 0.99862, 0.99994, 1.0012, 1.0025, 1.0038, 1.0051, 1.0064, 1.0076, 1.0089, 1.0101, 1.0113, 1.0125, 1.0136, 1.0148, 1.0159, 1.017, 1.0181, 1.0192, 1.0202, 1.0212, 1.0222, 1.0232, 1.0241, 1.0251, 1.0259, 1.0268, 1.0276, 1.0284, 1.0291, 1.0299, 1.0305, 1.0312, 1.0318, 1.0324, 1.0329, 1.0334, 1.0339, 1.0343, 1.0347, 1.0351, 1.0354, 1.0356, 1.0359, 1.036, 1.0362, 1.0363, 1.0363, 1.0363, 1.0363, 1.0362, 1.0361, 1.0359, 1.0357, 1.0354, 1.0351, 1.0347, 1.0343, 1.0339, 1.0334, 1.0328, 1.0323, 1.0316, 1.0309, 1.0302, 1.0295, 1.0286, 1.0278, 1.0269, 1.0259, 1.0249, 1.0239, 1.0228, 1.0217, 1.0205, 1.0193, 1.0181, 1.0168, 1.0154, 1.0141, 1.0127, 1.0112, 1.0097, 1.0082, 1.0066, 1.005, 1.0034, 1.0017, 1, 0.99832, 0.99656, 0.99477, 0.99296, 0.99111, 0.98924, 0.98734, 0.98542, 0.98347, 0.9815, 0.97951, 0.9775, 0.97547, 0.97342, 0.97136, 0.96928, 0.96719, 0.96508, 0.96297, 0.96084, 0.95871, 0.95657, 0.95442, 0.95227, 0.95011, 0.94795, 0.9458, 0.94364, 0.94149, 0.93934, 0.93719, 0.93506, 0.93293, 0.93081, 0.9287, 0.9266, 0.92451, 0.92245, 0.92039, 0.91836, 0.91634, 0.91435, 0.91237, 0.91042, 0.9085, 0.90659, 0.90472, 0.90287, 0.90105, 0.89927, 0.89751, 0.89578, 0.89409, 0.89244, 0.89081, 0.88923, 0.88768, 0.88617, 0.8847, 0.88326, 0.88187, 0.88052, 0.87921, 0.87795, 0.87672, 0.87554, 0.87441, 0.87331, 0.87227, 0.87127, 0.87031, 0.8694, 0.86853, 0.86771, 0.86694, 0.86621, 0.86553, 0.8649, 0.86431, 0.86376, 0.86327, 0.86281, 0.86241, 0.86204, 0.86173, 0.86145, 0.86122, 0.86103, 0.86089, 0.86079, 0.86072, 0.8607, 0.86072, 0.86078, 0.86088, 0.86101, 0.86118, 0.86139, 0.86163, 0.86191, 0.86222, 0.86256, 0.86294, 0.86334, 0.86378, 0.86424, 0.86473, 0.86525, 0.86579, 0.86636, 0.86695, 0.86756, 0.8682, 0.86885, 0.86952, 0.87021, 0.87092, 0.87165, 0.87239, 0.87314, 0.87391, 0.87468, 0.87547, 0.87627, 0.87708, 0.87789, 0.87872, 0.87954, 0.88038, 0.88122, 0.88206, 0.8829, 0.88375, 0.88459, 0.88544, 0.88628, 0.88713, 0.88797, 0.88881, 0.88965, 0.89048, 0.8913, 0.89213, 0.89294, 0.89375, 0.89456, 0.89535, 0.89614, 0.89692, 0.89769, 0.89846, 0.89921, 0.89996, 0.90069, 0.90142, 0.90214, 0.90285, 0.90354, 0.90423, 0.90491, 0.90557, 0.90623, 0.90688, 0.90751, 0.90814, 0.90876, 0.90937, 0.90996, 0.91055, 0.91113, 0.9117, 0.91227, 0.91282, 0.91337, 0.91391, 0.91444, 0.91497, 0.91549, 0.916, 0.91651, 0.91702, 0.91752, 0.91801, 0.91851, 0.919, 0.91949, 0.91997, 0.92046, 0.92094, 0.92143, 0.92191, 0.9224, 0.92289, 0.92338, 0.92388, 0.92438, 0.92488, 0.92539, 0.9259, 0.92642, 0.92695, 0.92748, 0.92803, 0.92858, 0.92914, 0.92971, 0.93029, 0.93088, 0.93148, 0.9321, 0.93272, 0.93336, 0.93402, 0.93468, 0.93536, 0.93606, 0.93677, 0.9375, 0.93824, 0.939, 0.93977, 0.94056, 0.94137, 0.9422, 0.94304, 0.9439, 0.94477, 0.94567, 0.94658, 0.94751, 0.94846, 0.94942, 0.95041, 0.95141, 0.95243, 0.95346, 0.95452, 0.95559, 0.95667, 0.95777, 0.95889, 0.96003, 0.96118};
//float cc[360] = {1.0, 1.0072, 1.014, 1.0204, 1.0263, 1.0318, 1.037, 1.0417, 1.0461, 1.0501, 1.0538, 1.0572, 1.0602, 1.063, 1.0654, 1.0676, 1.0694, 1.0711, 1.0725, 1.0736, 1.0745, 1.0752, 1.0757, 1.076, 1.0761, 1.076, 1.0757, 1.0753, 1.0747, 1.0739, 1.073, 1.072, 1.0708, 1.0696, 1.0682, 1.0667, 1.0651, 1.0634, 1.0616, 1.0597, 1.0577, 1.0557, 1.0536, 1.0514, 1.0492, 1.0469, 1.0446, 1.0422, 1.0398, 1.0373, 1.0348, 1.0323, 1.0297, 1.0271, 1.0245, 1.0219, 1.0193, 1.0166, 1.014, 1.0113, 1.0087, 1.006, 1.0033, 1.0007, 0.99804, 0.99539, 0.99276, 0.99014, 0.98753, 0.98494, 0.98237, 0.97981, 0.97727, 0.97476, 0.97226, 0.96979, 0.96735, 0.96493, 0.96253, 0.96017, 0.95783, 0.95552, 0.95324, 0.95099, 0.94878, 0.94659, 0.94444, 0.94232, 0.94023, 0.93817, 0.93615, 0.93417, 0.93222, 0.9303, 0.92841, 0.92657, 0.92475, 0.92297, 0.92123, 0.91952, 0.91784, 0.9162, 0.9146, 0.91303, 0.91149, 0.90999, 0.90852, 0.90708, 0.90568, 0.90431, 0.90298, 0.90167, 0.9004, 0.89916, 0.89796, 0.89678, 0.89564, 0.89453, 0.89344, 0.89239, 0.89137, 0.89037, 0.88941, 0.88847, 0.88756, 0.88668, 0.88583, 0.885, 0.8842, 0.88343, 0.88268, 0.88196, 0.88126, 0.88059, 0.87994, 0.87932, 0.87871, 0.87813, 0.87758, 0.87704, 0.87653, 0.87604, 0.87557, 0.87512, 0.87469, 0.87428, 0.8739, 0.87353, 0.87317, 0.87284, 0.87253, 0.87223, 0.87196, 0.8717, 0.87145, 0.87123, 0.87102, 0.87082, 0.87065, 0.87048, 0.87034, 0.87021, 0.87009, 0.86999, 0.86991, 0.86984, 0.86978, 0.86974, 0.86971, 0.8697, 0.8697, 0.86971, 0.86974, 0.86978, 0.86983, 0.8699, 0.86998, 0.87007, 0.87017, 0.87029, 0.87042, 0.87056, 0.87072, 0.87089, 0.87106, 0.87126, 0.87146, 0.87168, 0.8719, 0.87214, 0.87239, 0.87266, 0.87293, 0.87322, 0.87352, 0.87383, 0.87415, 0.87449, 0.87483, 0.87519, 0.87556, 0.87594, 0.87633, 0.87674, 0.87715, 0.87758, 0.87802, 0.87847, 0.87893, 0.8794, 0.87989, 0.88038, 0.88089, 0.88141, 0.88194, 0.88248, 0.88304, 0.8836, 0.88418, 0.88477, 0.88536, 0.88597, 0.88659, 0.88722, 0.88787, 0.88852, 0.88918, 0.88986, 0.89054, 0.89124, 0.89194, 0.89266, 0.89338, 0.89412, 0.89486, 0.89562, 0.89638, 0.89716, 0.89794, 0.89873, 0.89953, 0.90034, 0.90116, 0.90199, 0.90282, 0.90367, 0.90452, 0.90537, 0.90624, 0.90711, 0.90799, 0.90887, 0.90976, 0.91066, 0.91156, 0.91247, 0.91338, 0.9143, 0.91522, 0.91614, 0.91707, 0.91801, 0.91894, 0.91988, 0.92082, 0.92176, 0.9227, 0.92365, 0.9246, 0.92554, 0.92649, 0.92744, 0.92838, 0.92933, 0.93027, 0.93121, 0.93215, 0.93309, 0.93402, 0.93495, 0.93588, 0.9368, 0.93772, 0.93863, 0.93954, 0.94045, 0.94134, 0.94223, 0.94312, 0.944, 0.94487, 0.94573, 0.94658, 0.94743, 0.94827, 0.9491, 0.94992, 0.95073, 0.95154, 0.95233, 0.95312, 0.95389, 0.95465, 0.95541, 0.95615, 0.95689, 0.95761, 0.95833, 0.95903, 0.95972, 0.96041, 0.96108, 0.96175, 0.9624, 0.96305, 0.96369, 0.96432, 0.96494, 0.96555, 0.96616, 0.96676, 0.96735, 0.96794, 0.96852, 0.9691, 0.96968, 0.97026, 0.97083, 0.9714, 0.97198, 0.97255, 0.97313, 0.97371, 0.9743, 0.9749, 0.9755, 0.97612, 0.97675, 0.97739, 0.97804, 0.97872, 0.97941, 0.98013, 0.98087, 0.98164, 0.98243, 0.98326, 0.98413, 0.98503, 0.98597, 0.98695, 0.98798, 0.98906, 0.9902, 0.99139, 0.99264, 0.99396, 0.99534, 0.9968, 0.99833};

int errors[360] = {0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1};
float compass_zero = 0;


float compensate_compass(float compass_value) {
    float compensated_value = compass_value - errors[(int)compass_value];
    compensated_value -= compass_zero;
    compensated_value = 360 - compensated_value; // We want the value to increase when turning anticlockwise, as is convention
    if (compensated_value > 359.49) {
        compensated_value -= 360;
    } if (compensated_value < 0) {
        compensated_value += 360;
    }
    return compensated_value;
}

void *sensors_start(void *queues){
    uint8_t sonar_sn;
    uint8_t compass_sn;
    uint8_t gyro_sn;

    float compass_value;
    float sonar_value;
    float gyro_value;
    
    ev3_sensor_init();
    ev3_search_sensor(LEGO_EV3_US, &sonar_sn, 0);
    ev3_search_sensor(HT_NXT_COMPASS, &compass_sn, 0);
    ev3_search_sensor(LEGO_EV3_GYRO, &gyro_sn, 0);
    get_sensor_value0(compass_sn, &compass_zero);
    compass_zero -= errors[(int)compass_zero];
    printf("compass zero %f\n", compass_zero);
    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_sensor_to_main = tmp[0];

    while(1){
        get_sensor_value0(sonar_sn, &sonar_value);
        get_sensor_value0(gyro_sn, &gyro_value);
        get_sensor_value0(compass_sn, &compass_value);
        compass_value = compensate_compass(compass_value);
        //send to main
        send_message(queue_sensor_to_main, MESSAGE_SONAR, (int16_t)(sonar_value + 0.5));
        //send_message(queue_sensor_to_main, MESSAGE_GYRO, (int16_t)(gyro_value + 0.5));
        send_message(queue_sensor_to_main, MESSAGE_COMPASS, (int16_t)(compass_value + 0.5));
        //printf("compass: %d\n", (int16_t)(compass_value + 0.5));
        Sleep(10);
    }
}