#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <variant>
#include <cstddef>
#include <iostream>

struct worker_data {
	std::mutex until_ready_mutex;
	std::condition_variable until_ready_condition;
	bool is_ready = false;
	bool is_processed = false;
	sol::state worker_lua;
	sol::bytecode payload;
	std::variant<double, std::vector<double>> return_payload;

	worker_data() {
		worker_lua.open_libraries(sol::lib::base);
	}
};

void worker_thread(worker_data& data) {
	for (std::uint64_t loops = 0; true; ++loops) {
		// Wait until main() sends data
		std::unique_lock<std::mutex> data_lock(
		     data.until_ready_mutex);
		data.until_ready_condition.wait(
		     data_lock, [&data] { return data.is_ready; });

		if (data.payload.size() == 0) {
			// signaling we are done
			return;
		}

		// just for easier typing
		sol::state& lua = data.worker_lua;

		// we own the lock now, do the work
		std::variant<double, std::vector<double>> result
		     = lua.safe_script(data.payload.as_string_view());

		// store returning payload,
		// clear current payload
		data.return_payload = std::move(result);
		data.payload.clear();

		// Send result back to main
		std::cout << "worker_thread data processing is "
		             "completed: signaling & unlocking\n";
		data.is_processed = true;
		data.is_ready = false;
		data_lock.unlock();
		data.until_ready_condition.notify_one();
	}
}

int main() {

	// main lua state
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// set up functions, etc. etc.
	lua.script("function f () return 4.5 end");
	lua.script("function g () return { 1.1, 2.2, 3.3 } end");

	// kick off worker
	worker_data data;
	std::thread worker(worker_thread, std::ref(data));

	// main Lua state
	bool done_working = false;
	for (std::uint64_t loops = 0; !done_working; ++loops) {
		// finished working? send nothing
		// even loop? use f
		// otherwise, use g
		if (loops >= 3) {
			data.payload.clear();
			done_working = true;
		}
		else if ((loops % 2) == 0) {
			sol::function target = lua["f"];
			data.payload = target.dump();
		}
		else {
			sol::function target = lua["g"];
			data.payload = target.dump();
		}

		// send data to the worker thread
		{
			std::lock_guard<std::mutex> lk(
			     data.until_ready_mutex);
			data.is_ready = true;
			std::cout
			     << "function serialized: sending to worker "
			        "thread to execute on Lua state...\n";
		}
		data.until_ready_condition.notify_one();

		if (done_working) {
			break;
		}
		// wait for the worker
		{
			std::unique_lock<std::mutex>
			     lock_waiting_for_worker(
			          data.until_ready_mutex);
			data.until_ready_condition.wait(
			     lock_waiting_for_worker,
			     [&data] { return data.is_processed; });
			data.is_processed = false;
		}
		auto data_processor = [](auto& returned_data) {
			using option_type
			     = std::remove_cv_t<std::remove_reference_t<
			          decltype(returned_data)>>;
			if constexpr (std::is_same_v<option_type,
			                   double>) {
				std::cout << "received a double: "
				          << returned_data << "\n";
			}
			else if constexpr (std::is_same_v<option_type,
			                        std::vector<double>>) {
				std::cout
				     << "received a std::vector<double>: { ";
				for (std::size_t i = 0;
				     i < returned_data.size();
				     ++i) {
					std::cout << returned_data[i];
					if (i
					     != static_cast<std::size_t>(
					          returned_data.size() - 1)) {
						std::cout << ", ";
					}
				}
				std::cout << " }\n";
			}
			else {
				std::cerr << "OH MY GOD YOU FORGOT TO "
				             "HANDLE A TYPE OF DATA FROM A "
				             "WORKER ABORT ABORT ABORT\n";
				std::abort();
			}
		};
		std::visit(data_processor, data.return_payload);
	}

	// join and wait for workers to come back
	worker.join();

	// workers are back, exit program
	return 0;
}
