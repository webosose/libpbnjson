// Copyright (c) 2009-2023 LG Electronics, Inc.
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

#ifndef JVALUE_CXX_H_
#define JVALUE_CXX_H_

#include <pbnjson.h>

#include "japi.h"
#include "JResult.h"
#include "../c/jconversion.h"
#include "../c/jcallbacks.h"
#include "../c/jtypes.h"
#include "../c/compiler/deprecated_attribute.h"
#include "../c/compiler/cpp11.h"

#include <string>
#ifdef CPP11
#include <utility>
#include <initializer_list>
#else
#include <algorithm>
#endif

#include <stdint.h>
#include <unistd.h>	// for ssize_t

// Unused headers that we need to keep, because some pbnjson users relay on
// their existance after JValue.h/pbnjson.hpp inclusion
#include <vector>
#include <iostream>
#include <stdexcept>
#include <stack>

namespace pbnjson {

class JValueArrayElement;
class NumericString;

/**
 * This class represents an opaque object containing a JSON value.
 * The type that it represents can be any valid JSON value:
 * @li Object: As a string, represented as {"key":value,"key2":value} where values are any JSON type. Equivalent to the concept of an unordered map.
 * @li Array: As a string, represented by [] with comma-separated elements.  Elements are any JSON type.
 * @li String: Represented by text surrounded with double-quotes.  Double-quotes within the string are escaped with '\'
 * @li Number: An abstract number type that can be represented in several forms as a string (+/-XXXX.YYYYEZZZZ) where almost all the parts
 *             are optional.
 * @li Boolean: As a string, "true" or "false" (without quotes).
 * @li Null: As a string, "null" (without quotes).
 * @see http://www.JSON.org
 */
class PJSONCXX_API JValue : public JResult
{
	friend class JDomParser;
	friend class JGenerator;
	friend class JValidator;
	friend class JSchema;
	friend class JQuery;
	friend JValue Object();
	friend JValue Array();

private:

	/// Static JSON `null` object
	static JValue JNULL;
	/// Static JSON invalid object
	static JValue JINVALID;

	jvalue_ref m_jval;

	static JValue& Null()
	{
		return JNULL;
	}

	static JValue& Invalid()
	{
		return JINVALID;
	}

	const char * asCString() const;

protected:
	JValue(jvalue_ref toOwn)
		: m_jval(toOwn)
	{
		if (toOwn == NULL)
			m_jval = JNULL.m_jval;
	}

	jvalue_ref grabOwnership()
	{
		jvalue_ref transferred = m_jval;
		m_jval = JNULL.m_jval;
		return transferred;
	}

public:
	/**
	 * @brief The InvalidType struct Deprecated type, used for ABI compatibility
	 */
	struct InvalidType : std::runtime_error
	{ InvalidType(std::string const &arg) : std::runtime_error(arg) { } } DEPRECATED_API;

	/**
	 * Convenience alias for representing a key/value pair within a JSON object.
	 */
	typedef std::pair<JValue, JValue> KeyValue;

	//{@
	/**
	 * Constructs an object representing JSON null.
	 *
	 * @see Object()
	 * @see Array()
	 */
	/**
	 * Constructs the default JSON value (NULL).
	 *
	 * @see pbnjson::Object
	 * @see pbnjson::Array
	 * @see template<class V> JValue(const V& v)
	 * @see JValue(const char *)
	 */
	JValue();

	/**
	 * Regular copy constructor. The newly constructed JValue object references
	 * the same JSON value as the other. Use duplicate() to create a copy
	 * of the referenced JSON.
	 *
	 * @see duplicate()
	 */
	JValue(const JValue& other);

#ifdef CPP11
	/**
	 * Move constructor. Constructed JValue object represent original object,
	 * while original is valid, but in undefined state
	 *
	 * @since C++11
	 *
	 * @param[in] other Value to move in
	 */
	JValue(JValue&& other)
		: JValue()
	{ swap(other); }

	JValue(std::initializer_list<JValue::KeyValue> l)
		: m_jval(jobject_create())
	{
		for (const auto& kv : l)
			put(kv.first, kv.second);
	}
#endif

	/**
	* Creates JValue, that takes ownership of passed jvalue_ref.
	*
	* @param[in] value a value to adopt
	* @return Newly created JValue.
	*/
	static JValue adopt(jvalue_ref value)
	{
		return JValue(value);
	}

