/**
   (c)2013, 2017 Karljohan Lundin Palmerius
   
   <COPYRIGHT NOTICE>
*/

#ifndef __TOUCHLIB_EFFOAW_HH__
#define __TOUCHLIB_EFFOAW_HH__

#include <deque>
#include <map>
#include <cstddef>

namespace touchlib {
  
  /**
     This is an end-fitting first-order adaptive window estimator of
     velocity from samples allowing jittering.
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
       Add a position sample. Each sample is associated to an id
       and a time.
         
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
       samples are too old.
    */
    void cleanup(double time);
      
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
touchlib::EFFOAW<VEC>::EFFOAW()
  : history_length(10),
    history_duration(1.f) {}

template<class VEC>
void touchlib::EFFOAW<VEC>::addSample(size_t id, VEC position, double time){
  
  time_list_t &time_list = history[id].first;
  position_list_t &position_list = history[id].second;
  
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
VEC touchlib::EFFOAW<VEC>::estimateVelocity(size_t id, TYPE error_threshold, size_t *samples) const {

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
VEC touchlib::EFFOAW<VEC>::estimatePosition(size_t id, TYPE error_threshold, double time, size_t *ret_samples) const {

  typename std::map< size_t, time_position_t >::const_iterator hist = history.find(id);
  if( hist == history.end() ){
    return VEC(0); }
  
  const time_list_t & time_list = hist->second.first;
  const position_list_t & position_list = hist->second.second;
  
  size_t samples;
  VEC velocity = estimateVelocity(id, error_threshold, &samples);

  double sum_dtime = 0.0;
  VEC sum_position(0);
  for (int idx = 0; idx < samples; ++idx) {
    sum_dtime += time_list[idx] - time_list[0];
    sum_position += position_list[idx];
  }

  double avg_dtime = sum_dtime / samples;
  VEC avg_position = sum_position * (1.f/samples);

  if (ret_samples != nullptr)
    *ret_samples = samples;
  
  return avg_position + (time - time_list[0] - avg_dtime) * velocity;
}

template<class VEC>
void touchlib::EFFOAW<VEC>::cleanup(double time){
  
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
}

#endif
