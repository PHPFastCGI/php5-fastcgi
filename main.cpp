#include <phpcpp.h>
#include <fcgiapp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class FastCGIApplication: public Php::Base
{

private:
	FCGX_Request request;
	int          hasRequest;

	void ready(void)
	{
		if (!hasRequest) {
			throw Php::Exception("There is no request ready to operate on");
		}
	}

public:
	         FastCGIApplication() {}
	virtual ~FastCGIApplication() {}

	void __construct(Php::Parameters &params)
	{
		hasRequest = 0;

		Php::Value path    = nullptr;
		Php::Value backlog = 5;

		switch (params.size()) {
			case 2:  backlog = params[1]; // fall though
			case 1:  path    = params[0]; break;
			case 0:  break;
			default: throw Php::Exception("The FastCGIApplication constructor takes a maximum of 2 arguments");
		}

		int sock = 0;

		if (path.type() != Php::Type::Null) {
			sock = FCGX_OpenSocket((const char *) path, (int) backlog);

			if (sock < 0) {
				throw Php::Exception("Could not open socket on given path");
			}
		}

		FCGX_InitRequest(&request, sock, 0);
	}

	void __destruct(void)
	{
		FCGX_Free(&request, 1);
	}

	Php::Value accept(void)
	{
		return (hasRequest = (FCGX_Accept_r(&request) >= 0));
	}

	void finish(void)
	{
		hasRequest = 0;

		FCGX_Finish_r(&request);
	}

	void setExitStatus(Php::Parameters &params)
	{
		ready();

		int exitStatus = (int) params[0];
		FCGX_SetExitStatus(exitStatus, request.out);
	}

	Php::Value getParam(Php::Parameters &params)
	{
		ready();

		const char * name = (const char *) params[0];

		return FCGX_GetParam(name, request.envp);
	}

	Php::Value getParams(void)
	{
		ready();

		char **envp = request.envp;

		Php::Array params;

		while (*envp) {
			char *name  = strdup(*envp);
			char *value = strchr(name, '=');

			if (value == NULL) {
				free(name);
				continue;
			}

			*(value++) = 0;

			params[(const char *) name] = (const char *) value;

			free(name);

			envp++;
		}

		return params;	
	}

	Php::Value stdinRead(Php::Parameters &params)
	{
		ready();

		int length = (int) params[0];

		Php::Value data;

		char *buffer = data.reserve(length);

		int read = FCGX_GetStr(buffer, length, request.in);

		data.reserve(read);

		return data;
	}

	Php::Value stdinEof(void)
	{
		ready();

		return (FCGX_HasSeenEOF(request.in) == EOF);
	}

	Php::Value stdoutWrite(Php::Parameters &params)
	{
		ready();

		Php::Value data = params[0];

		int written = FCGX_PutStr((const char *) data, data.size(), request.out);

		if (written < 0) {
			return false;
		}

		return written;
	}

	Php::Value stdoutEof(void)
	{
		ready();

		return (FCGX_HasSeenEOF(request.out) == EOF);
	}
};

extern "C" {
    PHPCPP_EXPORT void *get_module() 
    {
        static Php::Extension extension("fastcgi", "1.0");

	/* Initialize library */
	extension.onStartup([]() {
		FCGX_Init();
	});

	/* Define Interface: FastCGIApplicationInterface */
	Php::Interface fastCGIApplicationInterface("FastCGIApplicationInterface");

	fastCGIApplicationInterface.method("accept");
	fastCGIApplicationInterface.method("finish");
	fastCGIApplicationInterface.method("setExitStatus", { Php::ByVal("exitStatus", Php::Type::Numeric) });
	fastCGIApplicationInterface.method("getParam",      { Php::ByVal("name",       Php::Type::String)  });
	fastCGIApplicationInterface.method("getParams");
	fastCGIApplicationInterface.method("stdinRead",     { Php::ByVal("length",     Php::Type::Numeric) });
	fastCGIApplicationInterface.method("stdinEof");
	fastCGIApplicationInterface.method("stdoutWrite",   { Php::ByVal("data",       Php::Type::String)  });
	fastCGIApplicationInterface.method("stdoutEof");

	/* Define Class: FastCGIApplication */
	Php::Class<FastCGIApplication> fastCGIApplicationClass("FastCGIApplication");

	fastCGIApplicationClass.method("__construct", &FastCGIApplication::__construct, {
		Php::ByVal("path",    Php::Type::String, false),
		Php::ByVal("backlog", Php::Type::Numeric, false)
	});
	fastCGIApplicationClass.method("__destruct",    &FastCGIApplication::__destruct);

	fastCGIApplicationClass.method("accept",        &FastCGIApplication::accept);
	fastCGIApplicationClass.method("finish",        &FastCGIApplication::finish);
	fastCGIApplicationClass.method("setExitStatus", &FastCGIApplication::setExitStatus, { Php::ByVal("exitStatus", Php::Type::Numeric) });
	fastCGIApplicationClass.method("getParam",      &FastCGIApplication::getParam,      { Php::ByVal("name",       Php::Type::String)  });
	fastCGIApplicationClass.method("getParams",     &FastCGIApplication::getParams);
	fastCGIApplicationClass.method("stdinRead",     &FastCGIApplication::stdinRead,     { Php::ByVal("length",     Php::Type::Numeric) });
	fastCGIApplicationClass.method("stdinEof",      &FastCGIApplication::stdinEof);
	fastCGIApplicationClass.method("stdoutWrite",   &FastCGIApplication::stdoutWrite,   { Php::ByVal("data",       Php::Type::String)  });
	fastCGIApplicationClass.method("stdoutEof",     &FastCGIApplication::stdoutEof);

	fastCGIApplicationClass.implements(fastCGIApplicationInterface);

	extension.add(fastCGIApplicationInterface);
	extension.add(fastCGIApplicationClass);

        return extension;
    }
}