	/**
	 * Construct primitive JSON value from basic types
	 *
	 * The supported C types are converted as follows:
	 *
	 * |  C type                                  | JSON type               |
	 * | :--------------------------------------  | :---------------------- |
	 * | std::string                              | JSON string             |
	 * | int32_t, int64_t, double, NumericString  | JSON number             |
	 * | bool                                     | JSON bool               |
	 * | nullptr                                  | JSON null (since C++11) |
	 *
	 * @see JObject()
	 * @see JArray()
	 * @see NumericString
	 */
	//@{
	/**
	 * Construct boolean value
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const bool    v);
	/**
	 * Construct number value from int_32
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const int32_t v);
	/**
	 * Construct number value from int_64
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const int64_t v);
	/**
	 * Construct number value from double
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const double  v);
	/**
	 * Construct string value
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const char        *v);
	/**
	 * Construct string value
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const std::string &v);
	/**
	 * Construct JSON value from JSON array element
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const JValueArrayElement &v);
#ifdef CPP11
	/**
	 * Construct JSON null value
	 */
	JValue(decltype(nullptr) const&) : m_jval(JNULL.m_jval) { }
#endif
	/**
	 * Construct number value from numeric string
	 *
	 * @param[in] v The value to convert to a JSON value.
	 */
	JValue(const NumericString &v);
	//@}

	/**
	 * Create a new instance of JSON value, identical to this one but completely independent.
	 *
	 * @return The reference to newly created JSON value.
	 */
	JValue duplicate() const;

	/**
	 * Swap this instance of JSON value with provided one
	 *
	 * @param[in] other Value to swap with
	 */
	void swap(JValue& other)
	{
		JResult::swap(other);
		std::swap(m_jval, other.m_jval);
	}
	//@}

	~JValue();

	//{@
	/**
	 * Copy the other JSON value into this one.
	 *
	 * @param[in] other The JSON value to copy.
	 * @return The reference to this object except with its contents re-assigned to be a copy of the
	 *         other value.
	 */
	JValue& operator=(const JValue& other);

#ifdef CPP11
	/**
	 * Move assignment operator. Moves the other JSON value into this one.
	 * Original object goes into the undefined state. Self assignment
	 * lead to undefined behavior.
	 *
	 * @since C++11
	 *
	 * @param[in] other The JSON value to move in
	 * @return The reference to this object except that its content is re-assigned
	 */
	JValue& operator=(JValue&& other)
	{
		swap(other);
		return *this;
	}
#endif

	/**
	 * Convenience method - negation of what the equality operator would return
	 *
	 * @param[in] other JSON object to compare with
	 * @return see #operator== for details
	 */
	template <class T>
	bool operator!=(const T& other) const { return !((*this) == other); }

	/**
	 * Test this object against another object.
	 *
	 * @param[in] other The JSON value to compare against
	 * @return Returns True if both the compared objects have the same JSON value and type. In essence,
	 *         they must both have the same JSON type & contain the same contents. For example if comparing objects
	 *         then they must have the same key-value pairs, if comparing arrays then they must have the same elements
	 *         in the same order.
	 */
	bool operator==(const JValue& other) const;
	/**
	 * Test this object against a string.
	 *
	 * @param[in] other The value to test this JSON string against.
	 * @return True if this object represents a JSON string with the same bytes.
	 *
	 * @note If the original JSON string is encoded with utf-8 then they must be compared manually
	 */
	bool operator==(const std::string& other) const;
	/**
	 * Test this object against a C string.
	 *
	 * @param[in] other null-terminated C string to test this JSON string against.
	 * @return True if this object represents a JSON string with the same bytes.
	 *
	 * @note If the original JSON string is encoded with utf-8 then they must be compared manually
	 */
	bool operator==(const char * other) const;
	/**
	 * Test this object against a floating point number.
	 * @param[in] other The value to test this JSON number against
	 * @return True if this object represents a JSON number with the same value.
	 */
	bool operator==(const double& other) const;
	/**
	 * Test this object against a 64-bit signed integer.
	 *
	 * @param[in] other The value to test this JSON number against.
	 * @return True if this object represents a JSON number that can be converted
	 * to a 64-bit signed integer without error and it has a value equivalent to the requested value.
	 */
	// using const with pass-by-reference so that we minimize the amount of copies on 32-bit machines
	// and make it easier for the compiler to optimize
	bool operator==(const int64_t& other) const;
	/**
	 * Test this object against a 32-bit signed integer.
	 *
	 * @param[in] other The value to test this JSON number against.
	 * @return True if this object represents a JSON number that can be converted
	 * to a 32-bit signed integer without error and it has a value equivalent to the requested value.
	 */
	bool operator==(int32_t other) const;
	/**
	 * Test whether this object has the requested boolean value.
	 *
	 * @param[in] other The value to test this JSON boolean against.
	 * @return True if this object represents a JSON boolean with the given value.
	 */
	bool operator==(bool other) const;
	//@}

