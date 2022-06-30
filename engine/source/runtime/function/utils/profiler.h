#include <list>
#include <stack>
#include <string>
#include <chrono>

namespace Piccolo
{
    struct ProfilingEvent 
    {
		std::string name;
		uint64_t start_time;
		uint64_t duration;
		uint32_t depth;
	};

	class IProfilingResultOutput 
    {
	public:
		virtual ~IProfilingResultOutput() {}
		virtual void output(const std::list<ProfilingEvent> result) = 0;
	};

	class ChromeProfilingResultOutput : public IProfilingResultOutput 
    {
	public:
		ChromeProfilingResultOutput(std::string file) : file_name(file) {}
		virtual ~ChromeProfilingResultOutput() {}
		virtual void output(const std::list<ProfilingEvent> result) override;
	private:
		std::string file_name;
	};

	namespace Profiler 
    {
	
		
		void init(bool enable = true);

		void begin(const std::string& name);
		void end();
		void output(IProfilingResultOutput* output);
	

    /////////
		void tickCurrentTimePoint();
	}
}