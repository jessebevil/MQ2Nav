
#include "pch.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "Recast.h"
#include "RecastDebugDraw.h"
#include "InputGeom.h"
#include "Sample_TileMesh.h"
#include "Sample_Debug.h"

#include "Interface.h"

#include "zone-utilities/log/log_macros.h"
#include "zone-utilities/log/log_stdout.h"
#include "zone-utilities/log/log_file.h"

#include <time.h>
#include <memory>


// logger to log to the build context
class LogContext : public EQEmu::Log::LogBase
{
public:
	LogContext(BuildContext& ctx) : m_ctx(ctx) {}
	virtual ~LogContext() {}

	virtual void OnRegister(int enabled_logs) {}
	virtual void OnUnregister() {}
	virtual void OnMessage(EQEmu::Log::LogType log_type, const std::string &message)
	{
		char time_buffer[512];
		time_t current_time;
		struct tm *time_info;

		time(&current_time);
		time_info = localtime(&current_time);

		strftime(time_buffer, 512, "[%m/%d/%y %H:%M:%S] ", time_info);

		switch (log_type)
		{
		case EQEmu::Log::LogTrace:
			m_ctx.log(RC_LOG_PROGRESS, "[Trace]%s%s\n", time_buffer, message.c_str());
			break;
		case EQEmu::Log::LogDebug:
			m_ctx.log(RC_LOG_PROGRESS, "[Debug]%s%s\n", time_buffer, message.c_str());
			break;
		case EQEmu::Log::LogInfo:
			m_ctx.log(RC_LOG_PROGRESS, "[Info]%s%s\n", time_buffer, message.c_str());
			break;
		case EQEmu::Log::LogWarn:
			m_ctx.log(RC_LOG_WARNING, "[Warn]%s%s\n", time_buffer, message.c_str());
			break;
		case EQEmu::Log::LogError:
			m_ctx.log(RC_LOG_ERROR, "[Error]%s%s\n", time_buffer, message.c_str());
			break;
		case EQEmu::Log::LogFatal:
			m_ctx.log(RC_LOG_ERROR, "[Fatal]%s%s\n", time_buffer, message.c_str());
			break;
		default:
			m_ctx.log(RC_LOG_PROGRESS, "[All]%s%s\n", time_buffer, message.c_str());
			break;
		}
	}

private:
	BuildContext& m_ctx;
};

int main(int argc, char* argv[])
{
	eqLogInit(-1);
	eqLogRegister(std::make_shared<EQEmu::Log::LogFile>("eqnav.log"));
	eqLogRegister(std::make_shared<EQEmu::Log::LogStdOut>());
	//eqLogRegister(std::make_shared<LogContext>(ctx));

	std::string startingZone;
	if (argc > 1)
		startingZone = argv[1];

	Interface window(startingZone);
	return window.RunMainLoop();
}