	/**
	 * Looks up the value in this array with the given index
	 *
	 * @note This operator does not distinguish between failures. Failure can
	 *       be due to one of the following reasons:
	 *     -#  This value doesn't represent a JSON array
	 *     -#  The index is out of bounds
	 *     -#  The value associated with that index is null
	 *
	 * @param[in] index The key to look up
	 * @return The value at the requested index.
	 * @see JValue::isNull
	 * @see JValue::hasKey
	 * @see JValue::isArray
	 */
	JValueArrayElement operator[](int index) const;

	//{@
	/**
	 * Looks up the value associated with the given key from the object.
	 *
	 * @param[in] key The key to look up
	 * @return The value associated with the key, JINVALID if key is missing.
	 * @see JValue::isValid
	 * @see JValue::hasKey
	 * @see JValue::isObject
	 */
	JValueArrayElement operator[](const std::string& key) const;

	/**
	 * Looks up the value associated with the given key from the object.
	 *
	 * @param[in] key The key to look up
	 * @return The value associated with the key, JINVALID if key is missing.
	 *
	 * @see JValueArrayElement::operator[]
	 */
	JValueArrayElement operator[](const char* key) const;

	/**
	 * Looks up the value associated with the given key from the object.
	 *
	 * @param[in] key The key to look up
	 * @return The value associated with the key, JINVALID if key is missing.
	 *
	 * @see JValueArrayElement::operator[]
	 */
	JValueArrayElement operator[](const raw_buffer& key) const;
	//@}

	/**
	  * Return if object is valid
	  *
	  * @return false if is JINVALID or NULL
	 */
#ifdef CPP11
	explicit
#endif
	operator bool() const;


	/**
	 * @brief Converts the JSON value to it's equivalent string representation
	 *        that is ready to be transferred across the wire (with all appropriate
	 *        escaping and quoting performed). With pretty-print option.
	 *
	 * @param indent An indent for pretty-printed format. Allowed symbols: \\n, \\v, \\f, \\t, \\r and space.
	 *               Combinations of them are permitted as well. Optional. NULL by default
	 * @return The string representation of the JSON value
	 */
	std::string stringify(const char *indent = NULL);

	/**
	 * Returns whether or not this JSON object has a key/value pair with the given key.
	 *
	 * @param[in] key The key to find in the object
	 * @return True if this JValue represents a JSON object & the object contains
	 *         the key.
	 */
	bool hasKey(const std::string& key) const;

	/**
	 * Returns the count of members in a JSON object.
	 *
	 * @return The count of members of the object, or -1 if this isn't an object.
	 */
	ssize_t objectSize() const;

	/**
	 * Returns the length of this JSON array.
	 *
	 * @return The size of the array, or -1 if this isn't an array.
	 */
	ssize_t arraySize() const;

	/**
	 * Returns underlying jvalue_ref
	 *
	 * @note Returned value shouldn't be manually freed
	 * @return Underlying jvalue_ref
	 */
	jvalue_ref peekRaw() const
	{
		return m_jval;
	}

	//{@
	/**
	 * Insert a JSON value into this array.
	 *
	 * @param[in] i The index to insert into.  If it is past the end of the array, then
	 *              all intermediary elements are initialized to NULL.  Any elements >= index,
	 *              are shifted up by 1.
	 * @param[in] value The JSON value to insert at that index.
	 * @return True if this object represents a JSON array & the element was successfully inserted at the requested location.
	 * @see put(size_t, const JValue&)
	 */
	bool put(long i, const JValue& value)
	{
		if (i < 0)
			return false;
		return put((size_t)i, value);
	}

	/**
	 * Insert a JSON value into this array.
	 *
	 * @param[in] i The index to insert into.  If it is past the end of the array, then
	 *              all intermediary elements are initialized to NULL.  Any elements >= index,
	 *              are shifted up by 1.
	 * @param[in] value The JSON value to insert at that index.
	 * @return True if this object represents a JSON array and the element was successfully inserted at the requested location.
	 * @see put(size_t, const JValue&)
	 */
	bool put(int i, const JValue& value)
	{
		return put((long)i, value);
	}

