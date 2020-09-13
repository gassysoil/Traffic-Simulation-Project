#include <iostream>
#include <random>
#include "TrafficLight.h"

using namespace std;

/* Implementation of class "MessageQueue" */


    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

template <typename T>
T MessageQueue<T>::receive()
{
    unique_lock<mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); }); 
    
   //Refer this portion to: https://github.com/pdagger/CppND-Concurrent-Traffic-Simulation/blob/master/src/TrafficLight.cpp
    T msg = move(_queue.back());
    _queue.pop_back();

   return msg;
}

    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
   // Though this part is pretty straightforward, I refer this nice implementation to:
   //https://github.com/pdagger/CppND-Concurrent-Traffic-Simulation/blob/master/src/TrafficLight.cpp
    lock_guard<mutex> uLock(_mutex);
    _queue.push_back(move(msg));
    _condition.notify_one();
  
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}


    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

void TrafficLight::waitForGreen()
{
    while (true)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        if (_honglvdengQ.receive() == TrafficLightPhase::green) return;
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
void TrafficLight::simulate()
{
   threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
// virtual function which is executed in a thread

void TrafficLight::cycleThroughPhases(){
    random_device rnd_dev;
    default_random_engine d_r_e(rnd_dev()); // generate random number here
    uniform_real_distribution<float> uni_dist(4, 6); // unitform distribution here
    float last_cycle_duration = uni_dist(d_r_e);
    chrono::time_point<chrono::system_clock> last_update;
  
  	last_update = chrono::system_clock::now();
    while (true) 
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        long Tsince_last_update = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - last_update).count(); 
      
        if (Tsince_last_update >= last_cycle_duration){
            if (TrafficLight::getCurrentPhase() == red) {
                this->_currentPhase = green;
            }
            else {
                this->_currentPhase = red;
            }
          
            _honglvdengQ.send(move(_currentPhase));
            last_update = chrono::system_clock::now();
            last_cycle_duration = uni_dist(d_r_e);
        }
    }
}
