////shamelessly coppied from https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
//
//#ifndef THREAD_POOL_H
//#define THREAD_POOL_H
//
//#include <vector>
//#include <queue>
//#include <functional>
//#include <mutex>
//#include <thread>
//#include <glm/glm.hpp>
//
//using namespace std;
//using namespace glm;
//
//class ThreadPool {
//public:
//    void Start();
//    void QueueJob(const ivec3& job);
//    void Stop();
//    bool busy();
//
//private:
//    void ThreadLoop();
//
//    bool should_terminate = false;           // Tells threads to stop looking for jobs
//    std::mutex queue_mutex;                  // Prevents data races to the job queue
//    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
//    std::vector<std::thread> threads;
//    std::queue<ivec3> jobs;
//};
//
//
//#endif
