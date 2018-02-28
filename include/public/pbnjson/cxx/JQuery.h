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

#ifndef JQUERY_CXX_H_
#define JQUERY_CXX_H_

#include "pbnjson.h"
#include "JValue.h"

namespace pbnjson {

/**
 * The class is C++ wrapper for jsonselect query language.
 */
class PJSONCXX_API JQuery : public JResult
{
private:
	jquery_ptr m_query;
	jvalue_ref m_jval;

public:
	/**
	 * Create query object from the query string.
	 *
	 * \param query Query string
	 */
	JQuery(const std::string& query)
		: m_query(jquery_create(query.c_str(), &error))
		, m_jval(0)
	{ }

	/**
	 * Create query object from the query string.
	 *
	 * \param query Query string
	 */
	JQuery(const char *query)
		: m_query(jquery_create(query, &error))
		, m_jval(0)
	{ }

	~JQuery()
	{
		if (m_query)
			jquery_free(m_query);
	}

	/**
	 * Start query execution for the JSON.
	 *
	 * \param JSON JSON object
	 *
	 * \see JQuery::operator()
	 */
	JQuery& apply(const JValue& JSON)
	{
		m_jval = JSON.m_jval;
		return *this;
	}

	/**
	 * Start query execution for the JSON.
	 *
	 * \param JSON JSON object
	 *
	 * \see JQuery::apply
	 */
	JQuery& operator()(const JValue& JSON)
	{ return apply(JSON); }

	/**
	 * JQuery iterator
	 */
	class iterator : public std::iterator<std::forward_iterator_tag, JValue>
	{
	private:
		jvalue_ref _c;
		jquery_ptr _q;

	public:
		iterator(jquery_ptr q)
			: _q(q)
		{
			if (_q) _c = jquery_next(_q);
			else    _c = jinvalid();
		}

		iterator& operator++()
		{
			_c = jquery_next(_q);
			return *this;
		}

		value_type operator*() const
		{ return JValue(jvalue_copy(_c)); }

		bool operator!=(const iterator& o)
		{ return _c != o._c; }
	};

	/**
	 * Returns an iterator to the first query result for current JSON.
	 *
	 * \retval iterator
	 */
	iterator begin()
	{
		return m_query && jquery_init(m_query, m_jval, 0)
			? iterator(m_query)
			: iterator(0);
	}

	/**
	 * Returns an iterator to the end of the query for current JSON
	 * (JInvalid).
	 *
	 * \retval iterator
	 */
	iterator end()
	{ return iterator(0); }
};

}

#endif /* JQUERY_CXX_H_ */
