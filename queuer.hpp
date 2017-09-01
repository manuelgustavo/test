#ifndef QUEUER_HPP
#define QUEUER_HPP


#include <thread_tools/threadex.hpp>
#include <memory>
#include <boost/lockfree/queue.hpp>
#include <atomic>


namespace thread_tools {

    // TEMPLATE CLASS queuer SCALAR
	template<typename T, typename U>
	class Queuer : public ThreadEx<Queuer<T, U>> {
        // processes the queue
        void ProcessQueue() {
            while (!queue_.empty() && !stop_forced_) {
                T* p;
                queue_.pop(p);
                ++pop_count_;
                std::unique_ptr<T> pT(p);
                (*static_cast<U*>(this))(std::move(pT));// std::move(pT)); // call the (unique_ptr(T)) operator of the derived class
            }
        }

        // data members
        boost::lockfree::queue<T*> queue_{ 0 };
        std::atomic_ullong pop_count_{ 0 };
        std::atomic_ullong push_count_{ 0 };

        std::condition_variable the_condition_variable_;
        std::mutex the_mutex_;

        std::atomic_bool stop_now_ = false;
        std::atomic_bool stop_forced_ = false;

    protected:
        void StopQueuer() {
            Stop();
            WaitForFinish();

            // delete any remaining items (if any due to a forcible stop)
            while (!queue_.empty() && !stop_forced_) {
                T* p;
                queue_.pop(p);
                ++pop_count_;
                delete p;
            }
        }
   
	public:
        using type = T;

		Queuer() {}

		~Queuer() {
			StopQueuer();
		}

		void Reserve(size_t size) {
			queue_.reserve(size);
		}

		// thread function
		void operator()() {
			while (!stop_now_ && !stop_forced_) {
				{
					std::unique_lock<std::mutex> lock(the_mutex_);
					the_condition_variable_.wait(lock, [this] {return !queue_.empty() || stop_now_ || stop_forced_; });
				}
				ProcessQueue();
			}
			ProcessQueue(); // final processing of the queue
		}

		// it will stop the processing of the queue, but will wait until all items are consumed
		void Stop() {
			{
				std::lock_guard<std::mutex> lock(the_mutex_);
				stop_now_ = true;
			}
			the_condition_variable_.notify_all();
		}

		// it will stop the processing of the queue but not wait until all items are consumed
		void StopForced() {
			{
				std::lock_guard<std::mutex> lock(the_mutex_);
				stop_forced_ = true;
			}
			the_condition_variable_.notify_all();
		}

		void Push(std::unique_ptr<T> p) {
			auto pT = p.release();
			queue_.push(pT);
			++push_count_;
			the_condition_variable_.notify_all();
		}

		// returns true if the queue is lock free
        auto IsLockFree() {
			return queue_.is_lock_free() && push_count_.is_lock_free() && pop_count_.is_lock_free();
		}

		// returns the queue size
		unsigned long long Size() {
			return push_count_ - pop_count_;
		}

		// returns the cumulative push count
        unsigned long long PushCount() {
			return push_count_;
		}

		// returns the cumulative pop count
        unsigned long long PopCount() {
			return pop_count_;
		}

	

	};

    // TEMPLATE CLASS queuer ARRAY
    template<typename T, typename U>
    class Queuer < T[], U> : public ThreadEx<Queuer<T[], U>> {
        // data members
        boost::lockfree::queue<T*> queue_{ 0 };
        std::atomic_ullong pop_count_{ 0 };
        std::atomic_ullong push_count_{ 0 };

        std::condition_variable the_condition_variable_;
        std::mutex the_mutex_;

        std::atomic_bool stop_now_ = false;
        std::atomic_bool stop_forced_ = false;

        // processes the queue
        void ProcessQueue() {
            while (!queue_.empty() && !stop_forced_) {
                T* p;
                queue_.pop(p);
                ++pop_count_;
                std::unique_ptr<T[]> pT(p);
                (*static_cast<U*>(this))(std::move(pT));// std::move(pT)); // call the (unique_ptr(T)) operator of the derived class
            }
        }

    protected:
        void StopQueuer() {
            Stop();
            WaitForFinish();

            // delete any remaining items (if any due to a forcible stop)
            while (!queue_.empty() && !stop_forced_) {
                T* p;
                queue_.pop(p);
                ++pop_count_;
                delete[] p;
            }
        }
    public:
        using type = T[];

        Queuer() {}

        ~Queuer() {
            StopQueuer();
        }

        void Reserve(size_t size) {
            queue_.reserve(size);
        }

        // thread function
        void operator()() {
            while (!stop_now_ && !stop_forced_) {
                {
                    std::unique_lock<std::mutex> lock(the_mutex_);
                    the_condition_variable_.wait(lock, [this] {return !queue_.empty() || stop_now_ || stop_forced_; });
                }
                ProcessQueue();
            }
            ProcessQueue(); // final processing of the queue
        }

        // it will stop the processing of the queue, but will wait until all items are consumed
        void Stop() {
            {
                std::lock_guard<std::mutex> lock(the_mutex_);
                stop_now_ = true;
            }
            the_condition_variable_.notify_all();
        }

        // it will stop the processing of the queue but not wait until all items are consumed
        void StopForced() {
            {
                std::lock_guard<std::mutex> lock(the_mutex_);
                stop_forced_ = true;
            }
            the_condition_variable_.notify_all();
        }

        void Push(std::unique_ptr<T[]> p) {
            auto pT = p.release();
            queue_.push(pT);
            ++push_count_;
            the_condition_variable_.notify_all();
        }

        // returns true if the queue is lock free
        auto IsLockFree() {
            return queue_.is_lock_free() && push_count_.is_lock_free() && pop_count_.is_lock_free();
        }

        // returns the queue size
        auto Size() {
            return push_count_ - pop_count_;
        }

        // returns the cumulative push count
        unsigned long long PushCount() {
            return push_count_;
        }

        // returns the cumulative pop count
        unsigned long long PopCount() {
            return pop_count_;
        }

    };
}

#endif // QUEUER_HPP