	/**
	 * Insert a JSON value into this array.
	 *
	 * @param[in] index The index to insert into.  If it is past the end of the array, then
	 *              all intermediary elements are initialized to NULL.  Any elements >= index,
	 *              are shifted up by 1.
	 * @param[in] value The JSON value to insert at that index.
	 * @return True if this object represents a JSON array and the element was successfully inserted at the requested location.
	 */
	bool put(size_t index, const JValue& value);
	//@}

	//{@
	/**
	 * Convenience method for adding a value to a JSON object.
	 *
	 * This behaves like a regular map if the key already exists.
	 *
	 * @param[in] key name of a key
	 * @param[in] value Any JSON object.
	 * @return True if this object represents a JSON object, the key represents a JSON string, and the key/value pair was successfully inserted.
	 */
	bool put(const std::string& key, const JValue& value);

	/**
	 * Add a key/value pair to a JSON object.
	 *
	 * This behaves like a regular map if the key already exists.
	 *
	 * @param[in] key An object representing a JSON string
	 * @param[in] value Any JSON object.
	 * @return True if this object represents a JSON object, the key represents a JSON string, and the key/value pair was successfully inserted.
	 */
	bool put(const JValue& key, const JValue& value);

	/**
	 * Convenience method for adding a value to a JSON object.
	 *
	 * This behaves like a regular map if the key already exists.
	 *
	 * @param[in] key name of a key
	 * @param[in] value Any JSON object.
	 * @return True if this object represents a JSON object and the key/value pair was successfully inserted.
	 */
	bool put(const char *key, const JValue& value)
	{
		return put(std::string(key), value);
	}
	//@}

	/**
	* @brief Remove the element that is located at the position specified by
	* the index from the given array.
	*
	* NOTE: It is unspecified what happens if an invalid index is passed.
	*       Currently this will result in a log message.
	*
	* @param idx The index of the element to be removed
	* @return True if the element was removed, false if the value is not an array, index is out of bounds,
	*         or some problem occured in the removal.
	* @see jarray_remove(jvalue_ref, ssize_t)
	*/
	bool remove(ssize_t idx);

	/**
	* Remove any key/value association in the object with the specified key value.
	*
	* @param key The key to use
	* @return True if there was an association under the key. False if there was not.
	*/
	bool remove(const char *key);

	/**
	* Remove any key/value association in the object with the specified key value.
	*
	* @param key The key to use
	* @return True if there was an association under the key. False if there was not.
	* @see remove(const char *)
	*/
	bool remove(const std::string &key);

	/**
	* Remove any key/value association in the object with the specified key value.
	*
	* @param key The key to use
	* @return True if there was an association under the key. False if there was not.
	* @see remove(const char *)
	*/
	bool remove(const JValue &key);

	/**
	 * Convenience method for appending an element to an array.
	 *
	 * If this isn’t an array or some other error occurs during insertion then a JSON null is returned.
	 *
	 * @param[in] element The JSON value to append to this array.
	 * @return A reference to this object if this represents a JSON array or JSON null otherwise.
	 */
	JValue& operator<<(const JValue& element);

	/**
	 * Convenience method for appending an element to an array.
	 *
	 * If this isn’t an array or some other error occurs during insertion then a JSON null is returned.
	 *
	 * @param[in] element Native value, which can be passed to the JValue constructor.
	 *
	 * @return A reference to this object if this represents a JSON array or JSON null otherwise.
	 */
	template <typename T>
	JValue& operator<<(const T& element)
	{
		return operator<<(JValue(element));
	}

	/**
	 * Convenience method for adding a key-value pair to an object.
	 *
	 * If this isn't an object, or some other error during insertion occurred, then the return is a JSON null.
	 *
	 * @param[in] pair key-value pair
	 * @return A reference to this object if this value represents a JSON object or JSON null otherwise.
	 */
	JValue& operator<<(const KeyValue& pair);

	/**
	 * Convenience method for appending to a JSON array.
	 *
	 * @param[in] value The JSON value to append to this array
	 * @return True if this value is an array & the value was added successfully.
	 */
	bool append(const JValue& value);

