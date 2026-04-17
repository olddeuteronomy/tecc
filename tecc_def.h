/*----------------------------------------------------------------------
------------------------------------------------------------------------
Copyright (c) 2020-2026 The Emacs Cat (https://github.com/olddeuteronomy/tecc).

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
------------------------------------------------------------------------
----------------------------------------------------------------------*/

#ifndef TECC_DEF_H
#define TECC_DEF_H

#define TECC_ 1

// Check for Linux
#if defined(linux) || defined(__linux) || defined(__linux__)
#  define TECC_OS_LINUX_ 1
#endif

// Check for Unix
#if defined(unix) || defined(__unix) || defined(__unix__)
#  define TEC_OS_UNIX_ 1
#endif

// Check for MS Windows
#if defined(_MSC_VER) && (defined(_WIN32) || defined(_WIN64))
#  define TECC_OS_WINDOWS_ _MSC_VER
#endif

// Check for Apple
#if defined(__APPLE__)
#  define TECC_OS_MACOS_ __APPLE__
#endif

// New line.
#if defined (TECC_OS_WINDOWS_)
#  define TECC_NL "\r\n"
#else
#  define TECC_NL "\n"
#endif

// Enquoting symbol.
#define TECC_ENQUOTE(s) "\"" #s "\""
#define TECC_STR(x)     #x
#define TECC_XSTR(x)    STR(x)

// To disable the 'unused variable' compiler warning. Ignored on MS Windows.
#define TECC_unused __attribute__((unused))

// Type‑safe variadic API using macro overloading.
#define TECC_GET_MACRO_0(NAME,...) NAME
#define TECC_GET_MACRO_1(_1,NAME,...) NAME
#define TECC_GET_MACRO_2(_1,_2,NAME,...) NAME
#define TECC_GET_MACRO_3(_1,_2,_3,NAME,...) NAME
#define TECC_GET_MACRO_4(_1,_2,_3,_4,NAME,...) NAME

// Get a pointer to a `member` inside a struct `type`.
#define TECC_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))


#define TECC_API
#define TECC_IMPL


#ifdef __cplusplus
extern "C" {
#endif

// ...

#ifdef __cplusplus
}
#endif

#endif // TECC_DEF_H
