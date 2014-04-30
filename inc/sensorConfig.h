#ifndef _SENSOR_CONFIG_H_
#define _SENSOR_CONFIG_H_

typedef struct centroid{ 
  float r; 
  float b;
} centroid_t; 

typedef struct {
  float m; 
  float c; 
} line; 

// Metal 
// Yellow
/*struct centroid cent_sen1[] = { { 0.0623f,  0.1179f,  0.9114f },
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
*/
struct centroid sen_mins[]  = { { 339.92f, 74.48f  }, 
                                { 260.40f, 134.16f }, 
                                { 186.76f, 72.96f  }, 
                                { 340.4f,  77.60f  }, 
                                { 499.36f, 118.2f  }, 
                                { 453.68f, 107.04f } }; 

struct centroid sen_maxs[]  = { { 267.2f,  104.16f }, 
                                { 410.8f,  132.76f }, 
                                { 481.6f,  203.36f }, 
                                { 358.32f, 194.40f }, 
                                { 255.88f, 177.20f }, 
                                { 313.76f, 201.12f } }; 

line lineOne[] = { { 1.0f,  -0.25f },
                   { 1.4f,  -0.91f },
                   { 1.08f, -0.38f },
                   { 0.71f, -0.11f },
                   { 0.75f, -0.15f },
                   { 0.75f, -0.09f } };

line lineTwo[] = { { 1.0f,  0.05f  },
                   { 1.4f,  -0.49f },
                   { 1.08f, -0.18f },
                   { 0.71f, 0.09f  }, 
                   { 0.75f, 0.05f  },
                   { 0.75f, 0.19f  } };

#endif 
