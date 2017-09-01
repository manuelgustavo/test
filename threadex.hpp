#ifndef THREAD_EX_HPP
#define THREAD_EX_HPP


#include <thread>
#include <memory>
#include <future>

// CRTP for performance in inheritance
namespace thread_tools { 
	
	// Encapsulates thread usage.
	// It guarantees the thread is joined on destruction.
	template<typename T>
	class ThreadEx {
        std::unique_ptr<std::thread> thread_;
        std::promise<void> started_;

	public:
		ThreadEx() {}

		virtual ~ThreadEx() {
			WaitForFinish();
		}

		void WaitForFinish() {
			if (thread_ && thread_->joinable()) {
				thread_->join();
				thread_.reset();
			}
		}
		
		// starts the thread and waits for it to be running.
		void Run() {
			if (!thread_) {
				try {
					thread_ = std::make_unique<std::thread>(std::ref(*static_cast<T*>(this)));  // use static polimorphism
				} catch (...) {
					throw std::runtime_error("thread_ex::run(): Not able to create thread!");
				}
			}
			else
				throw std::runtime_error("thread_ex::run(): Thread already running!");
		}

		void NotifyStarted() {
			started_.set_value();
		}

		void WaitStarted() {
			started_.get_future().wait();
		}

	};
}

#endif // THREAD_EX_HPP