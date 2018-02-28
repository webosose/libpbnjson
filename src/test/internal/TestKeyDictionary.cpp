// Copyright (c) 2016-2018 LG Electronics, Inc.
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

#include "pbnjson.hpp"
extern "C" {
#include "src/pbnjson_c/key_dictionary.h"
}

#include <gtest/gtest.h>

#include <memory>
#include <array>
#include <thread>
#include <cstdlib>

#include <pthread.h>

using namespace pbnjson;

namespace {
	JValue keyDictionaryLookup(const std::string &key)
	{ return JValue::adopt(::keyDictionaryLookup(key.data(), key.size())); }

	std::string randomString()
	{
		size_t len = rand() % 40;
		char buf[len];
		for (size_t i = 0; i < len; ++i)
		{
			constexpr size_t chars = ('9'-'0'+1) + 2*('Z'-'A'+1) + 6;
			char c = '0' + rand() % chars;
			if (c > '9') c = c - '9' + 'A';
			if (c > 'Z') c = c - 'Z' + 'z';
			if (c > 'z')
			{
				switch (c - 'z')
				{
				case 0: c = '_'; break;
				case 1: c = '.'; break;
				case 2: c = '-'; break;
				case 3: c = '+'; break;
				case 4: c = '/'; break;
				case 5: c = '*'; break;
				}
			}
			buf[i] = c;
		}
		return {buf, len};
	}
} // anonymous namespace

TEST(TestKeyDictionary, temporary_buf)
{
	JValue jval;
	{
		char buf[] = {'a','b','c','d'};
		static_assert(sizeof(buf) == 4, "No trailing zero included in buf");
		jval = JValue::adopt(keyDictionaryLookup(buf, sizeof(buf)));
		EXPECT_EQ(std::string("abcd"), jval.asString());
	}
	{
		char buf1[] = {'d','e','f','g'};
		char buf2[] = {'a','b','c','d'};
		static_assert(sizeof(buf1) == 4, "No trailing zero included in buf2");
		static_assert(sizeof(buf2) == 4, "No trailing zero included in buf2");
		auto jval2 = JValue::adopt(keyDictionaryLookup(buf1, sizeof(buf1)));
		auto jval3 = JValue::adopt(keyDictionaryLookup(buf2, sizeof(buf2)));
		EXPECT_NE(jval.peekRaw(), jval2.peekRaw());
		EXPECT_EQ(jval.peekRaw(), jval3.peekRaw());
	}
}

TEST(TestKeyDictionary, simple)
{
	static const std::string key1 = "abcdefg";
	static const std::string key2 = "bcdefgh";
	auto jval1a = keyDictionaryLookup(key1);
	auto jval1b = keyDictionaryLookup(key1);
	auto jval2 = keyDictionaryLookup(key2);
	EXPECT_EQ(jval1a.peekRaw(), jval1b.peekRaw());
	EXPECT_NE(jval2.peekRaw(), jval1a.peekRaw());
	EXPECT_EQ(key1, jval1a.asString());
	EXPECT_EQ(key2, jval2.asString());
	auto *ptr1 = jval1a.peekRaw();
	jval1a = {};
	jval1a = keyDictionaryLookup(key1);
	EXPECT_EQ(ptr1, jval1a.peekRaw())
		<< "jval1b still keeps refs on key1";
	jval1a = {};
	jval1b = {};
	auto jval3 = keyDictionaryLookup("cdefghi"); // just to ensure allocation will take place of key1
	jval1a = keyDictionaryLookup(key1);
	EXPECT_NE(ptr1, jval1a.peekRaw())
		<< "key1 had to be re-allocated from scratch";
}

TEST(TestKeyDictionary, threaded_simple)
{
	constexpr size_t nthreads = 16, nsteps = 10000;
	static const std::string key1 = "abcdefg";
	static const std::string key2 = "bcdefgh";
	const auto f = []() {
		for (size_t step = 0; step < nsteps; ++step)
		{
			auto jval1a = keyDictionaryLookup(key1);
			auto jval1b = keyDictionaryLookup(key1);
			auto jval2 = keyDictionaryLookup(key2);
			EXPECT_EQ(jval1a.peekRaw(), jval1b.peekRaw());
			EXPECT_NE(jval2.peekRaw(), jval1a.peekRaw());
			EXPECT_EQ(key1, jval1a.asString());
			EXPECT_EQ(key2, jval2.asString());
			auto *ptr1 = jval1a.peekRaw();
			jval1a = {};
			jval1a = keyDictionaryLookup(key1);
			EXPECT_EQ(ptr1, jval1a.peekRaw())
				<< "jval1b still keeps refs on key1";
			auto jval3 = keyDictionaryLookup(randomString()); // just yet another allocation
		}
	};

	// just check that it works without theads
	f();

	// now lets check that rule in threads
	std::array<std::thread, nthreads> threads;
	for (auto &thread : threads) thread = std::thread(f);
	for (auto &thread : threads) thread.join();
}

TEST(TestKeyDictionary, release_vs_lookup)
{
	constexpr size_t nthreads = 16, nsteps = 10000;
	static const std::string key = "abcdefg";
	pthread_barrier_t ready, steady;
	const auto f = [&ready, &steady](bool lookup) {
		for (size_t step = 0; step < nsteps; ++step)
		{
			JValue jval;
			pthread_barrier_wait(&ready); // sync all on start
			if (!lookup) // lock key for release
			{
				jval = keyDictionaryLookup(key);
				EXPECT_EQ(key, jval.asString());
			}

			pthread_barrier_wait(&steady); // all is set
			if (lookup)
			{
				jval = keyDictionaryLookup(key); // do lookup
				EXPECT_EQ(key, jval.asString());
			}
			else
			{
				jval = {}; // do release
			}
		}
	};

	pthread_barrier_init(&ready, nullptr, nthreads);
	pthread_barrier_init(&steady, nullptr, nthreads);

	// now lets check that rule in threads
	std::array<std::thread, nthreads> threads;
	bool lookup = false;
	for (auto &thread : threads) thread = std::thread(f, (lookup = !lookup));
	for (auto &thread : threads) thread.join();
}
