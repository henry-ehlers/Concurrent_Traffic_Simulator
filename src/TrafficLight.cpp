#include <iostream>
#include <random>
#include <chrono>
#include <random>
#include <thread>
#include <future>

#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
// THE NAMING IS STUPID: 'RECEIVE' GETS, WHEREAS 'SEND' SETS

template <typename T>
T MessageQueue<T>::receive() {
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	// SOURCE: https://knowledge.udacity.com/questions/116321
  
  	// LOCK OFF ACCESS TO MESSAGE QUEUE 
  	std::cout << "RECEIVING\n";
  	std::unique_lock<std::mutex> a_lock(_mutex);
  
  	// START THREAD BUT ONLY CONTINUE WHEN PROMSISE -> FUTURE (?)
  	// FROM: https://knowledge.udacity.com/questions/369653
  	std::cout << "wainting\n";
 	_cond.wait( a_lock, [this] { return !_queue.empty(); } );
  	std::cout << "done waiting\n";
  
  	// GET MESSAGE FROM QUEUE (USING MOVE SEMANTICS)
  	T message = std::move(_queue.back());
  	
  	// RESET QUEUE
	_queue.clear();
  	
  	// RETURN THE MESSAGE
	return message;
  
  	// UNIQUE_LOCK UNLOCK AUTOMATICALLY WHEN IT GOES OUT OF SCOPE
};

template <typename T>
void MessageQueue<T>::send(T &&message) {
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	// SOURCE: https://knowledge.udacity.com/questions/281826
  
  	// LOCK OFF ACCESS TO MESSAGE QUEUE 
  	std::unique_lock<std::mutex> a_lock(_mutex);
  
  	// ADD MESSAGE TO QUEUE
  	_queue.push_back(std::move(message));
  
  	// NOTIFY (?)
  	_cond.notify_one();
  
  	// UNIQUE_LOCK UNLOCK AUTOMATICALLY WHEN IT GOES OUT OF SCOPE
};


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	TrafficLightPhase phase;
  	std::cout << "in waitForGreen\n";
  	while (true) {
    	phase = _messageQueue.receive();
      	if (phase == TrafficLightPhase::green){
        	// break;
          	return;
        }
    };
}


void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  
  	// SOURCE: https://knowledge.udacity.com/questions/395593
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
	// FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  	
  	// SETUP RANDOM NUMBER GENERATOR 
  	// SOURCE: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
  	std::random_device rd; 										// obtain a random number from hardware
    std::mt19937 gen(rd()); 									// seed the generator
    std::uniform_int_distribution<> distr(4000000, 6000000); 	// define the range
  
  	// INITIALIZE
 	std::chrono::steady_clock::time_point t_begin = std::chrono::steady_clock::now();
  	int loop_time = distr(gen);
	
  	// INFINITE LOOP
    while (true) {
		
    	// MEASURE TIME
      	// SOURCE: https://stackoverflow.com/questions/2808398/easily-measure-elapsed-time
      	std::chrono::steady_clock::time_point t_end   = std::chrono::steady_clock::now();
      	int elapsed_micro_sec = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_begin).count();
      	
    	// CHECK IF ENOUGH TIME HAS ELAPSED
      	if (elapsed_micro_sec >= loop_time) {
          
          	// TOGGLE TRAFFIC LIGHT (COULD USE TERNARY HERE)
          	// MAYBE ADD MUTEX / LOCK HERE?
        	if (this->getCurrentPhase() == TrafficLightPhase::red)  {
    			_currentPhase = TrafficLightPhase::green;
    		} else if (this->getCurrentPhase() == TrafficLightPhase::green) {
    			_currentPhase = TrafficLightPhase::red;
    		}
          	this->_messageQueue.send(std::move(_currentPhase));
          
          	// GENERATE NEW TIME INTERVAL AND STARTING POINT
          	loop_time = distr(gen);
          	t_begin = std::chrono::steady_clock::now();
          
        };
      
      // SLEEP TO ENSURE THE PROGRAM CAN BE CLOSED (?)
      // SOURCE: https://knowledge.udacity.com/questions/514784
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      
	};
}

