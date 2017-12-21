/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Intel Corporation nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Liam Girdwood <liam.r.girdwood@linux.intel.com>
 *         Keyon Jie <yang.jie@linux.intel.com>
 */

#ifndef __INCLUDE_DEBUG__
#define __INCLUDE_DEBUG__

#include <reef/mailbox.h>
#include <platform/platform.h>
#include <stdint.h>
#include <stdlib.h>

/* panic reasons */
#define PANIC_MEM	0
#define PANIC_WORK	1
#define PANIC_IPC	2
#define PANIC_ARCH	3
#define PANIC_PLATFORM	4
#define PANIC_TASK	5
#define PANIC_EXCEPTION	6
#define PANIC_DEADLOCK	7
#define PANIC_STACK	8

#define DEBUG

#ifdef DEBUG

/* dump file and line to start of mailbox or shared memory */
#define dbg() \
	do { \
		volatile uint32_t *__m = (uint32_t*)mailbox_get_debug_base(); \
		*(__m++) = (__FILE__[0] << 24) + (__FILE__[1] << 16) +\
			 (__FILE__[2] << 8) + (__FILE__[3]); \
		*(__m++) = (__func__[0] << 24) + (__func__[1] << 16) + \
			(__func__[2] << 8) + (__func__[3]); \
		*__m = __LINE__; \
	} while (0);

/* dump file and line to offset in mailbox or shared memory */
#define dbg_at(__x) \
	do { \
		volatile uint32_t *__m = (uint32_t*)mailbox_get_debug_base() + __x; \
		*(__m++) = (__FILE__[0] << 24) + (__FILE__[1] << 16) +\
			 (__FILE__[2] << 8) + (__FILE__[3]); \
		*(__m++) = (__func__[0] << 24) + (__func__[1] << 16) + \
			(__func__[2] << 8) + (__func__[3]); \
		*__m = __LINE__; \
	} while (0);

/* dump value to start of mailbox or shared memory */
#define dbg_val(__v) \
	do { \
		volatile uint32_t *__m = \
			(volatile uint32_t*)mailbox_get_debug_base(); \
		*__m = __v; \
	} while (0);

/* dump value to offset in mailbox or shared memory */
#define dbg_val_at(__v, __x) \
	do { \
		volatile uint32_t *__m = \
			(volatile uint32_t*)mailbox_get_debug_base() + __x; \
		*__m = __v; \
	} while (0);

/* dump data area at addr and size count to start of mailbox or shared memory */
#define dump(addr, count) \
	do { \
		volatile uint32_t *__m = (uint32_t*)mailbox_get_debug_base(); \
		volatile uint32_t *__a = (uint32_t*)addr; \
		volatile int __c = count; \
		while (__c--) \
			*(__m++) = *(__a++); \
	} while (0);

/* dump data area at addr and size count at mailbox offset or shared memory */
#define dump_at(addr, count, offset) \
	do { \
		volatile uint32_t *__m = (uint32_t*)mailbox_get_debug_base() + offset; \
		volatile uint32_t *__a = (uint32_t*)addr; \
		volatile int __c = count; \
		while (__c--) \
			*(__m++) = *(__a++); \
	} while (0);

/* dump object to start of mailbox */
#define dump_object(__o) \
	dbg(); \
	dump(&__o, sizeof(__o) >> 2);

/* dump object from pointer at start of mailbox */
#define dump_object_ptr(__o) \
	dbg(); \
	dump(__o, sizeof(*(__o)) >> 2);

#define dump_object_ptr_at(__o, __at) \
	dbg(); \
	dump_at(__o, sizeof(*(__o)) >> 2, __at);

#else

#define dbg()
#define dbg_at(__x)
#define dbg_val(__v)
#define dbg_val_at(__v, __x)
#define dump(addr, count)
#define dump_object(__o)
#define dump_object_ptr(__o)
#endif

/* panic and stop executing any more code */
#define panic(_p) \
	do { \
		interrupt_global_disable(); \
		dbg_val(0xdead0000 | _p) \
		platform_panic(_p); \
		while(1) {}; \
	} while (0);

/* dump stack as part of panic */
#define panic_dump_stack(_p) \
	do { \
		extern uint32_t __stack; \
		extern uint32_t _stack_sentry; \
		uint32_t _stack_bottom = (uint32_t)&__stack; \
		uint32_t _stack_limit = (uint32_t)&_stack_sentry; \
		uint32_t _stack_top = arch_get_stack_ptr(); \
		uint32_t _size = _stack_bottom - _stack_top; \
		uint32_t _panic = _p; \
		dbg_val(0xdead0000 | _p) \
		dbg_val_at(_stack_top, 1) \
		dbg_val_at(_stack_bottom, 2) \
		/* is stack smashed ? */\
		if (_stack_bottom <= _stack_limit) { \
			dbg_val_at(0x51ac0000 | _p, 3); \
			_stack_bottom = _stack_limit; \
			_panic = PANIC_STACK; \
		} \
		platform_panic(_panic); \
		dump_at(_stack_top, (_size - sizeof(uint32_t)) >> 2, 4) \
		\
		while(1) {}; \
	} while (0);

#endif
