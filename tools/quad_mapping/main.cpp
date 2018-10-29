#include <iostream>
#include <cmath>
#include "targa.h"

// ARGB
using Color =int32_t;
int32_t Black = 0xFF000000;
int32_t White = 0xFFFFFFFF;
int32_t Red = 0xFFFF0000;
int32_t Green = 0xFF00FF00;
int32_t Blue = 0xFF0000FF;
const int32_t W = 320;
const int32_t H = 201;
int32_t framebuffer[W * H];

struct Texture {
    Texture(const std::string& filename) {
      int load = tga_read(&data, filename.c_str());
      if (load != TGA_NOERR) {
        exit(1);
      }
    }

    Color sample(float u, float v) {
//      u = round(u);
//      v = round(v);
      if (u < 0) u = 0;
      if (u > 1) u = 1;
      if (v < 0) v = 0;
      if (v > 1) v = 1;
      assert(u >= 0);
      assert(u <= 1);
      assert(v >= 0);
      assert(v <= 1);
      int img_u = u * (float)(data.width-1);
      int img_v = v * (float)(data.height-1);
      int32_t offset = (img_u + data.width * img_v) * 4;
      int* color= (int32_t*)&data.image_data[offset];
//      printf("Sampled:%.8X", color);
      return *color;
    }

    tga_image data;
};
struct Point {
    float x;
    float y;
    float z;

    float u,v;
};

/*
 *         0------------1
 *         |            |
 *         |            |
 *         |            |
 *         |            |
 *         3------------2
 */
struct Quad {
    Quad();
    Quad(Point& p0, Point& p1, Point& p2, Point& p3) {
      points[0] = p0;
      points[1] = p1;
      points[2] = p2;
      points[3] = p3;
    }

    Quad(Point p0, Point p1, Point p2, Point p3) {
      points[0] = p0;
      points[1] = p1;
      points[2] = p2;
      points[3] = p3;
    }

    void perspectiveDivive() {
      for (int i =0 ; i < 4 ; i++) {
        points[i].x /= points[i].z;
        points[i].y /= points[i].z;
//        printf("x=%.2f, y=%.2f\n", points[i].x, points[i].y);
      }
    }

    int width() {
      return points[1].x - points[0].x;
    }

    int deltaYTop() {
      return points[1]. y - points[0].y ;
    }

    int deltaYBotton() {
      return points[2]. y - points[3].y ;
    }

    Point points[4];
};


void PlotColor(int x, int y , Color color){
  int spx, spy;
  spx = x + W/2;
  spy = y + H/2;
  spy = (H) - spy;

  framebuffer[spx + spy * W] = color;
}
void Plot(Point& point , Texture texture){
  PlotColor(point.x, point.y, texture.sample(point.u, point.v));
}

void DrawColumnPerspective(Point& start, Point& end, Texture& texture) {

}

void DrawColumnAffine(Point& start, Point& end, Texture& texture) {
  PlotColor(start.x, start.y, Green);
  PlotColor(end.x, end.y, Green);
  Point cursor = start;
  int height = (int)start.y - (int)end.y ;
  printf("height: %d\n", height);
  int y=start.y;
  for (int i=0 ; i < height +1; i++) {
    cursor.y = y;
    cursor.v = i / (float)(height);
    printf("y=%d, v=%.2f\n", y, cursor.v);
    Plot(cursor, texture);
    y--;
  }
}

void drawQuad(Quad& quad, Texture texture) {
  float width = quad.width()+1;
  float deltaYTop = quad.deltaYTop() / (width );
  float deltaYBotton = quad.deltaYBotton() / (width );

  Point start = quad.points[0];
  Point end = quad.points[3];

  for (int i = 0 ; i < width ; i++) {

    // Affine texturing
    start.u = end.u = i / (width-1);

    // Perspective-correct (not working)
//    float alpha = i / (width-1);
//    start.u = end.u = (alpha/end.z) / ((1-alpha)/start.z + alpha /end.z);

    printf("u=%.2f\n", end.u);

    DrawColumnAffine(start, end, texture);
//    DrawColumnPerspective(start, end_column, texture);
    start.y += deltaYTop;
    end.y += deltaYBotton;
    start.x += 1;
    end.x += 1;
  }
}

int main() {
  for (size_t i = 0; i < W * H; i++) {
    framebuffer[i] = Black;
  }

//  Texture red("../MWALL4_1.tga");
  Texture red("../red.tga");
  Quad quad({-80, 50, 0.5, 0, 0}, {25,50, 2, 1, 0}, {25, -50, 2, 1, 1}, {-80, -50, 0.5, 0, 1});
  quad.perspectiveDivive();
  drawQuad(quad, red);

  Texture blue("../bluec.tga");
  Quad quadBlue({25, 50, 2, 0, 0}, {125, 50, 2, 1, 0}, {125, -50, 2, 1, 1}, {25, -50, 2, 0, 1});
  quadBlue.perspectiveDivive();
  drawQuad(quadBlue, blue);
//
  Texture green("../green.tga");
  Quad quadGreen({125, 50, 2, 0, 0}, {160, 50, 1, 1, 0}, {160, -50, 1, 1, 1}, {125, -50, 2, 0, 1});
  quadGreen.perspectiveDivive();
  drawQuad(quadGreen, green);

  tga_write_bgr("test.tga", (uint8_t *) framebuffer, W, H, 32);
  return 0;
}