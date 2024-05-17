
#include <gmCore/MathConstants.hh>
#include <gmGraphics/AABB.hh>
#include <gmGraphics/PoseTransform.hh>
#include <gmGraphics/IntersectionVisitor.hh>

using namespace gramods;

typedef gmGraphics::IntersectionLine Line;

TEST(gmGraphics, AABB_isIntersecting) {

  gmGraphics::AABB aabb(Eigen::Vector3f(2.f, 2.f, 2.f));
  aabb += Eigen::Vector3f(6.f, 6.f, 6.f);

  EXPECT_FALSE(
      aabb.isIntersecting(Line::lineSegment({0.f, 4.f, 4.f}, {1.f, 4.f, 4.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::lineSegment({1.f, 4.f, 4.f}, {4.f, 4.f, 4.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::lineSegment({3.f, 4.f, 4.f}, {5.f, 4.f, 4.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::lineSegment({4.f, 4.f, 4.f}, {7.f, 4.f, 4.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::lineSegment({7.f, 4.f, 4.f}, {8.f, 4.f, 4.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::lineSegment({1.f, 4.f, 4.f}, {8.f, 4.f, 4.f})));

  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({0.f, 4.f, 4.f}, {1.f, 0.f, 0.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({1.f, 4.f, 4.f}, {1.f, 0.f, 0.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({3.f, 4.f, 4.f}, {1.f, 0.f, 0.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({4.f, 4.f, 4.f}, {1.f, 0.f, 0.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::forwardRay({7.f, 4.f, 4.f}, {1.f, 0.f, 0.f})));

  EXPECT_FALSE(
      aabb.isIntersecting(Line::forwardRay({0.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::forwardRay({1.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({3.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({4.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({7.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));

  EXPECT_TRUE(
      aabb.isIntersecting(Line::forwardRay({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f})));

  EXPECT_TRUE(aabb.isIntersecting(
      Line::infiniteRay({0.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(aabb.isIntersecting(
      Line::infiniteRay({1.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(aabb.isIntersecting(
      Line::infiniteRay({3.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(aabb.isIntersecting(
      Line::infiniteRay({4.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_TRUE(aabb.isIntersecting(
      Line::infiniteRay({7.f, 4.f, 4.f}, {-1.f, 0.f, 0.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::infiniteRay({1.f, 4.f, 4.f}, {0.f, 1.f, 0.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::infiniteRay({1.f, 1.f, 1.f}, {1.f, 0.f, 0.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::infiniteRay({1.f, 1.f, 1.f}, {0.f, 1.f, 0.f})));
  EXPECT_FALSE(
      aabb.isIntersecting(Line::infiniteRay({1.f, 1.f, 1.f}, {0.f, 0.f, 1.f})));
}

TEST(gmGraphics, AABB_getIntersections) {

  gmGraphics::AABB aabb(Eigen::Vector3f(2.f, 2.f, 2.f));
  aabb += Eigen::Vector3f(6.f, 6.f, 6.f);

  auto epsilon = std::numeric_limits<float>::epsilon() * 6.f;

#define EXPECT_ZERO(LINE)                                                      \
  {                                                                            \
    auto data = aabb.getIntersections(LINE);                                   \
    EXPECT_EQ(data.size(), 0) << "aabb.getIntersections(" << #LINE << ")";     \
  }
#define EXPECT_ONE(LINE, XA)                                                   \
  {                                                                            \
    auto line = LINE;                                                          \
    auto data = aabb.getIntersections(line);                                   \
    EXPECT_EQ(data.size(), 1) << "aabb.getIntersections(" << #LINE << ")";     \
    if (data.size() > 0)                                                       \
      EXPECT_LE((line.getPosition(data[0]) - XA).norm(), epsilon)              \
          << "aabb.getIntersections(" << #LINE << ")[0]";                      \
  }
#define EXPECT_TWO(LINE, XA, XB)                                               \
  {                                                                            \
    auto line = LINE;                                                          \
    auto data = aabb.getIntersections(line);                                   \
    EXPECT_EQ(data.size(), 2);                                                 \
    if (data.size() > 0)                                                       \
      EXPECT_LE((line.getPosition(data[0]) - XA).norm(), epsilon)              \
          << "aabb.getIntersections(" << #LINE << ")[0]";                      \
    if (data.size() > 1)                                                       \
      EXPECT_LE((line.getPosition(data[1]) - XB).norm(), epsilon)              \
          << "aabb.getIntersections(" << #LINE << ")[1]";                      \
  }

  EXPECT_ZERO(Line::lineSegment({0.f, 4.f, 4.f}, {1.f, 4.f, 4.f}));
  EXPECT_ONE(Line::lineSegment({1.f, 4.f, 4.f}, {4.f, 4.f, 4.f}),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_ZERO(Line::lineSegment({3.f, 4.f, 4.f}, {5.f, 4.f, 4.f}));
  EXPECT_ONE(Line::lineSegment({4.f, 4.f, 4.f}, {7.f, 4.f, 4.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f));
  EXPECT_ZERO(Line::lineSegment({7.f, 4.f, 4.f}, {8.f, 4.f, 4.f}));
  EXPECT_TWO(Line::lineSegment({1.f, 4.f, 4.f}, {8.f, 4.f, 4.f}),
             Eigen::Vector3f(2.f, 4.f, 4.f),
             Eigen::Vector3f(6.f, 4.f, 4.f));

  EXPECT_TWO(Line::forwardRay({0.f, 4.f, 4.f}, {1.f, 0.f, 0.f}),
             Eigen::Vector3f(2.f, 4.f, 4.f),
             Eigen::Vector3f(6.f, 4.f, 4.f));
  EXPECT_TWO(Line::forwardRay({1.f, 4.f, 4.f}, {1.f, 0.f, 0.f}),
             Eigen::Vector3f(2.f, 4.f, 4.f),
             Eigen::Vector3f(6.f, 4.f, 4.f));
  EXPECT_ONE(Line::forwardRay({3.f, 4.f, 4.f}, {1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f));
  EXPECT_ONE(Line::forwardRay({4.f, 4.f, 4.f}, {1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f));
  EXPECT_ZERO(Line::forwardRay({7.f, 4.f, 4.f}, {1.f, 0.f, 0.f}));

  EXPECT_ZERO(Line::forwardRay({0.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}));
  EXPECT_ZERO(Line::forwardRay({1.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}));
  EXPECT_ONE(Line::forwardRay({3.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_ONE(Line::forwardRay({4.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_TWO(Line::forwardRay({7.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f),
             Eigen::Vector3f(2.f, 4.f, 4.f));

  EXPECT_TWO(Line::forwardRay({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}),
             Eigen::Vector3f(2.f, 2.f, 2.f),
             Eigen::Vector3f(6.f, 6.f, 6.f));

  EXPECT_TWO(Line::infiniteRay({0.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_TWO(Line::infiniteRay({1.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_TWO(Line::infiniteRay({3.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_TWO(Line::infiniteRay({4.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f),
             Eigen::Vector3f(2.f, 4.f, 4.f));
  EXPECT_TWO(Line::infiniteRay({7.f, 4.f, 4.f}, {-1.f, 0.f, 0.f}),
             Eigen::Vector3f(6.f, 4.f, 4.f),
             Eigen::Vector3f(2.f, 4.f, 4.f));

  EXPECT_ZERO(Line::infiniteRay({1.f, 4.f, 4.f}, {0.f, 1.f, 0.f}));
  EXPECT_ZERO(Line::infiniteRay({1.f, 1.f, 1.f}, {1.f, 0.f, 0.f}));
  EXPECT_ZERO(Line::infiniteRay({1.f, 1.f, 1.f}, {0.f, 1.f, 0.f}));
  EXPECT_ZERO(Line::infiniteRay({1.f, 1.f, 1.f}, {0.f, 0.f, 1.f}));

#undef EXPECT_ZERO
#undef EXPECT_ONE
#undef EXPECT_TWO
}

struct AabbNode : gmGraphics::Node {

  gmGraphics::AABB aabb = gmGraphics::AABB(Eigen::Vector3f(2.f, 2.f, 2.f));

  virtual std::vector<float>
  getIntersections(const Line &line) override {
    return aabb.getIntersections(line);
  }
};

TEST(gmGraphics, IntersectionVisitor_PoseTransform_AABB) {
  auto N0 = std::make_shared<AabbNode>();
  N0->aabb += Eigen::Vector3f(6.f, 6.f, 6.f);
  N0->initialize();

  auto T0 = std::make_shared<gmGraphics::PoseTransform>();
  T0->addNode(N0);
  T0->initialize();

  auto epsilon = std::numeric_limits<float>::epsilon() * 6.f;

#define EXPECT_ZERO(LINE)                                                      \
  {                                                                            \
    auto line = LINE;                                                          \
    gmGraphics::IntersectionVisitor visitor(line);                             \
    T0->accept(&visitor);                                                      \
    EXPECT_EQ(visitor.intersections.size(), 0) << "visitor(" << #LINE << ")";  \
  }
#define EXPECT_ONE(LINE, XAL, XAG)                                             \
  {                                                                            \
    auto line = LINE;                                                          \
    gmGraphics::IntersectionVisitor visitor(line);                             \
    T0->accept(&visitor);                                                      \
    EXPECT_EQ(visitor.intersections.size(), 1) << "visitor(" << #LINE << ")";  \
    if (visitor.intersections.size() > 0) {                                    \
      EXPECT_LE((visitor.intersections[0].local_position - XAL).norm(),        \
                epsilon)                                                       \
          << "visitor(" << #LINE << ")[0].local_position";                     \
      EXPECT_LE((visitor.intersections[0].position - XAG).norm(), epsilon)     \
          << "visitor(" << #LINE << ")[0].position";                           \
    }                                                                          \
  }
#define EXPECT_TWO(LINE, XAL, XAG, XBL, XBG)                                   \
  {                                                                            \
    auto line = LINE;                                                          \
    gmGraphics::IntersectionVisitor visitor(line);                             \
    T0->accept(&visitor);                                                      \
    EXPECT_EQ(visitor.intersections.size(), 2) << "visitor(" << #LINE << ")";  \
    if (visitor.intersections.size() > 0) {                                    \
      EXPECT_LE((visitor.intersections[0].local_position - XAL).norm(),        \
                epsilon)                                                       \
          << "visitor(" << #LINE << ")[0].local_position ("                    \
          << visitor.intersections[0].local_position.transpose()               \
          << " == " << XAL.transpose() << ")";                                 \
      EXPECT_LE((visitor.intersections[0].position - XAG).norm(), epsilon)     \
          << "visitor(" << #LINE << ")[0].position ("                          \
          << visitor.intersections[0].position.transpose()                     \
          << " == " << XAG.transpose() << ")";                                 \
    }                                                                          \
    if (visitor.intersections.size() > 1) {                                    \
      EXPECT_LE((visitor.intersections[1].local_position - XBL).norm(),        \
                epsilon)                                                       \
          << "visitor(" << #LINE << ")[1].local_position ("                    \
          << visitor.intersections[1].local_position.transpose()               \
          << " == " << XAL.transpose() << ")";                                 \
      EXPECT_LE((visitor.intersections[1].position - XBG).norm(), epsilon)     \
          << "visitor(" << #LINE << ")[1].position ("                          \
          << visitor.intersections[1].position.transpose()                     \
          << " == " << XAG.transpose() << ")";                                 \
    }                                                                          \
  }

  /*
    gmGraphics::AABB aabb { Eigen::Vector3f(2.f, 2.f, 2.f)
                            Eigen::Vector3f(6.f, 6.f, 6.f) }
  */

  T0->setPosition({0.f, 2.f, 0.f});

  EXPECT_ZERO(Line::lineSegment({0.f, 3.f, 4.f}, {8.f, 3.f, 4.f}));
  EXPECT_ZERO(Line::lineSegment({0.f, 9.f, 4.f}, {8.f, 9.f, 4.f}));
  EXPECT_ZERO(Line::lineSegment({1.f, 0.f, 4.f}, {1.f, 9.f, 4.f}));
  EXPECT_ZERO(Line::lineSegment({7.f, 0.f, 4.f}, {7.f, 9.f, 4.f}));
  EXPECT_TWO(Line::lineSegment({0.f, 7.f, 4.f}, {8.f, 7.f, 4.f}),
             Eigen::Vector3f(2.f, 5.f, 4.f),
             Eigen::Vector3f(2.f, 7.f, 4.f),
             Eigen::Vector3f(6.f, 5.f, 4.f),
             Eigen::Vector3f(6.f, 7.f, 4.f));

  T0->setOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));
  T0->setOrientationCenter({4.f, 4.f, 4.f});

  EXPECT_ZERO(Line::lineSegment({0.f, 3.f, 4.f}, {8.f, 3.f, 4.f}));
  EXPECT_ZERO(Line::lineSegment({0.f, 9.f, 4.f}, {8.f, 9.f, 4.f}));
  EXPECT_ZERO(Line::lineSegment({1.f, 0.f, 4.f}, {1.f, 9.f, 4.f}));
  EXPECT_ZERO(Line::lineSegment({7.f, 0.f, 4.f}, {7.f, 9.f, 4.f}));
  EXPECT_TWO(Line::lineSegment({0.f, 7.f, 4.f}, {8.f, 7.f, 4.f}),
             Eigen::Vector3f(2.f, 4.f, 3.f),
             Eigen::Vector3f(2.f, 7.f, 4.f),
             Eigen::Vector3f(6.f, 4.f, 3.f),
             Eigen::Vector3f(6.f, 7.f, 4.f));

}
