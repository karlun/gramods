
#ifndef GM_TRACK_BASE_ESTIMATOR
#define GM_TRACK_BASE_ESTIMATOR

#include <gmTrack/Controller.hh>


class BaseEstimator {
public:

  BaseEstimator();

  int process();

  void setScale(bool on) { scale = on; }

  void setUniform(bool on) { uniform = on; }

  void setController(std::shared_ptr<gramods::gmTrack::Controller> controller) {
    this->controller = controller;
  }

  Eigen::Matrix4f getBase() { return base; }

private:

  int getSamples(std::vector<Eigen::Vector3f> &samples);
  int getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x);

  int printUnitBase(Eigen::Vector3f origin_position,
                    Eigen::Vector3f x_position,
                    Eigen::Vector3f y_position);

  int printScaleBase(Eigen::Vector3f origin_position,
                     Eigen::Vector3f x_position,
                     Eigen::Vector3f y_position,
                     Eigen::Vector3f z_position);

  int printUniformBase(Eigen::Vector3f origin_position,
                       Eigen::Vector3f x_position,
                       Eigen::Vector3f y_position,
                       Eigen::Vector3f z_position);

  bool scale;
  bool uniform;
  std::shared_ptr<gramods::gmTrack::Controller> controller;

  Eigen::Matrix4f base;

};

#endif
