#include "pch.h"
#include <cstdio>
#include <malloc.h>
#include <vector>
#include <string>
#include "mymod.h"


std::string ServiceStateToString(DWORD dwCurrentState)
{
	switch (dwCurrentState)
	{
	case SERVICE_STOPPED:
	{
		return std::string("SERVICE_STOPPED");
	}
	case SERVICE_START_PENDING:
	{
		return std::string("SERVICE_START_PENDING");
	}
	case SERVICE_STOP_PENDING:
	{
		return std::string("SERVICE_STOP_PENDING");
	}
	case SERVICE_RUNNING:
	{
		return std::string("SERVICE_RUNNING");
	}
	case SERVICE_CONTINUE_PENDING:
	{
		return std::string("SERVICE_CONTINUE_PENDING");
	}
	case SERVICE_PAUSE_PENDING:
	{
		return std::string("SERVICE_PAUSE_PENDING");
	}
	case SERVICE_PAUSED:
	{
		return std::string("SERVICE_PAUSED");
	}
	default:
		return std::string("Unknown service state");
	}
}

emacs_value get_services(emacs_env* env, ptrdiff_t nargs, emacs_value* args, void* data)
{
	SC_HANDLE scHandle = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

	if (nullptr == scHandle)
	{
		LPSTR pszErrorMessage = nullptr;

		DWORD cchErrorMessage = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
			, nullptr, GetLastError(), NULL, (LPSTR)&pszErrorMessage, NULL, nullptr);

		emacs_value res = env->make_string(env, pszErrorMessage, cchErrorMessage);
		LocalFree(pszErrorMessage);
		return res;
	}

	DWORD cbBytesNeeded = 0;
	DWORD dwServicesReturned;

	DWORD dwError = EnumServicesStatusExA(scHandle, SC_ENUM_TYPE::SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
		nullptr, 0, &cbBytesNeeded, &dwServicesReturned, nullptr, nullptr);

	std::vector<BYTE> arrInfoBytes;

	if (ERROR_MORE_DATA == GetLastError())
	{
		arrInfoBytes.resize(cbBytesNeeded);
	}
	
	dwError = EnumServicesStatusExA(scHandle, SC_ENUM_TYPE::SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
		arrInfoBytes.data(), cbBytesNeeded, &cbBytesNeeded, &dwServicesReturned, nullptr, nullptr);
	
	ENUM_SERVICE_STATUS_PROCESS* pArrServices = (ENUM_SERVICE_STATUS_PROCESS*)arrInfoBytes.data();
	std::vector<ENUM_SERVICE_STATUS_PROCESS> arrServices(pArrServices, pArrServices + dwServicesReturned);

	std::vector<emacs_value> arrResults;
	for (const auto& service : arrServices)
	{
		std::string state = ServiceStateToString(service.ServiceStatusProcess.dwCurrentState);

		emacs_value svcInfo[2] =
		{
			env->make_string(env, service.lpDisplayName, strnlen_s(service.lpDisplayName, 1024)),
			env->make_string(env, state.c_str(), state.size())
		};

		arrResults.push_back(env->funcall(env, env->intern(env, "cons"), 2, svcInfo));
	}

	return env->funcall(env, env->intern(env, "list"), arrResults.size(), arrResults.data());
}

int emacs_module_init(struct emacs_runtime* runtime)
{
	emacs_env* env = runtime->get_environment(runtime);

	emacs_value args[][2] =
	{
		{
			env->intern(env, "get-services"),
			env->make_function(env, 1, 1,get_services, NULL, nullptr)
		},
	};

	constexpr size_t cArgs = (sizeof(args) / sizeof(emacs_value)) / 2;
	for (size_t i = 0; i < cArgs; ++i)
	{
		env->funcall(env, env->intern(env, "defalias"), 2, args[i]);
	}

	return 0;
}
