// Copyright (c) 2009-2024 LG Electronics, Inc.
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

#include <JValue.h>

#include <pbnjson.h>
#include <JSchema.h>
#include <JGenerator.h>
#include <cassert>
#include <limits>
#include <cmath>
#include <type_traits>
#include "liblog.h"

#ifdef DBG_CXX_MEM_STR
#define PJ_DBG_CXX_STR(expr) expr
#else
#define PJ_DBG_CXX_STR(expr) do { } while(0)
#endif

namespace pbnjson {

JValue JValue::JNULL(jnull());
JValue JValue::JINVALID(jinvalid());

static inline raw_buffer strToRawBuffer(const std::string& str)
{
	return (raw_buffer){str.c_str(), str.length()};
}

static inline void arrayit_inc_index(size_t& cur, size_t step, size_t max)
{
    // Used a signed type to handle potential overflow
    int temp = static_cast<int>(cur) + step;
    cur = ( ( static_cast<size_t>(temp) ) < max ) ? ( static_cast<size_t>(temp) ) : -1 ;
}

static inline void arrayit_dec_index(size_t& cur, size_t step, size_t min)
{
    // Use a signed type to handle potential underflow
    int temp = static_cast<int>(cur) - step;
    cur = ( temp >= min ) ? static_cast<size_t>(temp) : -1;
}

JValue::ArrayIterator::ArrayIterator()
	: _parent(NULL)
	, _index(-1)
{
}

JValue::ArrayIterator::ArrayIterator(jvalue_ref parent)
	: _parent(NULL)
	, _index(-1)
{
	if (LIKELY(jis_valid(parent) && jis_array(parent)))
	{
		if (jarray_size(parent) != 0)
		{
			_index  = 0;
			_parent = jvalue_copy(parent);
		}
	}
	else
	{
		assert("Can't iterate over non-array");
	}
}

JValue::ArrayIterator::ArrayIterator(const ArrayIterator& other)
	: _parent(jvalue_copy(other._parent))
	, _index(other._index)
{
}

JValue::ArrayIterator::~ArrayIterator()
{
	j_release(&_parent);
}

JValue::ArrayIterator& JValue::ArrayIterator::operator=(const ArrayIterator &other)
{
	if (this != &other)
	{
		j_release(&_parent);
		_parent = jvalue_copy(other._parent);
		_index  = other._index;
	}

	return *this;
}

JValue::ArrayIterator& JValue::ArrayIterator::operator++()
{
	ssize_t parent = jarray_size(_parent);
	if(parent < 0)
		parent = 0;

	arrayit_inc_index(_index, 1, parent);
	return *this;
}

JValue::ArrayIterator JValue::ArrayIterator::operator++(int)
{
	ArrayIterator it(*this);
	++(*this);
	return it;
}

JValue::ArrayIterator& JValue::ArrayIterator::operator--()
{
	arrayit_dec_index(_index, 1, 0);
	return *this;
}

JValue::ArrayIterator JValue::ArrayIterator::operator--(int)
{
	ArrayIterator it(*this);
	--(*this);
	return it;
}

JValue::ArrayIterator JValue::ArrayIterator::operator+(size_type n) const
{
	ArrayIterator it(*this);
	arrayit_inc_index(it._index, n, jarray_size(it._parent));
	return it;
}

JValue::ArrayIterator JValue::ArrayIterator::operator-(size_type n) const
{
	ArrayIterator it(*this);
	arrayit_dec_index(it._index, n, 0);
	return it;
}

bool JValue::ArrayIterator::operator==(const ArrayIterator& other) const
{
	if (this == &other)
		return true;
	return _index == other._index;
}

const JValue::ArrayIterator::value_type JValue::ArrayIterator::operator*() const
{
	return JValue(jvalue_copy(jarray_get(_parent, _index)));
}

JValue::ObjectIterator::ObjectIterator()
	: _parent(0)
	, _at_end(true)
	, _it()
{
	_key_value.key = 0;
	_key_value.value = 0;
}

JValue::ObjectIterator::ObjectIterator(jvalue_ref parent)
	: _parent(0)
	, _at_end(false)
{
	_key_value.key = 0;
	_key_value.value = 0;

	if (LIKELY(jobject_iter_init(&_it, parent)))
	{
		_parent = jvalue_copy(parent);
		_at_end = !jobject_iter_next(&_it, &_key_value);
	}
	else
	{
		assert("Can't iterate over non-object");
	}
}

JValue::ObjectIterator::ObjectIterator(const ObjectIterator& other)
	: _it(other._it)
	, _parent(jvalue_copy(other._parent))
	, _key_value(other._key_value)
	, _at_end(other._at_end)
{
}

JValue::ObjectIterator::~ObjectIterator()
{
	j_release(&_parent);
}

JValue::ObjectIterator& JValue::ObjectIterator::operator=(const ObjectIterator &other)
{
	if (this != &other)
	{
		_it = other._it;
		j_release(&_parent);
		_parent = jvalue_copy(other._parent);
		_key_value = other._key_value;
		_at_end = other._at_end;
	}
	return *this;
}

/**
 * specification says it's undefined, but implementation-wise,
 * the C api will return the current iterator if you try to go past the end.
 *
 */
JValue::ObjectIterator& JValue::ObjectIterator::operator++()
{
	_at_end = !jobject_iter_next(&_it, &_key_value);
	return *this;
}

JValue::ObjectIterator JValue::ObjectIterator::operator++(int)
{
	ObjectIterator result(*this);
	++(*this);
	return result;
}

JValue::ObjectIterator JValue::ObjectIterator::operator+(size_t n) const
{
	ObjectIterator next(*this);
	for (; n > 0; --n)
		++next;
	return next;
}

bool JValue::ObjectIterator::operator==(const ObjectIterator& other) const
{
	if (this == &other)
		return true;
	if (_at_end && other._at_end)
		return true;
	if (_at_end || other._at_end)
		return false;
	return jstring_equal(_key_value.key, other._key_value.key);
}

const JValue::KeyValue JValue::ObjectIterator::operator*() const
{
	return KeyValue(jvalue_copy(_key_value.key), jvalue_copy(_key_value.value));
}


/**
 * specification says it's undefined. in the current implementation
 * though, jobj_iter_init should return end() when this isn't an object
 * (it also takes care of printing errors to the log)
 */
JValue::ObjectIterator JValue::begin()
{
	return ObjectIterator(m_jval);
}

/**
 * Specification says it's undefined.  In the current implementation
 * though, jobj_iter_init_last will return a NULL pointer when this isn't
 * an object (it also takes care of printing errors to the log)
 *
 * Specification says undefined if we try to iterate - current implementation
 * won't let you iterate once you hit end.
 */
JValue::ObjectIterator JValue::end()
{
	return ObjectIterator();
}

/**
 * specification says it's undefined. in the current implementation
 * though, jobj_iter_init should return end() when this isn't an object
 * (it also takes care of printing errors to the log)
 */
JValue::ObjectConstIterator JValue::begin() const
{
	return ObjectConstIterator(m_jval);
}

/**
 * Specification says it's undefined.  In the current implementation
 * though, jobj_iter_init_last will return a NULL pointer when this isn't
 * an object (it also takes care of printing errors to the log)
 *
 * Specification says undefined if we try to iterate - current implementation
 * won't let you iterate once you hit end.
 */
JValue::ObjectConstIterator JValue::end() const
{
	return ObjectConstIterator();
}

//!@cond Doxygen_Suppress
JValue::JValue()
	: m_jval(JNULL.m_jval)
{
}

JValue::JValue(const int32_t value)
	: m_jval(jnumber_create_i64(value))
{
}

JValue::JValue(const int64_t value)
	: m_jval(jnumber_create_i64(value))
{
}

JValue::JValue(const double value)
	: m_jval(jnumber_create_f64(value))
{
}

JValue::JValue(const std::string &value)
	: m_jval(jstring_create_utf8(
				value.c_str(),
				value.size() < (static_cast<unsigned long>(std::numeric_limits<long>::max())) ? value.size() : 0 ) )
{
}

JValue::JValue(const char *str)
	: m_jval(jstring_create_utf8(
				str,
				strlen(str) < (static_cast<unsigned long>(std::numeric_limits<long>::max())) ? strlen(str) : 0 ) )
{
}

JValue::JValue(const bool value)
	: m_jval(jboolean_create(value))
{
}

JValue::JValue(const NumericString& value)
	: m_jval(jnumber_create((raw_buffer){value.c_str(), value.size()}))
{
}

JValue::JValue(const JValueArrayElement& other)
	: JResult(other)
	, m_jval(jvalue_copy(other.m_jval))
{
}

JValue::JValue(const JValue& other)
	: JResult(other)
	, m_jval(jvalue_copy(other.m_jval))
{
}

JValue::~JValue()
{
	j_release(&m_jval);
}

JValue& JValue::operator=(const JValue& other)
{
	if (this != &other)
	{
		JValue(other).swap(*this);
	}
	return *this;
}

JValue JValue::duplicate() const
{
	return jvalue_duplicate(this->peekRaw());
}

JValue Object()
{
	return jobject_create();
}

JValue Array()
{
	return jarray_create(NULL);
}

bool JValue::operator==(const JValue& other) const
{
	return jvalue_equal(m_jval, other.m_jval);
}

template <class T>
static bool numEqual(const JValue& jnum, const T& nativeNum)
{
	T num{};
	if (jnum.asNumber(num) == CONV_OK)
	{
		if (std::is_integral<T>::value)
		{
			return num == nativeNum;
		}
		else
		{
			return std::fabs(num - nativeNum) <= std::numeric_limits<T>::epsilon();
		}
	}
	return false;
}

bool JValue::operator==(const char * other) const
{
	const char * buffer = asCString();
	if (buffer == NULL)
		return false;

	return strcmp(buffer, other) == 0;
}

bool JValue::operator==(const std::string& other) const
{
	const char * buffer = asCString();
	if (buffer == NULL)
		return false;

	return other.compare(buffer) == 0;
}

bool JValue::operator==(const double& other) const
{
	return numEqual(*this, other);
}

bool JValue::operator==(const int64_t& other) const
{
	return numEqual(*this, other);
}

bool JValue::operator==(int32_t other) const
{
	return numEqual(*this, other);
}

bool JValue::operator==(bool other) const
{
	bool value;
	if (asBool(value) == CONV_OK)
		return value == other;
	return false;
}

JValueArrayElement JValue::operator[](int index) const
{
	return JValue(jvalue_copy(jarray_get(m_jval, index)));
}

JValueArrayElement JValue::operator[](const std::string& key) const
{
	return this->operator[](j_str_to_buffer(key.c_str(), key.size()));
}

JValueArrayElement JValue::operator[](const char* key) const
{
	return this->operator[](j_str_to_buffer(key, strlen(key)));
}

JValueArrayElement JValue::operator[](const raw_buffer& key) const
{
	return JValueArrayElement(jvalue_copy(jobject_get(m_jval, key)));
}

JValue::operator bool() const
{
	return jis_valid(m_jval);
}

bool JValue::put(size_t index, const JValue& value)
{
	long safe_value = 0;
	if (index < static_cast<unsigned long>(std::numeric_limits<long>::max()))
	{
		safe_value = static_cast<long>(index);
	}
	else
	{
		PJ_LOG_WARN("Warning: Value cannot be safely cast to int.");
    }
	return jarray_set(m_jval, safe_value, value.peekRaw());
}

bool JValue::put(const std::string& key, const JValue& value)
{
	return put(JValue(key), value);
}

bool JValue::put(const JValue& key, const JValue& value)
{
	return jobject_set2(m_jval, key.peekRaw(), value.peekRaw());
}

bool JValue::remove(ssize_t idx)
{
	return jarray_remove(m_jval, idx);
}

bool JValue::remove(const char *key)
{
	raw_buffer buf;
	buf.m_str = key;
	buf.m_len = strlen(key);
	return jobject_remove(m_jval, buf);
}

bool JValue::remove(const std::string &key)
{
	raw_buffer buf;
	buf.m_str = key.c_str();
	buf.m_len = key.length();
	return jobject_remove(m_jval, buf);
}

bool JValue::remove(const JValue &key)
{
	if (!jis_string(key.m_jval))
		return false;
	return jobject_remove(m_jval, jstring_get_fast(key.m_jval));
}

JValue& JValue::operator<<(const JValue& element)
{
	if (!append(element))
		return Null();
	return *this;
}

JValue& JValue::operator<<(const KeyValue& pair)
{
	if (!put(pair.first, pair.second))
		return Null();
	return *this;
}

bool JValue::append(const JValue& value)
{
	return jarray_set(m_jval, jarray_size(m_jval), value.peekRaw());
}

std::string JValue::stringify(const char *indent)
{
	const char *str = jvalue_prettify(m_jval, indent);

	return str ? str : "";
}

bool JValue::hasKey(const std::string& key) const
{
	return jobject_get_exists(m_jval, strToRawBuffer(key), NULL);
}

ssize_t JValue::objectSize() const
{
	return jobject_size(m_jval);
}

ssize_t JValue::arraySize() const
{
	return jarray_size(m_jval);
}

bool JValue::isValid() const
{
	return jis_valid(m_jval);
}

JValueType JValue::getType() const
{
	return jget_type(m_jval);
}

bool JValue::isNull() const
{
	return jis_null(m_jval);
}

bool JValue::isNumber() const
{
	return jis_number(m_jval);
}

bool JValue::isString() const
{
	return jis_string(m_jval);
}

bool JValue::isObject() const
{
	return jis_object(m_jval);
}

bool JValue::isArray() const
{
	return jis_array(m_jval);
}

bool JValue::isBoolean() const
{
	return jis_boolean(m_jval);
}

template <>
ConversionResultFlags JValue::asNumber<int32_t>(int32_t& number) const
{
	return jnumber_get_i32(m_jval, &number);
}

template <>
ConversionResultFlags JValue::asNumber<int64_t>(int64_t& number) const
{
	return jnumber_get_i64(m_jval, &number);
}

template <>
ConversionResultFlags JValue::asNumber<double>(double& number) const
{
	return jnumber_get_f64(m_jval, &number);
}

template <>
ConversionResultFlags JValue::asNumber<std::string>(std::string& number) const
{
	raw_buffer asRaw;
	ConversionResultFlags result;

	result = jnumber_get_raw(m_jval, &asRaw);
	if (result == CONV_OK)
		number = std::string(asRaw.m_str, asRaw.m_len);

	return result;
}

template <>
ConversionResultFlags JValue::asNumber<NumericString>(NumericString& number) const
{
	std::string num;
	ConversionResultFlags result;

	result = asNumber(num);
	number = num;

	return result;
}

template <>
int32_t JValue::asNumber<int32_t>() const
{
	int32_t result = 0;
	asNumber(result);
	return result;
}

template <>
int64_t JValue::asNumber<int64_t>() const
{
	int64_t result = 0;
	asNumber(result);
	return result;
}

template <>
double JValue::asNumber<double>() const
{
	double result = 0;
	asNumber(result);
	return result;
}

template <>
std::string JValue::asNumber<std::string>() const
{
	std::string result;
	asNumber(result);
	return result;
}

template <>
NumericString JValue::asNumber<NumericString>() const
{
	return NumericString(asNumber<std::string>());
}

const char * JValue::asCString() const
{
	if (!isString()) {
		return NULL;
	}

	raw_buffer backingBuffer = jstring_get_fast(m_jval);

	return backingBuffer.m_str;
}

ConversionResultFlags JValue::asString(std::string &asStr) const
{
	if (!isString()) {
		return CONV_NOT_A_STRING;
	}

	raw_buffer backingBuffer = jstring_get_fast(m_jval);
	if (backingBuffer.m_str == NULL) {
		asStr = "";
		return CONV_NOT_A_STRING;
	}

	asStr = std::string(backingBuffer.m_str, backingBuffer.m_len);

	return CONV_OK;
}

ConversionResultFlags JValue::asBool(bool &result) const
{
	return jboolean_get(m_jval, &result);
}

NumericString::operator JValue()
{
	return JValue(*this);
}

//!@endcond Doxygen_Suppress

}
