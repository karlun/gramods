
#ifndef GM_TRACK_BASE_ESTIMATOR
#define GM_TRACK_BASE_ESTIMATOR

#include <gmTrack/Controller.hh>


class BaseEstimator {
public:

  enum struct Mode {
    UNIT,
      FREE_SCALE,
      UNIFORM_SCALE
  };

  BaseEstimator();

  int process();

  void setMode(Mode m) { mode = m; }

  void setController(std::shared_ptr<gramods::gmTrack::Controller> controller) {
    this->controller = controller;
  }

  Eigen::Matrix4f getBase() { return base; }

private:

  int getSamples(std::vector<Eigen::Vector3f> &samples);
  int getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x);

  int estimateUnitBase(Eigen::Vector3f origin_position,
                       Eigen::Vector3f x_position,
                       Eigen::Vector3f y_position);

  int estimateFreeScaleBase(Eigen::Vector3f origin_position,
                            Eigen::Vector3f x_position,
                            Eigen::Vector3f y_position,
                            Eigen::Vector3f z_position);

  int estimateUniformScaleBase(Eigen::Vector3f origin_position,
                               Eigen::Vector3f x_position,
                               Eigen::Vector3f y_position,
                               Eigen::Vector3f z_position);

  Mode mode;
  std::shared_ptr<gramods::gmTrack::Controller> controller;

  Eigen::Matrix4f base;

};

#endif
