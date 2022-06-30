#include "profiler.h"
#include <stdexcept>
#include <fstream>
namespace Piccolo
{
    void ChromeProfilingResultOutput::output(const std::list<ProfilingEvent> result) {
		std::string result_json = "[\n";
		for (auto& i : result) {
			result_json += "{\"name\": \"" + i.name + "\"," \
                + "\"ph\":\"X\",\"id\":\"0\",\"pid\":\"0\",\"tid\":\"" \
                + std::to_string(i.depth) \
                + "\",\"ts\":" + std::to_string(i.start_time) \
                + ",\"dur\":" + std::to_string(i.duration) + "},\n";

		}

		result_json[result_json.size() - 2] = ' ';
		result_json += "]\n";

		std::ofstream out(file_name);
		out << result_json;
	}

    namespace Profiler
    {
        uint32_t current_depth = 0;
		bool in_record = false;
		std::chrono::steady_clock::time_point start_time_point;
		std::chrono::steady_clock::time_point current_time_point;
		uint64_t time_duration_from_start;
		std::stack<ProfilingEvent> work_stack;
		std::list<ProfilingEvent> result_list;

		ProfilingEvent current_event;
        bool enable_profiling;
        ///////////////
        void init(bool enable) {
            enable_profiling = enable;
            start_time_point = std::chrono::high_resolution_clock::now();
        }

        void begin(const std::string& name) {
            if(!enable_profiling) return;

            tickCurrentTimePoint();
            if (in_record == false) {
                in_record = true;
                current_event.name = name;
                current_event.start_time = time_duration_from_start;
                current_event.depth = current_depth;
            }
            else {
                work_stack.push(current_event);
                current_event.name = name;
                current_event.start_time = time_duration_from_start;
                current_event.depth = current_depth;
            }
            current_depth++;
        }

        void end() {
            if(!enable_profiling) return;

            tickCurrentTimePoint();
            if (in_record == false) {
                throw std::runtime_error("Did not start any records");
            }
            current_event.duration = time_duration_from_start - current_event.start_time;
            result_list.push_back(current_event);

            if (!work_stack.empty()) {
                current_event = work_stack.top();
                work_stack.pop();
            }
            else {
                in_record = false;
            }
            current_depth--;
        }

        void tickCurrentTimePoint() {
            current_time_point = std::chrono::high_resolution_clock::now();
            time_duration_from_start = std::chrono::duration_cast<std::chrono::microseconds>(current_time_point - start_time_point).count();
        }

        void output(IProfilingResultOutput* output) {
            if(!enable_profiling) return;
            
            if (in_record) {
                throw std::runtime_error(" ");
            }
            output->output(result_list);
        }

    }


}