/*
	How to profile JIT-code with CodeAnalyst or VTune
	mkprof a prof ; to get binary for CodeAnalyst
	mkprof i prof ; to get binary for VTune
	@author herumi
*/
#include <stdio.h>
#include <math.h>
#include <xbyak/xbyak.h>

#ifdef USE_CODEANALYST
#ifdef _WIN64
#define AMD64
#endif
#include "CAJITNTFLib.h"
#pragma comment(lib, "CAJitNtfyLib.lib")
#endif

#ifdef USE_VTUNE
#include "jitprofiling.h"
#pragma comment(lib, "libittnotify.lib")
#pragma comment(lib, "jitprofiling.lib")
#endif

struct Code : public Xbyak::CodeGenerator {
	Code()
	{
		mov(eax, 1000000);
	L("@@");
		for (int i = 0; i < 10; i++) {
			sub(eax, 1);
		}
		jg("@b");
		mov(eax, 1);
		ret();
	}
};

struct Code2 : public Xbyak::CodeGenerator {
	Code2()
	{
		mov(eax, 1000000);
	L("@@");
		for (int i = 0; i < 10; i++) {
			xorps(xm0, xm0);
		}
		sub(eax, 1);
		jg("@b");
		mov(eax, 1);
		ret();
	}
};

double s1(int n)
{
	double r = 0;
	for (int i = 0; i < n; i++) {
		r += 1.0 / (i + 1);
	}
	return r;
}

double s2(int n)
{
	double r = 0;
	for (int i = 0; i < n; i++) {
		r += 1.0 / (i * i + 1) + 2.0 / (i + 3);
	}
	return r;
}

#ifdef USE_VTUNE
void SetJitCode(void *ptr, size_t size, const char *name)
{
	iJIT_Method_Load jmethod = {0};
	jmethod.method_id = iJIT_GetNewMethodID();
	jmethod.class_file_name = "";
	jmethod.source_file_name = __FILE__;

	jmethod.method_load_address = ptr;
	jmethod.method_size = size;
	jmethod.line_number_size = 0;

	jmethod.method_name = const_cast<char*>(name);
	int ret = iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void*)&jmethod);
	printf("iJIT_NotifyEvent ret=%d\n", ret);
}
#endif

int main()
{
	Code c;
	Code2 c2;
	int (*f)() = (int (*)())c.getCode();
	int (*g)() = (int (*)())c2.getCode();

	printf("f:%p, %d\n", f, (int)c.getSize());
	printf("g:%p, %d\n", g, (int)c2.getSize());
#ifdef USE_CODEANALYST
	puts("use CodeAnalyst API");
	CAJIT_Initialize();
	CAJIT_LogJITCode((size_t)f, c.getSize(), L"f");
	CAJIT_LogJITCode((size_t)g, c2.getSize(), L"g");
#endif
#ifdef USE_VTUNE
	puts("use VTune API");
	iJIT_IsProfilingActiveFlags agent = iJIT_IsProfilingActive();
	fprintf(stderr, "iJIT_IsProfilingActive ret=%d\n", agent);
	SetJitCode((void*)f, c.getSize(), "f");
	SetJitCode((void*)g, c2.getSize(), "g");
#endif

	double sum = 0;
	for (int i = 0; i < 20000; i++) {
		sum += s1(i);
		sum += s2(i);
	}
	printf("sum=%f\n", sum);
	for (int i = 0; i < 2000; i++) {
		sum += f();
	}
	printf("f=%f\n", sum);
	for (int i = 0; i < 2000; i++) {
		sum += g();
	}
	printf("g=%f\n", sum);
#ifdef USE_CODEANALYST
	CAJIT_CompleteJITLog();
#endif
#ifdef USE_VTUNE
	iJIT_NotifyEvent(iJVM_EVENT_TYPE_SHUTDOWN, NULL);
#endif
	puts("end");
}
