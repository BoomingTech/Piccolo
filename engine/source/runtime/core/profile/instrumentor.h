#pragma once
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>

#define PICCOLO_PROFILE

namespace Piccolo {
	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult {
		std::string name;

		FloatingPointMicroseconds start;
		std::chrono::microseconds elapsed_time;
		std::thread::id thread_id;
	};

	struct InstrumentationSession {
		std::string name;
	};

	class Instrumentor {
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void BeginSession(const std::string& name, const std::string& filepath = "results.json") {
			std::lock_guard<std::mutex> lock(mutex_);
			if (current_session_) {
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				//if (Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
				//{
			    //	LOG_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", name, current_session_->name);
				//}
				InternalEndSession();
			}
			output_stream_.open(filepath);

			if (output_stream_.is_open()) {
				current_session_ = new InstrumentationSession({ name });
				WriteHeader();
			}
			else {
				//if (Log::GetCoreLogger()) // Edge case: BeginSession() might be before Log::Init()
				//{
				//	LOG_ERROR("Instrumentor could not open results file '{0}'.", filepath);
				//}
			}
		}

		void EndSession() {
			std::lock_guard<std::mutex> lock(mutex_);
			InternalEndSession();
		}

		void WriteProfile(const ProfileResult& result) {
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.elapsed_time.count()) << ',';
			json << "\"name\":\"" << result.name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":\"" << result.thread_id << "\",";
			json << "\"ts\":" << result.start.count();
			json << "}";

			std::lock_guard<std::mutex> lock(mutex_);
			if (current_session_)
			{
				output_stream_ << json.str();
				output_stream_.flush();
			}
		}

		static Instrumentor& Get() {
			static Instrumentor instance;
			return instance;
		}

	private:
		Instrumentor() : current_session_(nullptr) {}

		~Instrumentor() { EndSession(); }

		void WriteHeader() {
			output_stream_ << "{\"otherData\": {},\"traceEvents\":[{}";
			output_stream_.flush();
		}

		void WriteFooter() {
			output_stream_ << "]}";
			output_stream_.flush();
		}

		// Note: you must already own lock on mutex_ before
		// calling InternalEndSession()
		void InternalEndSession() {
			if (current_session_) {
				WriteFooter();
				output_stream_.close();
				delete current_session_;
				current_session_ = nullptr;
			}
		}

	private:
		std::mutex mutex_;
		InstrumentationSession* current_session_;
		std::ofstream output_stream_;
	};

	class InstrumentationTimer {
	public:
		InstrumentationTimer(const char* name)
			: name_(name), stopped_(false) { start_time_ = std::chrono::steady_clock::now(); }

		~InstrumentationTimer() {
			if (!stopped_)
				Stop();
		}

		void Stop() {
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = FloatingPointMicroseconds{ start_time_.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(start_time_).time_since_epoch();

			Instrumentor::Get().WriteProfile({ name_, highResStart, elapsedTime, std::this_thread::get_id() });

			stopped_ = true;
		}

	private:
		const char* name_;
		std::chrono::time_point<std::chrono::steady_clock> start_time_;
		bool stopped_;
	};

	namespace InstrumentorUtils {
		template <size_t N>
		struct ChangeResult {
			char data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K]) {
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N) {
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;
				if (matchIndex == K - 1)
					srcIndex += matchIndex;
				result.data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	} // namespace InstrumentorUtils
} // namespace Rocket

#if defined(PICCOLO_PROFILE)
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define PICCOLO_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define PICCOLO_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define PICCOLO_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define PICCOLO_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define PICCOLO_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define PICCOLO_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define PICCOLO_FUNC_SIG __func__
#else
#define PICCOLO_FUNC_SIG "PICCOLO_FUNC_SIG unknown!"
#endif

#define PICCOLO_PROFILE_BEGIN_SESSION(name, filepath) ::Piccolo::Instrumentor::Get().BeginSession(name, filepath)
#define PICCOLO_PROFILE_END_SESSION() ::Piccolo::Instrumentor::Get().EndSession()
#define PICCOLO_PROFILE_SCOPE_LINE2(name, line) \
	::Piccolo::InstrumentationTimer timer##line(name)
#define PICCOLO_PROFILE_SCOPE_LINE(name, line) PICCOLO_PROFILE_SCOPE_LINE2(name, line)
#define PICCOLO_PROFILE_SCOPE(name) PICCOLO_PROFILE_SCOPE_LINE(name, __LINE__)
#define PICCOLO_PROFILE_FUNCTION() PICCOLO_PROFILE_SCOPE(PICCOLO_FUNC_SIG)
#else
#define PICCOLO_PROFILE_BEGIN_SESSION(name, filepath)
#define PICCOLO_PROFILE_END_SESSION()
#define PICCOLO_PROFILE_SCOPE(name)
#define PICCOLO_PROFILE_FUNCTION()
#endif