	/**
	 * Determines whether or not this JSON value is valid. Parsing functions can return invalid json object if an error occurs
	 * @return True if this is an invalid value, false otherwise
	 */
	bool isValid() const;

	/**
	 * Get JSON value type.
	 *
	 * @return Type of JSON value
	 */
	JValueType getType() const;

	/**
	 * Determines whether or not this JSON value is null
	 * @return True if this is a JSON null, false otherwise
	 */
	bool isNull() const;

	/**
	 * Determines whether or not this JSON value is number
	 * @return True if this is a JSON number, false otherwise.
	 */
	bool isNumber() const;

	/**
	 * Determines whether or not this JSON value is string
	 * @return True if this is a JSON string, false otherwise.
	 */
	bool isString() const;
	/**
	 * Determines whether or not this JSON value is object
	 * @return True if this is a JSON object, false otherwise.
	 */
	bool isObject() const;
	/**
	 * Determines whether or not this JSON value is array
	 * @return True if this is a JSON array, false otherwise.
	 */
	bool isArray() const;
	/**
	 * Determines whether or not this JSON value is boolean
	 * @return True if this is a JSON boolean, false otherwise.
	 */
	bool isBoolean() const;

	//{@
	/**
	 * Store the numeric representation of this number in the requested native type.
	 *
	 * This library doesn't make any assumptions about the native type to use when representing the number.  Instead,
	 * when the library is asked to convert it to a native type (e.g. int, double), it will convert it & return a conversion result
	 * indicating how accurately the native type represents the original string.
	 *
	 * @note Rounding errors in native floating point representation are not necessarily accounted for.
	 * For example, if the serialized JSON contained 0.01, the returned value may return
	 * CONV_OK even though the native type will not be exactly 0.01 - this may change in future).
	 *
	 * @note Integers are indistinguishable from floating point numbers (even if the serialized form is X.000000).  This is by design.
	 *
	 * @param[out] number Pass-by-reference to the variable to store the converted result in.
	 * @return CONV_OK if this objects represents a number that was stored in the native type without any problems.
	 */
	template <class T>
	ConversionResultFlags asNumber(T& number) const;

	/**
	 * Converts this JSON value to a native numeric type.
	 *
	 * @note Behaviour is undefined if this JSON value does not represent a number.
	 *
	 * @return The native numeric representation of this JSON value.
	 */
	template <class T>
	T asNumber() const
	{
		T result;
		asNumber(result);
		return result;
	}
	//@}

	/**
	 * Store the text within this JSON value (if it is a JSON string) within the STL string.
	 *
	 * @param[out] asStr Pass-by-reference to the STL string to copy in to
	 * @return CONV_OK if this JSON value represents a JSON string, otherwise some error code.
	 */
	ConversionResultFlags asString(std::string& asStr) const;

	/**
	 * Converts this JSON value to a native string type. This is a convenience method.
	 *
	 * Behaviour is undefined if this JSON value does not represent a string.
	 *
	 * @return The native string representation of this JSON value.
	 *
	 * @note Use of this method is not recommended
	 */
	std::string asString() const
	{
		std::string result;
		asString(result);
		return result;
	}

	/**
	 * Converts this JSON value to a native boolean type.
	 *
	 * @param[out] value Pass-by-reference to the boolean to assign
	 * @return CONV_OK if this JSON value represents a JSON boolean.
	 */
	ConversionResultFlags asBool(bool &value) const;

	/**
	 * Convenience method to convert to a boolean.
	 *
	 * @note Use of this method is not recommended, use asBool(bool &value) instead
	 */
	bool asBool() const
	{
		bool result;
		asBool(result);
		return result;
	}

	template <typename It>
	class IteratorFactory
	{
		friend class JValue;

	public:
		It begin() const { return It(_origin); }
		It end() const { return It(); }

		~IteratorFactory() { j_release(&_origin); }

	private:
		IteratorFactory(const JValue& val)
			: _origin(jvalue_copy(val.m_jval))
		{
		}

		jvalue_ref _origin;
	};

