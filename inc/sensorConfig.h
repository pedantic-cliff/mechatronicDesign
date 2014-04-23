#ifndef _SENSOR_CONFIG_H_
#define _SENSOR_CONFIG_H_

typedef struct centroid{ 
  float r; 
  float g; 
  float b;
} centroid_t; 

// Boundary
// Metal 
// Yellow
// *unused*
struct centroid cent_sen1[] = { { 20.3476f,   24.9848f,   32.8168f  },
                                { 504.8492f,  164.6696f,  175.5200f },
                                { 593.3808f,  169.5852f,  93.9536f  },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen2[] = { { 9.0712f,    18.2656f,   28.5484f  },
                                { 563.576f,   314.728f,   252.4576f },
                                { 646.2884f,  294.6868f,  137.3836f },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen3[] = { { 39.6652f,   22.9736f,   16.1944f  },
                                { 528.9228f,  279.3492f,  246.348f  },
                                { 622.5508f,  278.5340f,  128.0524f },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen4[] = { { 4.846f,     10.7836f,   20.4683f  },
                                { 587.6532f,  270.662f,   242.5528f },
                                { 649.5108f,  267.7844f,  121.8856f },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen5[] = { { 7.6296f,    16.1092f,  27.2552f   },
                                { 697.9676f,  341.2996f, 268.9824f  },
                                { 669.7268f,  306.7811f, 126.0228f  },
                                { 2239.8f,    2219.5f,   2158.0f    } };

struct centroid cent_sen6[] = { { 5.7876f,    19.5964f,   40.2176f  },
                                { 583.1836f,  263.3447f,  260.4156f },
                                { 598.9264f,  217.8180f,  115.1432f },
                                { 2239.8f,    2219.5f,    2158.0f   } };

#endif 
