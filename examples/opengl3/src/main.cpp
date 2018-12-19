#include <iostream>

#include "opengl3_base.h"
#include "test_app.h"

#include "nurbs/nurbs.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include <cmath>

using namespace std;
// Unit circle
nurbs::RationalCurve3f getCurve() {
  nurbs::RationalCurve3f crv;
  crv.control_points = {
    glm::vec3(  0,  0,  0),
    glm::vec3(  1,  0,  0),
    glm::vec3(  2,  0,  0),
    glm::vec3(  3,  0,  0)
  };
  const float sqrt2_over_2 = std::sqrt(2.f) / 2.f;
  crv.weights = {
    1,
    1,
    1,
    1
  };
  crv.knots = {
    0, 0, 0, 0.5, 1, 1, 1
  };
  crv.degree = 2;
  return crv;
}

nurbs::RationalSurface3f getHemisphere() {
  nurbs::RationalSurface3f srf;
  srf.degree_u = 3;
  srf.degree_v = 3;
  srf.knots_u = { 0, 0, 0, 0, 1, 1, 1, 1 };
  srf.knots_v = { 0, 0, 0, 0, 1, 1, 1, 1 };
  srf.control_points = { 4, 4,
                        {glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1),
                         glm::vec3(2, 0, 1), glm::vec3(2, 4, 1),  glm::vec3(-2, 4, 1),  glm::vec3(-2, 0, 1),
                         glm::vec3(2, 0, -1), glm::vec3(2, 4, -1), glm::vec3(-2, 4, -1), glm::vec3(-2, 0, -1),
                         glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, -1)
                        }
  };
  srf.weights = { 4, 4,
                 {1,       1.f / 3.f, 1.f / 3.f, 1,
                  1.f / 3.f, 1.f / 9.f, 1.f / 9.f, 1.f / 3.f,
                  1.f / 3.f, 1.f / 9.f, 1.f / 9.f, 1.f / 3.f,
                  1,       1.f / 3.f, 1.f / 3.f, 1
                 }
  };
  return srf;
}

int main(int argc, char * argv[]) {
  auto crv = getCurve();
  std::vector<glm::vec3> curve;
  for (float para_u = 0.f; para_u <= 1.f + std::numeric_limits<float>::epsilon(); para_u += 0.1f)
  {
    glm::vec3 pt1 = nurbs::CurvePoint(crv, para_u);
    curve.push_back(pt1);

  }

  auto srf = getHemisphere();
  nurbs::array2<glm::vec3> surface(11, 11);
  for (float para_v = 0.f; para_v <= 1.f + std::numeric_limits<float>::epsilon(); para_v += 0.1f)
  {
    for (float para_u = 0.f; para_u <= 1.f + std::numeric_limits<float>::epsilon(); para_u += 0.1f)
    {
      glm::vec3 pt2 = nurbs::SurfacePoint(srf, para_u, para_v);
      surface(para_u, para_v) = pt2;
    }
  }
  int a = 0;

  try
  {
    vktuto::TestApp test_app;
    test_app.Run();
  }
  catch (const std::exception& except)
  {
    std::cerr << "[Error] catched: "
              << except.what() << std::endl; 
  }

  return EXIT_SUCCESS;
}