	/**
	 * The iterator class for values in a JSON array.
	 */
	class ArrayIterator
			: public std::iterator<std::input_iterator_tag, JValue, void, void, void>
	{
		friend class JValue;
		friend class IteratorFactory<ArrayIterator>;

	public:
		typedef size_t size_type;

		/**
		 * Construct an iterator with a detached state, which is equivalent to the end
		 * position. This iterator is suitable for equality comparison with any other iterator,
		 * and if another iterator is equal to ArrayIterator() then that iterator is at end of the collection.
		 *
		 * @note It is undefined behaviour to apply any other operation to a newly constructed iterator.
		 */
		ArrayIterator();

		/**
		  * Destruct an interator
		  */
		~ArrayIterator();

		/**
		 * Construct iterator and copy iterator data from @p other
		 *
		 * @param other Other iterator
		 */
		ArrayIterator(const ArrayIterator& other);

		/**
		 * Copy iterator data from @p other
		 *
		 * @param other Other iterator
		 *
		 * @return Current iterator
		 */
		ArrayIterator& operator=(const ArrayIterator& other);

		/**
		 * Pre-increment the iterator to the next element.
		 *
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \return This iterator except it has moved 1 element forward.
		 */
		ArrayIterator& operator++();

		/**
		 * Post-increment the iterator to the next element.
		 *
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \see ArrayIterator::operator++()
		 * \return An iterator representing the current element (not the next element)
		 */
		ArrayIterator operator++(int);

		/**
		 * Pre-decriment the iterator to the previous element.
		 *
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \return This iterator except it has moved 1 element backward.
		 */
		ArrayIterator& operator--();

		/**
		 * Post-decriment the iterator to the previous element.
		 *
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \see ArrayIterator::operator--()
		 * \return An iterator representing the current element (not the previous element)
		 */
		ArrayIterator operator--(int);

		/**
		 * Jump n elements forward.
		 *
		 * \note Behaviour is unspecified if you try to jump to an element after the last one.
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \return An iterator representing n elements ahead.
		 */
		ArrayIterator operator+(size_t n) const;

		/**
		 * Jump n elements backward.
		 *
		 * \note Behaviour is unspecified if you try to jump to an element after the first one.
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \return An iterator representing n elements behind.
		 */
		ArrayIterator operator-(size_t n) const;

		/**
		 * Convenience operator for determining inequality.
		 *
		 * @return True if the iterators point to different key/value pairs.
		 * @see operator==(const ArrayIterator&)
		 */
		bool operator!=(const ArrayIterator& other) const { return !(operator==(other)); }

		/**
		 * Determine equality between iterators - do they point to the same index.
		 *
		 * @note Behaviour is unspecified if array is modified during iteration.
		 *
		 * @param[in] other The iterator to compare to
		 * @return True if both iterators are for the same array and represent the same point in iteration.
		 */
		bool operator==(const ArrayIterator& other) const;

		/**
		 * Return the const reference to the value which this iterator points to.
		 *
		 * @return JValue object. If this is not a valid iterator, then JSON null is returned.
		 */
		const value_type operator*() const;

	private:
		explicit ArrayIterator(jvalue_ref parent);

	private:
		jvalue_ref _parent;
		size_type  _index;
	};

	/**
	 * The iterator class for key/value pairs in a JSON object.
	 */
	class ObjectIterator
		: public std::iterator<std::input_iterator_tag, JValue, void, void, void>
	{
		friend class JValue;
		friend class IteratorFactory<ObjectIterator>;

	public:
		// A detached state, which is equivalent to the end position.
		// Is suitable for equality comparison with any other iterator,
		// and if another iterator is equal to ObjectIterator(),
		// the iterator is at end of the collection.
		//
		// It is undefined behaviour to apply any other operation to a defaultly
		// constructed iterator.

		/**
		 * @brief ObjectIterator defautl cunstructor of ObjectIterator class
		 */
		ObjectIterator();

		/**
		 * Destruct Object Iterator
		 */
		~ObjectIterator();

		/**
		 * @brief ObjectIterator Copy contructor
		 * @param other other instance
		 */
		ObjectIterator(const ObjectIterator& other);

		/**
		 * @brief operator = assignment operator.
		 * @param other instance of ObjectIterator
		 * @return created instance of ObjectIterator
		 */
		ObjectIterator& operator=(const ObjectIterator& other);

		/**
		 * Pre-increment the iterator to the next element.
		 *
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \return This iterator except it has moved 1 element forward.
		 */
		ObjectIterator& operator++();

		/**
		 * Post-increment the iterator to the next element.
		 *
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 *
		 * \see ObjectIterator::operator++()
		 * \return An iterator representing the current element (not the next element)
		 */
		ObjectIterator operator++(int);

		/**
		 * Jump n elements forward. Note that this is an O(n) operation. Consider using
		 * std::advance() as this operator may be deprecated in the future.
		 *
		 * \param n Count of elements
		 *
		 * \return An iterator representing n elements ahead.
		 *
		 * \note Behaviour is unspecified if you try to jump to an element after the last one.
		 * \note Behaviour is undefined if you try to call it with end() iterator.
		 */
		ObjectIterator operator+(size_t n) const;

		/**
		 * Convenience operator for determining inequality.
		 *
		 * \param other Iterator to compare with
		 *
		 * @return True if the iterators point to different key/value pairs.
		 * @see operator==(const ObjectIterator&)
		 */
		bool operator!=(const ObjectIterator& other) const { return !(this->operator==(other)); }

		/**
		 * Determine equality between iterators - do they point to the same key/value pair.
		 *
		 * @note Behaviour is unspecified if objects are modified during iteration.
		 *
		 * @param[in] other The iterator to compare to
		 * @return True if both iterators are for the same object and represent the same point in iteration.
		 */
		bool operator==(const ObjectIterator& other) const;

		/**
		 * Retrieve the key-value pair this iterator represents.
		 *
		 * \note If you iterate, the key/value stored within the reference are undefined.
		 *
		 * @return A pair of JValue objects.  The key is a string and the value is any JSON value.  If this is not a valid iterator,
		 *         then JSON null is returned for both fields.
		 */
		const KeyValue operator*() const;

	private:
		explicit ObjectIterator(jvalue_ref parent);

	private:
		jobject_iter _it;
		jvalue_ref _parent;
		jobject_key_value _key_value;
		bool _at_end;
	};

