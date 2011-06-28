#include <stdio.h>
#include <syslog.h>
#include <rtdk.h>
#include <asm-generic/bits/current.h>

int __wrap_vfprintf(FILE *stream, const char *fmt, va_list args)
{
	if (unlikely(xeno_get_current() != XN_NO_HANDLE &&
		     !(xeno_get_current_mode() & XNRELAX)))

		return rt_vfprintf(stream, fmt, args);
	else {
		rt_print_flush_buffers();
		return vfprintf(stream, fmt, args);
	}
}

int __wrap_vprintf(const char *fmt, va_list args)
{
	return __wrap_vfprintf(stdout, fmt, args);
}

int __wrap_fprintf(FILE *stream, const char *fmt, ...)
{
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = __wrap_vfprintf(stream, fmt, args);
	va_end(args);

	return rc;
}

int __wrap_printf(const char *fmt, ...)
{
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = __wrap_vfprintf(stdout, fmt, args);
	va_end(args);

	return rc;
}

void __wrap_vsyslog(int priority, const char *fmt, va_list ap)
{
	if (unlikely(xeno_get_current() != XN_NO_HANDLE &&
		     !(xeno_get_current_mode() & XNRELAX)))
		return rt_vsyslog(priority, fmt, ap);
	else {
		rt_print_flush_buffers();
		vsyslog(priority, fmt, ap);
	}
}

void __wrap_syslog(int priority, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	__wrap_vsyslog(priority, fmt, args);
	va_end(args);
}
