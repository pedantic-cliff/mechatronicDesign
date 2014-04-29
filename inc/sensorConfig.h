#ifndef _SENSOR_CONFIG_H_
#define _SENSOR_CONFIG_H_

typedef struct centroid{ 
  float r; 
  float g; 
  float b;
} centroid_t; 

// Metal 
// Yellow
struct centroid cent_sen1[] = { { 0.0623f,  0.1179f,  0.9114f },
                                { 0.9936f,  0.9912f,  0.0221f } };

struct centroid cent_sen2[] = { { 0.0523f,  0.8682f,  0.9491f },
                                { 0.9042f,  0.1227f,  0.0317f } };

struct centroid cent_sen3[] = { { 0.5170f,  0.8554f,  0.9359f },
                                { 0.2411f,  0.0717f,  0.0619f } };

struct centroid cent_sen4[] = { { 0.2038f,  0.2796f,  0.7005f },
                                { 0.9857f,  0.9820f,  0.0182f } };

struct centroid cent_sen5[] = { { 0.3125f,  0.4796f,  0.8659f },
                                { 0.9611f,  0.6694f,  0.0296f } };

struct centroid cent_sen6[] = { { 0.5234f,  0.5307f,  0.8732f },
                                { 0.7334f,  0.1630f,  0.1154f } };

struct centroid sen_mins[]  = { { 448.97f,  114.91f,  98.660f }, 
                                { 535.48f,  278.84f,  122.12f }, 
                                { 504.17f,  214.47f,  85.123f }, 
                                { 431.99f,  172.63f,  125.20f }, 
                                { 582.69f,  283.86f,  120.17f }, 
                                { 542.60f,  222.05f,  93.469f } }; 

struct centroid sen_maxs[]  = { { 189.56f,  92.268f,  58.878f }, 
                                { 132.59f,  41.994f,  126.24f }, 
                                { 50.830f,  70.850f,  174.88f }, 
                                { 262.48f,  126.83f,  103.16f }, 
                                { 119.64f,  49.074f,  139.54f }, 
                                { 117.56f,  97.221f,  191.75f } }; 

#endif 
