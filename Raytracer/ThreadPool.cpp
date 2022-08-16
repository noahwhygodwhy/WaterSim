//#include "ThreadPool.hpp"
//
//
//
//void ThreadPool::ThreadLoop() {
//    while (true) {
//        ivec3 job;
//        {
//            std::unique_lock<std::mutex> lock(queue_mutex);
//            mutex_condition.wait(lock, [this] {
//                return !jobs.empty() || should_terminate;
//                });
//            if (should_terminate) {
//                return;
//            }
//            job = jobs.front();
//            jobs.pop();
//        }
//
//        job(); //TODO: call a function to do job here
//
//    }
//}
//
//
//void ThreadPool::QueueJob(const ivec3& job) {
//    {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        jobs.push(job);
//    }
//    mutex_condition.notify_one();
//}
//
//void ThreadPool::Start() {
//    const uint32_t num_threads = std::thread::hardware_concurrency(); // Max # of threads the system supports
//    threads.resize(num_threads);
//    for (uint32_t i = 0; i < num_threads; i++) {
//        threads.at(i) = std::thread(&ThreadPool::ThreadLoop);
//    }
//}
//
//bool ThreadPool::busy() {
//    bool poolbusy;
//    {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        poolbusy = jobs.empty();
//    }
//    return poolbusy;
//}
//
//void ThreadPool::Stop() {
//    {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        should_terminate = true;
//    }
//    mutex_condition.notify_all();
//    for (std::thread& active_thread : threads) {
//        active_thread.join();
//    }
//    threads.clear();
//}