/**
   (c)2013, 2017 Karljohan Lundin Palmerius
   
   <COPYRIGHT NOTICE>
*/

#ifndef GRAMODS_TOUCH_EFFOAW
#define GRAMODS_TOUCH_EFFOAW

#include <gmTouch/config.hh>

#include <deque>
#include <map>
#include <cstddef>
#include <limits>

#include <math.h>

BEGIN_NAMESPACE_GMTOUCH;
  
  /**
     This is an end-fitting first-order adaptive window estimator of
     velocity from samples allowing jittering. Units of both length
     and time can be arbitrarily chosen and the velocity will be
     estimated as distance over time.
  */
  template<class VEC>
  class EFFOAW {
      
  public:

    /** Creates a new end-fitting first-order adaptive window
        estimator using the default parameters. */
    EFFOAW();
      
    /** Set how many samples should be saved to be used when
        estimating the velocity. */
    void setHistoryLength(size_t N){
      history_length = N; }
      
    /** Get the number of samples that are saved to be used when
        estimating the velocity. */
    size_t getHistoryLength() const {
      return history_length; }
      
    /** Set how old samples should be saved to be used when
        estimating the velocity. */
    void setHistoryDuration(double t){
      history_duration = t; }
      
    /** Get how old samples are saved to be used when estimating the
        velocity. */
    double getHistoryDuration() const {
      return history_duration; }
    
    /**
       Returns the time associated with the last sample associated
       with the specified id, or -1 if there is no last sample
       available.
    */
    double getLastSampleTime(size_t id) {
      if (history.find(id) == history.end() || history[id].first.size() < 1)
        return -1;
      return history[id].first[0];
    }
    
    /**
       Add a position sample. Each sample is associated to an id and a
       time. If a new sample is provided with the same id and time as
       a previously added sample, this new sample will replace the
       old.
         
       @param[in] id The id to add position sample to.
       @param[in] position The position value to use as sample.
       @param[in] time The time at which the sample was taken.
    */
    void addSample(size_t id, VEC position, double time);
      
    /**
       Estimate and return the velocity using samples associated to
       id. The velocity will be expressed in chosen units per second.
         
       @param[in] id The id to estimate velocity for.
       @param[in] error An estimate of the position tracking error, used
       to determine how many samples to include in the averaging.
       @param[out] samples The number of history samples used in the
       estimation.
    */
    template<class TYPE>
    VEC estimateVelocity(size_t id, TYPE error, size_t *samples = nullptr) const;

    /**
       Estimate and return the position using samples associated to
       id. This is not really a functionality of EFFOAW, but uses the
       EFFOAW estimated velocity as a means to estimate a position in
       the immediate future. It is assumed that the specified time
       occurs after or at the time of the last sample.
       
       @param[in] id The id to estimate position for.
       @param[in] error An estimate of the position tracking error, used
       to determine how many samples to include in the averaging.
       @param[in] time The time to estimate the position for.
       @param[out] samples The number of history samples used in the
       estimation.
    */
    template<class TYPE>
    VEC estimatePosition(size_t id, TYPE error, double time, size_t *samples = nullptr) const;
      
    /**
       Clean-up old samples and unused ids. This function should be
       called from time to time, for example after calling
       addSample, to avoid too much unnecessary memory usage.
         
       @param time The current time, to use when estimating if
       samples are too old, or -1 to use the samples' time.
    */
    void cleanup(double time = -1);
      
  private:
      
    /** The number of samples that are saved. */
    size_t history_length;
      
    /** How old samples should be saved. */
    double history_duration;
      
    /** Type of list containing time values. */
    typedef std::deque<double> time_list_t;
      
    /** Type of list containing position values. */
    typedef std::deque<VEC> position_list_t;
      
    /** Type combining list of time values with list of position values. */
    typedef std::pair<time_list_t, position_list_t> time_position_t;
      
    /** History of points, per id. */
    std::map<size_t, time_position_t> history;
      
  };
}

template<class VEC>
gmTouch::EFFOAW<VEC>::EFFOAW()
  : history_length(10),
    history_duration(1.f) {}