	typedef const ArrayIterator  ArrayConstIterator;
	typedef const ObjectIterator ObjectConstIterator;

	/**
	 * Create an iterator over JSON array elements
	 */
	IteratorFactory<ArrayConstIterator> items() const
	{
		return IteratorFactory<ArrayConstIterator>(*this);
	}

	/**
	 * Create an iterator over JSON object properties
	 */
	IteratorFactory<ObjectConstIterator> children() const
	{
		return IteratorFactory<ObjectConstIterator>(*this);
	}

	/**
	 * Get the beginning iterator for this object.
	 *
	 * \note Behaviour is undefined if this isn't an object.
	 *
	 * \note Behaviour is currently undefined if you try to modify an object while iterating.
	 *       This is untested & likely unsafe.
	 *
	 * @note Iterating over this array is equivalent to walking a linked list
	 *
	 * @return An iterator representing the key/value pairs within this JSON object.
	 *
	 * \see end()
	 * \see begin() const
	 */
	ObjectIterator begin() DEPRECATED_API;
	/**
	 * Const-safe version of the iterator.
	 * \see begin()
	 * \return An iterator representing the key/value pairs within this JSON object.
	 */
	ObjectConstIterator begin() const DEPRECATED_API;

	/**
	 * Get the iterator representing the position after the last key-value pair in this object.
	 *
	 * \note Behaviour is undefined if this isn't an object.
	 *
	 * \note Behaviour is currently undefined if you try to modify an object while iterating.
	 *       This is untested & likely unsafe.
	 *
	 * \note Behaviour is undefined if you try to iterate the returned iterator.
	 *
	 * \return An iterator representing the position after the last key/value pair in this JSON object.
	 *
	 * \see begin()
	 * \see end() const
	 */
	ObjectIterator end() DEPRECATED_API;
	/**
	 * Const-safe version of the iterator.
	 *
	 * Get the constant  iterator representing the position after the last key-value pair in this object.
	 *
	 * \note Behaviour is undefined if this isn't an object.
	 *
	 * \note Behaviour is undefined if you try to iterate the returned iterator.
	 *
	 * \return An iterator representing the position after the last key/value pair in this JSON object.
	 *
	 * \see begin()
	 * \see end()
	 */
	ObjectConstIterator end() const DEPRECATED_API;
};

/**
 * This class represents a JSON object.
 */
class JObject : public JValue
{
public:

	/**
	 * Create an empty JSON object.
	 */
	JObject() : JValue(jobject_create())
	{ }

#ifdef CPP11
	/**
	 * Create a JSON object, which has its keys and values set from an initializer list.
	 *
	 * @note Enclose key/value pair in braces to mark a JValue::KeyValue pair. For example:
	 *
	 * @code
	 *   JValue v = JObject {
	 *     { "lvl1_key1", 12 },
	 *     { "lvl1_key2", JObject { {"lvl2_key1", 13} } }
	 *   };
	 * @endcode
	 */
	JObject(std::initializer_list<JValue::KeyValue> l)
		: JValue(l)
	{ }
#endif
};

/**
 * This class represent a JSON array.
 */
class JArray : public JValue
{
public:

