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

#ifndef INCLUDE_PUBLIC_PBNJSON_CXX_JINPUT_H_
#define INCLUDE_PUBLIC_PBNJSON_CXX_JINPUT_H_

#include "../c/jtypes.h"

#include <string>

namespace pbnjson {

struct JInput : raw_buffer
{
	JInput(const raw_buffer &buf) : raw_buffer(buf)
	{}

	JInput(const std::string &s)
	{
		m_str = s.data();
		m_len = s.length();
	}

	JInput(const char *s)
	{
		m_str = s;
		m_len = strlen(s);
	}

	JInput(const char *s, size_t l)
	{
		m_str = s;
		m_len = l;
	}
};

}



#endif /* INCLUDE_PUBLIC_PBNJSON_CXX_JINPUT_H_ */
