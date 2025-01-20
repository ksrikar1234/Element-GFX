#pragma once
#ifndef _GRIDPRO_GUI_API_INSTRUMENTATION_
#define _GRIDPRO_GUI_API_INSTRUMENTATION_

#include <ctime>
#include <thread>
#include <iostream>
#include <chrono>
#include <cstdio>


#include "gp_gui_debug.h"


namespace gridpro_gui
{
namespace Instrumentation {	

///////////////////////////////////////////////////////    
////////// A Scoped StopWatch
//////////////////////////////////////////////////////
///// Usage :
///// Stopwatch watch("task name"); 
//////////////////////////////////////////////////////    

    class Stopwatch {
    public :
    inline explicit Stopwatch(const char* taskName);
    inline ~Stopwatch();
    inline double end();
    inline double reset();
    inline double subtask(const char* subtask_name);
    inline double elapsedSeconds() const;

    private:
    // Print time taken
    inline double get_time_taken(); 
    // Data members
    const char* taskName;
    std::chrono::high_resolution_clock::time_point startTime;
    double last_subtask_time;
    bool isRunning = true;
    };


    // Constructor
    inline Stopwatch::Stopwatch(const char* taskName) : taskName(taskName), startTime(std::chrono::high_resolution_clock::now()), last_subtask_time(0.0f) {
            printf("Task %s started.\n", taskName);
    }

    // Destructor
    inline Stopwatch::~Stopwatch() {
        if(isRunning) get_time_taken();
    }

    // Explicitly end the stopwatch
    inline double Stopwatch::end() {
        isRunning = false;
        return get_time_taken();
    }

    // Reset the clock
    inline double Stopwatch::reset() {
        double elapsedTime = get_time_taken();  // Record the time taken until this point
        startTime = std::chrono::high_resolution_clock::now();
        printf("Stopwatch reset for task '%s'.\n", taskName);
        return elapsedTime;  // Return the time taken for the previous task
    }

    // log subtasks
    inline double Stopwatch::subtask(const char* subtask_name) {
        last_subtask_time = elapsedSeconds() - last_subtask_time;  // Record the time taken until this point
        printf("Time taken for subtask '%s' Primary task = '%s'.\n", subtask_name, taskName);
        return last_subtask_time;  // Return the time taken for the previous task
    }        

    // Get elapsed time in seconds as a double
    inline double Stopwatch::elapsedSeconds() const {
        std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
        std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        return duration.count() / 1.0e6;
    }
        
    // Print time taken
    inline double Stopwatch::get_time_taken() {
            std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
            //DEBUG_PRINT("Task ", taskName, " completed. Time taken: ", taskName, elapsedSeconds(), "s ,", duration.count(), "ns");
            std::cout << "Task " << taskName << " completed. Time taken: " << elapsedSeconds() << "s ," << duration.count() << "ns" << "\n"; 
            return elapsedSeconds();
        }
///////////////////////////////////////////////////////    
////////// A Frame Counter
//////////////////////////////////////////////////////
///// Usage :
///// FPSCounter fpscounter; 
////  fps.start();
////  ----------
////  fps.end();
//////////////////////////////////////////////////////    

    class FPSCounter {

    public:
         
         FPSCounter() { targetFPS = 1; frameDelay = (1000 / targetFPS);}

        ~FPSCounter() {}
        
         void start() {
             startTime = clock();
             numFrames = 0;
         }

         void end() {
		
             if (++numFrames % targetFPS == 0) {
                 clock_t endTime = clock();
                 currentFPS = calculateFPS(startTime, endTime, numFrames);
                 startTime = clock();
                 numFrames = 0;
              }
          }

         double getCurrentFPS() const {
                return currentFPS;
          }

         void printFPS(double fps) const {
               std::cout << "FPS: " << fps << std::endl;
          }

    private:
    int targetFPS;
    int frameDelay;
    clock_t startTime;
    int numFrames;
    double currentFPS;

    double calculateFPS(clock_t startTime, clock_t endTime, int numFrames) const {
        return static_cast<double>(numFrames) / ((endTime - startTime) / static_cast<double>(CLOCKS_PER_SEC));
    }

};
} // Instrumentation
} // GridPro_gui

#endif