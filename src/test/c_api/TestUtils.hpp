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

/**
 *  @file TestUtils.hpp
 */

#pragma once

#include <pbnjson.h>

#include <memory>

using namespace std;

template <typename T>
unique_ptr<T, function<void(T*)>> mk_ptr(T* p)
{
	static auto deleter = [] (T* p) { delete p; };
	return { p, deleter };
}

template <>
unique_ptr<jschema, function<void(jschema*)>> mk_ptr(jschema* p)
{
	static auto deleter = [] (jschema* p) { jschema_release(&p); };
	return { p, deleter };
}

template <>
unique_ptr<jvalue, function<void(jvalue*)>> mk_ptr(jvalue* p)
{
	static auto deleter = [] (jvalue* p) { j_release(&p); };
	return { p, deleter };
}
