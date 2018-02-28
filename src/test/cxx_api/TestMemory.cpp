// Copyright (c) 2015-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>
#include <pbnjson.hpp>
#include <fstream>
#include <malloc.h>
#ifdef HAVE_JSON_C
#include <json-c/json.h>
#endif

#include <malloc.h>
#include <mcheck.h>

using namespace std;

namespace {

void PrintMem(const char *msg)
{
	size_t size{0}, resident{0}, share{0};
	{
		ifstream ifs("/proc/self/statm");
		ifs >> size >> resident >> share;
	}
	size_t page_kb = sysconf(_SC_PAGE_SIZE) / 1024;
	auto rss = resident * page_kb;
	auto shared_mem = share * page_kb;

	auto info = mallinfo();

	cout << msg << ": rss=" << rss << " kB  shared=" << shared_mem << " kB  private=" << (rss - shared_mem)
		<< " kB  arena=" << info.arena << " B  mmaped=" << info.hblkhd
		<< " B  free=" << info.fordblks << " B" << endl;
	malloc_stats();
}

}

TEST(TestMemory, Pbnjson)
{
	PrintMem("Before");

	// set env var MALLOC_TRACE to some filename to dump trace
	// otherwise does nothing
	mtrace();
	{
		auto v = pbnjson::JDomParser::fromFile("samples/big.json");
		ASSERT_TRUE(v.isValid());
		PrintMem("Allocated");
	}
	muntrace();
	PrintMem("Freed");
}

#ifdef HAVE_CJSON
TEST(TestMemory, DISABLED_Cjson)
{
	ifstream ifs("samples/big.json");
	string input((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
	PrintMem("Before: ");
	auto v = json_tokener_parse(input.c_str());
	PrintMem("Allocated: ");

	json_object_put(v);
	PrintMem("Freed: ");
}
#endif