	/**
	 * Create an empty JSON array.
	 */
	JArray() : JValue(jarray_create(NULL))
	{ }

#ifdef CPP11
	/**
	 * Create a JSON object, filled with values from initializer list.
	 *
	 * You can use it to create filled array in place as in following example:
	 *
	 * @code
	 *   JValue v = JArray { "string", 3.14, true, nullptr, 42 };
	 *   JArray v { nullptr, nullptr, nullptr };
	 * @endcode
	 */
	JArray(std::initializer_list<JValue> l)
		: JValue(jarray_create(NULL))
	{
		for (auto v : l)
			append(v);
	}
#endif
};

/**
 * Convenience class for creating an already serialized form of a number.
 * This is useful if you want to avoid any serialization from native types to
 * string (or if you want a specific representation when serialized).
 *
 * @note All the constructors are explicit to avoid collisions with creating JSON
 * strings.
 */
class NumericString : public std::string
{
public:
	/**
	 * Construct numeric string
	 */
	explicit NumericString() : std::string("0") {}
	//explicit NumericString(const char *str) : std::string(str) {}
	template <class T>
	/**
	 * Construct numeric string copy
	 *
	 * @param other Source numerical string
	 */
	explicit NumericString(const T& other) : std::string(other) {}
	/**
	 * Construct numeric string from the string
	 *
	 * @param str string
	 * @param len string length
	 */
	NumericString(const char *str, size_t len) : std::string(str, len) {}

	/**
	 * Numerical string assignment operators. Make copy of the source string.
	 */
	//{@
	NumericString& operator=(const NumericString& other) { std::string::operator=(other); return *this; }
	NumericString& operator=(const std::string& other) { std::string::operator=(other); return *this; }
	NumericString& operator=(const char *other) { std::string::operator=(other); return *this; }
	//@}
	/**
	 * Compare string with another strings.
	 */
	//{@
	bool operator==(const NumericString& other) const { return this->operator==(reinterpret_cast<const std::string&>(other)); }
	bool operator==(const std::string& other) const { return reinterpret_cast<const std::string&>(*this) == other; }
	//@}

	/**
	 * Cast numerical string to a JSON value
	 */
	operator JValue();
};

/**
 * Create an empty JSON object node.
 *
 */
JValue Object();

/**
 * Construct an object representing a JSON array.
 */
JValue Array();

/*! \name asNumber template specializations
 * The different explicit specializations of the templatized asNumber
 * @see JValue::asNumber(T&) const
 */
//{@

/// asNumber template specializations
template <>
ConversionResultFlags JValue::asNumber<int32_t>(int32_t& value) const;

/// asNumber template specializations
template <>
ConversionResultFlags JValue::asNumber<int64_t>(int64_t& value) const;

/// asNumber template specializations
template <>
ConversionResultFlags JValue::asNumber<double>(double& value) const;
///asNumber template specializations
template <>
ConversionResultFlags JValue::asNumber<std::string>(std::string& value) const;
///asNumber template specializations
template <>
ConversionResultFlags JValue::asNumber<NumericString>(NumericString& value) const;

///asNumber template specializations
template <>
int32_t JValue::asNumber<int32_t>() const;

///asNumber template specializations
template <>
int64_t JValue::asNumber<int64_t>() const;

///asNumber template specializations
template <>
double JValue::asNumber<double>() const;

///asNumber template specializations
template <>
std::string JValue::asNumber<std::string>() const;
///asNumber template specializations
template <>
NumericString JValue::asNumber<NumericString>() const;
/// @}

/**
 * Class represents a JSON array element
 */
class PJSONCXX_API JValueArrayElement
	: public JValue
{
	friend class JValue;

	/**
	 * Construct new array element from the JSON value
	 *
	 * @param value Source JSON value
	 */
	JValueArrayElement(const JValue & value)
		: JValue(value)
	{ }
};

/**
 * Swap two instances of JSON values
 *
 * @param[in] _one JValue to swap with
 * @param[in] _two JValue to swap with
 */
inline void
swap(JValue& _one, JValue& _two)
{ _one.swap(_two); }

}

#endif /* JVALUE_H_ */