template<class VEC>
void gmTouch::EFFOAW<VEC>::addSample(size_t id, VEC position, double time){
  
  time_list_t &time_list = history[id].first;
  position_list_t &position_list = history[id].second;

  // Remove old samples that have the same time (override old sample)
  while (! time_list.empty() && fabs(time - time_list.front()) < std::numeric_limits<double>::epsilon()) {
    position_list.pop_front();
    time_list.pop_front();
  }

  position_list.push_front(position);
  time_list.push_front(time);
  
  while( position_list.size() > history_length ){
    position_list.pop_back();
    time_list.pop_back();
  }
  
  while( !time_list.empty() && time - time_list.back() > history_duration ){
    position_list.pop_back();
    time_list.pop_back();
  }
}

template<class VEC>
template<class TYPE>
VEC gmTouch::EFFOAW<VEC>::estimateVelocity(size_t id, TYPE error_threshold, size_t *samples) const {

  typename std::map< size_t, time_position_t >::const_iterator hist = history.find(id);
  if( hist == history.end() ){
    if (samples != nullptr)
      *samples = 0;
    return VEC(0);
  }
  
  const time_list_t & time_list = hist->second.first;
  const position_list_t & position_list = hist->second.second;
  
  if( position_list.size() < 2 ){ 
    if (samples != nullptr)
      *samples = 0;
    return VEC(0);
  }
  
  int best_valid_n = 1;
  for( int n = 2 ; n < position_list.size() ; ++n ){
    
    VEC diff = ( position_list[0] - position_list[n] )/( time_list[0] - time_list[n] );
    
    bool is_valid = true;
    for( int i = 1 ; i <= n ; ++i ){
      VEC offset = ( time_list[0] - time_list[i] ) * diff;
      TYPE est = ( position_list[0] - position_list[i] - offset ).length();
      if( est > error_threshold ){
        is_valid = false;
        break;
      }
    }
    
    if( is_valid ){
      best_valid_n = n; }
    else {
      break; }
  }

  if (samples != nullptr)
    *samples = best_valid_n + 1;
  
  return ( (position_list[0] - position_list[best_valid_n])
           / ( time_list[0] - time_list[best_valid_n] ) );
}

template<class VEC>
template<class TYPE>
VEC gmTouch::EFFOAW<VEC>::estimatePosition(size_t id, TYPE error_threshold, double time, size_t *ret_samples) const {

  typename std::map< size_t, time_position_t >::const_iterator hist = history.find(id);
  if( hist == history.end() ){
    if (ret_samples != nullptr)
      *ret_samples = 0;
    return VEC(0);
  }
  
  const time_list_t & time_list = hist->second.first;
  const position_list_t & position_list = hist->second.second;
  
  if (position_list.size() == 0) {
    if (ret_samples != nullptr)
      *ret_samples = 0;
    return VEC(0);
  }
  
  size_t samples;
  VEC velocity = estimateVelocity(id, error_threshold, &samples);

  // Two samples should give position identical to p0, minus floating
  // point errors
  if (samples <= 2) {
    if (ret_samples != nullptr)
      *ret_samples = 1;
    return position_list[0];
  }
  
  if (ret_samples != nullptr)
    *ret_samples = samples;
  
  double sum_time = 0.0;
  VEC sum_position(0);
  for (int idx = 0; idx < samples; ++idx) {
    sum_time += time_list[idx];
    sum_position += position_list[idx];
  }

  double avg_time = sum_time / samples;
  VEC avg_position = sum_position * (1.f/samples);

  if (ret_samples != nullptr)
    *ret_samples = samples;
  
  return avg_position + (time - avg_time) * velocity;
}

template<class VEC>
void gmTouch::EFFOAW<VEC>::cleanup(double time){

  if (time < 0)
    for (auto it : history)
      if (it.second.first.size() > 0 && it.second.first[0] > time)
        time = it.second.first[0];
  
  for( typename std::map< size_t, time_position_t >::iterator it = history.begin() ;
       it != history.end() ; ){
    
    time_list_t &time_list = it->second.first;
    position_list_t &position_list = it->second.second;
    
    while( !time_list.empty() && time_list.size() > history_length ){
      position_list.pop_back();
      time_list.pop_back();
    }
    
    while( !time_list.empty() && ( time - time_list.back() ) > history_duration ){
      position_list.pop_back();
      time_list.pop_back();
    }
    
    typename std::map< size_t, time_position_t >::iterator current_it = it;
    it++;
    
    if( time_list.empty() ){
      history.erase(current_it);
    }
  }

END_NAMESPACE_GMTOUCH;

#endif
