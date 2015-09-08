#include <phpcpp.h>
#include <fcgiapp.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#define BUFFER_SIZE 1024 * 20 // 20 KB buffer

Php::Value fastcgi_accept(Php::Parameters &params)
{
	Php::Value callback = params[0];

	FCGX_Init();

	FCGX_Request request;

	FCGX_InitRequest(&request, 0, 0);

	char *buffer = new char[BUFFER_SIZE];
	int bytes_read;

	while (FCGX_Accept_r(&request) == 0) {
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

			params[(const char *)name] = (const char *)value;

			free(name);

			envp++;
		}

		Php::Value stream = Php::call("fopen", "php://temp", "r+");

		do {
			bytes_read = FCGX_GetStr(buffer, BUFFER_SIZE, request.in);

			if (bytes_read > 0) {
				Php::call("fwrite", stream, bytes_read);
			}
		} while (bytes_read == BUFFER_SIZE);

		Php::call("rewind", stream);

		Php::Value result = callback(params, stream);

		Php::call("fclose", stream);

		if (Php::call("is_resource", result)) {
			Php::Value block;

			while ((block = Php::call("fread", result, BUFFER_SIZE)) != false) {
				FCGX_PutStr(block, block.size(), request.out);
			}
		} else {
			FCGX_PutStr(result, result.size(), request.out);
		}
	}

	delete buffer;

	return false;
}

extern "C" {
    PHPCPP_EXPORT void *get_module() 
    {
        static Php::Extension extension("fastcgi", "1.0");
        
       	extension.add("fastcgi_accept", fastcgi_accept, {
		Php::ByVal("callback", Php::Type::Callable)
	});

        return extension;
    }
}

