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
struct centroid cent_sen1[] = { { 55.0429f,   13.8353f,   16.2467f  },
                                { 512.197f,   156.469f,   171.469f  },
                                { 631.924f,   195.290f,   82.42075  },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen2[] = { { 52.7002f,   27.6542f,   22.6531f  },
                                { 547.4418f,  275.1181f,  240.702f  },
                                { 683.6942f,  303.9065f,  118.9787f },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen3[] = { { 69.7864f,   31.3532f,   30.2018f  },
                                { 600.123f,   350.248f,   296.2351f },
                                { 644.9321f,  312.3765f,  117.606f  },
                                { 2239.8f,    2219.5f,    2158.0f   } };

struct centroid cent_sen4[] = { { 61.3208f,   24.0464f,   22.4587f  },
                                { 526.1413f,  235.4206f,  22.7829f  },
                                { 684.4996f,  286.4242f,  103.9429f },
                                { 2239.8f,    2219.5f,   2158.0f    } };

struct centroid cent_sen5[] = { { 94.2126f,   39.4854f,  27.4833f   },
                                { 685.5562f,  331.4753f, 252.7719f  },
                                { 712.00495f, 327.00795f,105.6507f  },
                                { 2239.8f,    2219.5f,   2158.0f    } };

struct centroid cent_sen6[] = { { 86.0232f,   36.8853f,  29.5919f   },
                                { 655.5129f,  319.6518f, 283.9304f  },
                                { 749.5608f,  302.1528f, 126.9427f  },
                                { 2239.8f,    2219.5f,   2158.0f    } };

#endif 
