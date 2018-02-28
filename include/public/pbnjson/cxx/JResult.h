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

#ifndef INCLUDE_PUBLIC_PBNJSON_CXX_JRESULT_H_
#define INCLUDE_PUBLIC_PBNJSON_CXX_JRESULT_H_

#include <string>

#include "../c/jerror.h"
#include "../c/compiler/cpp11.h"

namespace pbnjson {

/**
 * JResult is a class, which can contain an error as a result of member
 * function execution. Later using class functions, the error may be detected
 * and logged.
 *
 * @see JParser
 * @see JQuery
 * @see JSchema
 */
class JResult
{
public:
	JResult() : error(NULL){}

	JResult(const JResult& other)
		: error(jerror_duplicate(other.error))
	{
	}

#ifdef CPP11
	JResult(JResult&& other)
	{
		error = other.error;
		other.error = nullptr;
	}
#endif

	JResult& operator=(JResult other)
	{
		swap(other);
		return *this;
	}

	/**
	* @brief Swap with passed JResult
	*
	* @param other JResult to swap
	*/
	void swap(JResult& other)
	{
		std::swap(error, other.error);
	}

	virtual ~JResult()
	{
		jerror_free(error);
	}

	/**
	 * Return textual representation of the error
	 *
	 * @return String
	*/
	std::string errorString() const
	{
		char msg[512];
		jerror_to_string(error, msg, sizeof(msg));
		return msg;
	}

	/**
	 * Return true if object contains an error.
	 *
	 * @return true if the object has error
	*/
	bool isError() const
	{
		return error != NULL;
	}

	/**
	 * Return if object contains an error
	 *
	 * @return true if has error
	*/
#ifdef CPP11
	explicit
#endif
	operator bool() const
	{
		return error == NULL;
	}

protected:
	/**
	 * @brief error struct from C code
	 */
	jerror *error;

	friend class JSchema;
};

}

#endif /* INCLUDE_PUBLIC_PBNJSON_CXX_JRESULT_H_ */
