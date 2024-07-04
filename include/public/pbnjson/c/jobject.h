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

#ifndef JOBJECT_H_
#define JOBJECT_H_

#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>	// for ssize_t
#include <assert.h>
#include <string.h>

#include "japi.h"
#include "jschema.h"
#include "jconversion.h"
#include "compiler/pure_attribute.h"
#include "compiler/nonnull_attribute.h"
#include "compiler/unused_attribute.h"
#include "compiler/builtins.h"
#include "jtypes.h"
#include "jerror.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file jobject.h
 *
 * jobject.h file provides a set of functions to manipulate the json object. The name of the function contains words that define its behaviour.
 * Also words are used in explanation of a function. The following description explains what each word means:
 *
 * Create - creates a JSON value with ownership given to the caller.  This call will usually only ever return NULL if memory allocation failed or a reference to a valid
 *          JSON value (never a reference to a JSON null).  Still, it is recommended to use jis_valid to check the result (in fact, always use jis_null instead of actually
 *          comparing against some value).  For instance, the creation of a number from a double might return null if you attempt to provide it with a NaN or an infinity or any other
 *          unsupported value. Example: #jobject_create
 *
 * Copy - retrieve ownership of the specified value.  Any changes to val must be reflected in the resultant value. Example: #jvalue_copy
 *
 * Release - release ownership of the specified value (after a Release, it is forbidden to use the pointer)
 *
 * Get - no ownership change - only valid for as long as the parent has an owner for the duration the child is used.
 *
 * Set/Add - no ownership change - both the parent that has a value set and the original owner have valid references (parent will grab a copy)
 *
 * Put/Append - ownership is transferred to the jvalue_ref parent ( you lose ownership - the lifetime of the object becomes that of the parent unless you have explicitely
 *              retrieved a copy before-hand)
 *
 * Remove - ownership is removed from the parent (& the parent will lose all references to the value).
 *
 * NOTE: Any unspecified behaviours listed are not an exhaustive list (and do not necessarily appear as a NOTE but will be attempted to be documented as such).
 * Any usage of the API that isn't explicitly documented, or is marked as simply the current implementation of unspecified behaviour,
 * is considered unspecified and suceptible to breakage at any time.  If a particular unspecified behaviour is necessary, please contact the maintainer to
 * update the documentation & change the API if appropriate.  Unspecified behaviour may result in crashes or incorrect program behaviour.
 *
 * NOTE: It is unspecified the actual value that a reference to a JSON null value will have (it is possible for it to be different from a C NULL & it is in the current implementation).
 *       The only guarantee is that jis_null will properly tell you if the value is a reference to a JSON null value or not.
 *
 * NOTE: It is unspecified the behaviour of the DOM inspection/modification functions if a NULL pointer
 *       or a JSON null reference is passed as the DOM node.
 *
 * NOTE: Unspecified behaviour results from passing in a C-NULL instead of the result from jnull() in to the API that is expecting a jvalue_ref.  In the current implementation it might be
 *       safe sometimes to do so.
 *
 * NOTE: It is unspecified what happens if the string pointer is null or the length is 0 when a raw_buffer is used
 *       to retrieve parameters from an object.  It is unspecified what happens is jis_null returns true for a jvalue_ref
 *       used as the key to lookup within an object.
 *
 * If a parameter to a function is marked as NON_NULL, then the behaviour is undefined if NULL is passed in
 * (with a release binary, it is likely to crash).  It is the callers responsibility to ensure that NULL is checked for.
 */

/*** Generic JSON Value operations ***/
/**
 *
 * @brief Returns a reference to a JSON value and increases reference count.
 *
 * Returns a reference to a same JSON value and increases reference count. Caller is responsible to release returned object
 *
 * @note Returns NULL if passed reference points to NULL
 * @note It is useless to call this API on the result of jnull() or jstring_empty(), although it is safe (effectively a NO-OP).
 */
PJSON_API jvalue_ref jvalue_copy(jvalue_ref val);

/**
 * @brief Create a deep copy of the JSON value.
 *
 * Create a deep copy of the JSON value so that modifications in val are guaranteed to not
 * be seen in the copy.
 *
 * The implementation may cheat however with immutable objects by still doing a reference count.
 *
 * @param val JSON value to duplicate
 * @return full copy of incoming json value
 */
PJSON_API jvalue_ref jvalue_duplicate(jvalue_ref val);

/**
 * @brief Check if two JSON values are identical.
 *
 * Check if two JSON values are identical. The objects are compared by values
 *
 * @param val1 JSON value to compare with
 * @param val2 JSON value to compare with
 * @return true if values are identical, otherwise false
 */
PJSON_API bool jvalue_equal(jvalue_ref val1, jvalue_ref val2) NON_NULL(1, 2);

/**
 * @brief Compare if one JSON value less, greater or identical with other.
 *
 * Compare if one JSON value less, greater or identical with other.
 * If value types are different rules of ordering are: JINVALID < JV_NULL < JV_BOOL < JV_NUM < JV_STR < JV_ARRAY < JV_OBJECT.
 * Strings are compared lexicographically, arrays are compared by values, objects are compared by keys and values.
 *
 * @param val JSON value to compare with
 * @param other JSON value to compare with
 * @return <0 if val less than other, >0 if val greater then other, otherwise 0
 */
PJSON_API int jvalue_compare(const jvalue_ref val1, const jvalue_ref val2) NON_NULL(1, 2);

/**
 * @brief Release ownership from *val.  *val has an undefined value afterwards.
 *
 * Release ownership from *val.  *val has an undefined value afterwards.  It is an error
 * to call this on references for which ownership does not preside with the caller
 * (i.e. only call when there was a _create or _copy call was made without a corresponding put, append, or release)
 *
 * It is safe to call this as many times as you want on jnull(), jstring_empty(), or NULL.
 *
 * @param val A pointer to a value reference to release ownership for.  In DEBUG mode, the reference is changed to some garbage value afterwards.
 */
PJSON_API void j_release(jvalue_ref *val);

/**
 * @brief Returns a reference to a value representing an invalid JSON null value.
 *
 * Returns a reference to a value representing an invalid JSON null value. It is
 * redundant (but not illegal) to copy or release ownership on this reference
 * since the implementation must guarantee that this reference has static
 * program scope.
 *
 * NOTE: Do not use this to check if jvalue is valid or not
 */
PJSON_API jvalue_ref jinvalid() PURE_FUNC;

/**
 * @brief Lets the caller determines whether or not the reference points to valid JSON value
 *
 * Lets the caller determines whether or not the reference points to valid JSON value
 * @param val A reference to a JSON value
 * @return true if val is a reference to a valid JSON value.  false otherwise
 */
PJSON_API bool jis_valid(jvalue_ref val);

/**
 * @brief Returns a reference to a value representing a JSON null.
 *
 * Returns a reference to a value representing a JSON null.  It is redundant (but not illegal) to copy
 * or release ownership on this reference since the implementation must guarantee that this reference has static
 * program scope.
 *
 * NOTE: Do not use this to check if jvalue_ref is a JSON null value
 */
PJSON_API jvalue_ref jnull() PURE_FUNC;

/**
 * @brief Returns the type of a JSON value (null, bool, str, etc.)
 *
 * Returns the type of a JSON value (null, bool, str, etc.)
 */
PJSON_API JValueType jget_type(jvalue_ref val);

/**
 * @brief Check if jvalue_ref is a reference to a JSON null value
 *
 * Check if jvalue_ref is a reference to a JSON null value
 * @param val A reference to a JSON value
 * @return true if val is a reference to a JSON null value.  false otherwise
 */
PJSON_API bool jis_null(jvalue_ref val);

/**
 * @brief Check validity of jvalue against schema.
 *
 * Check validity of jvalue against schema. The function is able to solve external references. In order to check
 * schema with externals, provide a appropriate resolver.
 *
 * @param val A reference to the JSON object to check
 * @param schema A schema with resolver
 * @return true if val is valid against schema
 *
 * @deprecated Use @ref jvalue_validate instead
 *
 */
PJSON_API bool jvalue_check_schema(jvalue_ref jref, const JSchemaInfoRef schema_info) NON_NULL(1, 2);

/**
 * @brief Check validity of jvalue against the schema.
 *
 * Check validity of jvalue against the schema.
 *
 * @param val A reference to the JSON object to check
 * @param schema A schema
 * @param err pbnjson error information
 * @return true, if val is valid against schema, false otherwise
 */
PJSON_API bool jvalue_validate(const jvalue_ref val, const jschema_ref schema, jerror **err) NON_NULL(1, 2);

/**
 * @brief jvalue_apply_schema is similar to jvalue_check_schema.
 *
 * jvalue_apply_schema is similar to jvalue_check_schema.
 * The difference is in behaviour with default values.
 * jvalue_apply_schema modifies jref by inserting default values(specified by schema).
 *
 * @param val A reference to the JSON object to check
 * @param schema A schema with resolver
 * @return true if val is valid against schema
 *
 * @see jvalue_check_schema
 *
 * @deprecated Use @ref jvalue_validate_apply instead
 */
PJSON_API bool jvalue_apply_schema(jvalue_ref val, const JSchemaInfoRef schema) NON_NULL(1, 2);

/**
 * @brief Check validity of jvalue against the schema.
 *
 * Check validity of jvalue against the schema.
 * Insert to the JSON object default values specified by the schema.
 *
 * @param val A reference to the JSON object to check
 * @param schema A schema
 * @param err pbnjson error information
 * @return true if val is valid against schema, false otherwise
 *
 * @see jvalue_validate
 */
PJSON_API bool jvalue_validate_apply(jvalue_ref val, const jschema_ref schema, jerror **err) NON_NULL(1, 2);

/**
 * @brief Equivalent to JSON.stringify within Javascript.
 *
 * Equivalent to JSON.stringify within Javascript.  Converts the JSON value to it's equivalent
 * string representation that is ready to be transferred across the wire (with all appropriate escaping and quoting performed).
 * The c-string returned has a lifetime equivalent at least to the lifetime of the reference provided.
 * It is unspecified as to whether or not it is also tied to the lifetime of any copies of the reference.
 *
 * @param val A reference to the JSON object to convert to a string.
 * @param schema The schema to validate against when converting to a string
 * @return The string representation of the value with a lifetime equivalent to the value reference. NULL if there is an error of any
 *         kind (e.g. schema validation failed)
 *
 * NOTE: FOR THE IMPLEMENTOR - even if it is possible to extend the lifetime, it's not necessarily smart if Copy-On-Write is used since a write
 * might cause weirdness conceptually with the lifetime of the buffer if the parent already had its ownership released (best not to encourage
 * such API usage).
 *
 * NOTE: This function doesn't resolve any references to external additions in the schema.
 * @deprecated Will be removed in 3.0. Use jvalue_stringify after jvalue_validate
 * @see jvalue_tostring_schemainfo()
 */
PJSON_API const char *jvalue_tostring(jvalue_ref val, const jschema_ref schema) NON_NULL(1, 2);

/**
 * @brief This function is similar to a function jvalue_tostring.
 *
 * This function is similar to a function jvalue_tostring, except that it does not perform schema validation.
 *
 * @param val A reference to the JSON object to convert to a string.
 * @return The string representation of the value with a lifetime equivalent to the value reference, or
 *		   NULL if there is an error of any kind
 * @deprecated Will be removed in 3.0. Use jvalue_stringify
 *
 * @see jvalue_tostring
 */
PJSON_API const char *jvalue_tostring_simple(jvalue_ref val) NON_NULL(1);

/**
 * @brief Just like jvalue_tostring(), but is able to resolve external references in the given schema.
 *
 * Just like jvalue_tostring(), but is able to resolve external references in the given schema.
 *
 * @param val JSON value to serialize.
 * @param schemainfo This is used to carry useful information to the parser.
 *          m_schema is expected to contain the schema
 *          m_resolver should contain a proper schema resolver
 * @deprecated Will be removed in 3.0. Use already resolved schema
 * @see JSchemaInfo
 * @see jvalue_tostring()
 */
PJSON_API const char *jvalue_tostring_schemainfo(jvalue_ref val, const JSchemaInfoRef schemainfo) NON_NULL(1, 2);

/*** JSON Object operations ***/
/**
 * @brief Create an empty JSON object node.
 *
 * Create an empty JSON object node.
 */
PJSON_API jvalue_ref jobject_create();

/**
 * @brief Create a JSON object node initialized with a variable number of items.
 *
 * Create a JSON object node initialized with a variable number of items.
 *
 * The last jobject_key_value item must have a NULL key pointer (an actual C NULL, not JSON null).  If the last object
 * in argument is not NULL - behaviour is undefined. Use #J_END_OBJ_DECL as the last argument.
 *
 * NOTE: If item.key isn't a valid JSON string or NULL pointer, then a JSON null is returned.
 *
 * @param item A key value pair
 * @param ... The remaining jobject_key_value items.
 *
 * @return A JSON Object with the specified key-value pairs
 *
 * @see #jobject_put
 * @see #J_END_OBJ_DECL
 */
PJSON_API jvalue_ref jobject_create_var(jobject_key_value item, ...);

/**
 * @brief Last argument in method jobject_create_var
 *
  */
#define J_END_OBJ_DECL ((jobject_key_value) {NULL, NULL})

/**
 * @brief  Create an empty JSON object node that is prepared to contain the requested number of
 * key-value pairs.
 *
 * Create an empty JSON object node that is prepared to contain the requested number of
 * key-value pairs.
 */
PJSON_API jvalue_ref jobject_create_hint(int capacityHint);

/**
 * @brief Check if a JSON value reference an object or not.
 *
 * Check if a JSON value reference an object or not.
 * @param val The reference to test
 * @return True if val represents an object, false otherwise
 */
PJSON_API bool jis_object(jvalue_ref val);

/**
 * @brief  Returns a reference to a JSON value within the parent object that has the specified key.
 *
 * Returns a reference to a JSON value within the parent object that has the specified key.
 *
 * @param obj The reference to the parent object to retrieve the value from
 * @param key The name of the key in the JSON object to retrieve the value for.
 * @param value A pointer to a JSON value reference which will contain the result.
 *              It is safe for this to be NULL.
 *
 * @return True if key is present within object, false if it is not or obj is not a valid object.
 *         NOTE: It is unspecified what *value is set to if this returns false.
 *
 * @see jobject_get
 */
PJSON_API bool jobject_get_exists(jvalue_ref obj, raw_buffer key, jvalue_ref *value);

/**
 * @brief Alternate method to determines whether or not the object contains a key.
 *
 * Alternate method to determines whether or not the object contains a key.
 *
 * This allows retrieval using a JSON value instead of a simple string buffer.
 *
 * @param obj json object to look in
 * @param key json string with key name
 * @param value returned json value
 * @return True if obj is an object and it has a key/value pair matching the specified key
 *
 * @see #jobject_get_exists
 * @see #jobject_containskey2
 */
PJSON_API bool jobject_get_exists2(jvalue_ref obj, jvalue_ref key, jvalue_ref *value);

/**
 * @brief Convenience method to determines whether or not the object contains a key.
 *
 * Convenience method to determines whether or not the object contains a key.
 *
 * @param obj json object to look in
 * @param key json string with key name
 * @return True if obj is an object and it has a key/value pair matching the specified key.
 */
static inline bool jobject_containskey(jvalue_ref obj, raw_buffer key)
{
	return jobject_get_exists(obj, key, NULL);
}

/**
 * @brief Alternate convenience method to determine whether or not the object contains a key.
 *
 * Alternate convenience method to determine whether or not the object contains a key.
 *
 * @param obj json object to look in
 * @param key json string with key name
 * @return returns true if an object has a key, otherwise returns false
 *
 * @see #jobject_containskey
 * @see #jobject_get_exists2
 */
static inline bool jobject_containskey2(jvalue_ref obj, jvalue_ref key)
{
	return jobject_get_exists2(obj, key, NULL);
}

/**
 * @brief Retrieve a reference to a JSON value within the parent object that has the specified key.
 * Ownership remains with obj.
 *
 * Retrieve a reference to a JSON value within the parent object that has the specified key.
 * Ownership remains with obj.
 *
 * Convenience wrapper which directly returns the JSON value or NULL if the value could not be retrieved.
 * This API can be useful where there is no need to distinguish between the key not being present,
 * obj not being a JSON object, or it containing a null
 *
 * @param obj The reference to the parent object to retrieve the value from
 * @param key The name of the key in the JSON object to retrieve the value for
 *
 * @return A reference to the JSON value associated with key under obj or a JSON null
 *
 * @see jobject_get_exists
 */
PJSON_API jvalue_ref jobject_get(jvalue_ref obj, raw_buffer key);

/**
 * @brief Returns jvalue by path specified in a variadic list.
 *
 * Returns jvalue by path specified in a variadic list. Under path it means - sequence of object/arrays/keys to reach necessary level
 *
 * @param obj The reference to the parent object to retrieve the value from
 * @param ... A variadic list of nested keys to reach the desired property.
 *
 * @return A reference to the JSON value associated with the sequence of the keys or JINVALID
 */
PJSON_API jvalue_ref jobject_get_nested(jvalue_ref obj, ...);

/**
 * @brief Remove any key/value association in the object with the specified key value.
 *
 * Remove any key/value association in the object with the specified key value.
 *
 * @param key The key to use
 * @param obj The reference to the parent object.
 * @return True if there was an association under key.  False if there was not or obj was not an object.
 */
PJSON_API bool jobject_remove(jvalue_ref obj, raw_buffer key);

/**
 * @brief Associate val with key in object obj.
 *
 * Associate val with key in object obj.  The object associates a copy of val.
 *
 * NOTE: It is unspecified whether or not changes to val after being set are reflected in the structure under obj.
 * NOTE: The RFC specifes behaviour is unspecified if key is already present within the object.  The implementation
 *       behaviour is that existing key/value pairs are replaced (insert/replace semantics).
 *
 * @param obj The JSON object to insert into
 * @param key The key to use for the association
 * @param val The reference to a JSON object containing the value to associate with key
 * @return True if the association was made, false otherwise.  Failure may occur for any number of reasons
 * (e.g. key is already present in the object, obj is not a JSON object, failure to allocate memory, etc)
 */
PJSON_API bool jobject_set(jvalue_ref obj, raw_buffer key, jvalue_ref val);

/**
 * @brief  Associate val with key in object obj.
 *
 * Associate val with key in object obj.  The object associates a copy of key and val.
 *
 * NOTE: It is unspecified whether or not changes to val after being set are reflected in the structure under obj.
 * NOTE: The RFC specifes behaviour is unspecified if key is already present within the object.  The implementation
 *       behaviour is that existing key/value pairs are replaced (insert/replace semantics).
 *
 * @param obj The JSON object to insert into
 * @param key The reference to a JSON string containing the key for the association
 * @param val The reference to a JSON object containing the value to associate with key
 * @return True if the association was made, false otherwise.  Failure may occur for any number of reasons
 * (e.g. key is already present in the object, obj is not a JSON object, failure to allocate memory, etc)
 */
PJSON_API bool jobject_set2(jvalue_ref obj, jvalue_ref key, jvalue_ref val);

/**
 * @brief Associate val with key in object obj.
 *
 * Associate val with key in object obj.  The object takes over ownership of val.  It is an error to call this
 * if you do not have ownership of val to begin with.
 *
 * NOTE: Behaviour of library is undefined when changes to key or val occur after transferring ownership to obj
 * NOTE: The RFC specifes behaviour is unspecified if key is already present within the object.  The implementation
 *       behaviour is that existing key/value pairs are replaced (insert/replace semantics).
 *
 * @param obj The JSON object to insert into
 * @param key The key to use for the association.  Ownership transfers to the object
 *            NOTE: Behaviour is undefined if key is not a valid JSON string
 * @param val The reference to the JSON object to associate with the key
 *            NOTE: Behaviour is undefined if val is a NULL pointer.
 * @return True if the association was made, false otherwise.  Failure may occur for any number of reasons
 * (e.g. key is already present in the object, obj is not a JSON object, failure to allocate memory, etc)
 */
PJSON_API bool jobject_put(jvalue_ref obj, jvalue_ref key, jvalue_ref val);

/**
 * @brief Return number of key-value pairs in object obj;
 *
 * Return number of key-value pairs in object obj;
 *
 * @param obj The JSON object to get size
 * @return number of key-value pairs in object
 */
PJSON_API size_t jobject_size(jvalue_ref obj);


// JSON Object iterators
/**
 * @brief Create an iterator for the object.
 *
 * Create an iterator for the object.  The iterator is allocated on the stack & will be automatically
 * cleaned.
 *
 * NOTE: It is assumed that ownership of obj is maintained for the lifetime of the iterator by the caller.
 * NOTE: Behaviour is undefined if the ownership is released while an iterator still exists.
 * NOTE: Behaviour is undefined if the object changes while iterating over it.
 *
 * @param iter Pointer to an iterator instance to be initialized
 * @param obj The JSON object to iterate over
 * @return true if iterator was created, false if the JSON value isn't an object.
 */
PJSON_API bool jobject_iter_init(jobject_iter *iter, jvalue_ref obj);

/**
 * @brief Obtain key-value pair of the object, advance the iterator to the next pair.
 *
 * Obtain key-value pair of the object, advance the iterator to the next pair.
 *
 * NOTE: Behaviour is unspecified if the iterator has not been initialized.
 *
 * Typical usage is the following:
  @code
    jobject_iter it;
    jobject_key_value key_value;

    jobject_iter_init(&it, obj);
    while (jobject_iter_next(&it, &key_value))
    {
        // Do whatever is neededed with the key_value
    }
  @endcode
 *
 * @param iter The iterator to use
 * @param keyval Structure to capture the current key-value pair.
 * @return true if more pairs are available, false if the end is reached.
 */
PJSON_API bool jobject_iter_next(jobject_iter *iter, jobject_key_value *keyval);

/*** JSON Array operations ***/
/**
 * @brief Create an empty array with the specified properties.
 *
 * Create an empty array with the specified properties.
 *
 * @param opts The options for the array (currently unspecified).  NULL indicates use default options.
 * @return A reference to the created array value.  The caller has ownership.
 */
PJSON_API jvalue_ref jarray_create(jarray_opts opts);

/**
 * @brief Creates a json array initialized with elements provided in parameters.
 *
 * Creates a json array initialized with elements provided in parameters.
 *
 * @param opts The options for the array (currently unspecified).  NULL indicates use default options.
 * @param ... A variadic list of JSON value references (jvalue_ref).  This is equivalent to calling jarray_create
 *     and then passing each reference through a jarray_put.  The last object must be a NULL pointer (not a JSON null -
 *     those are inserted into the array as elements)
 *
 * @return Created json array. User is responsible to release it
 */
PJSON_API jvalue_ref jarray_create_var(jarray_opts opts, ...);

/**
 * @brief last object in method jarray_create_var
 */
#define J_END_ARRAY_DECL NULL

/**
 * @brief Create an empty array.
 *
 * Create an empty array with the specified properties and the hint that the array will eventually contain capacityHint elements.
 *
 * @param opts The options for the array (currently unspecified).  NULL indicates use default options.
 * @param capacityHint A guess-timate of the eventual size of the array (implementation is free to ignore this).
 * @return A reference to the created array value.  The caller has ownership.
 */
PJSON_API jvalue_ref jarray_create_hint(jarray_opts opts, size_t capacityHint);

/**
 * @brief Determine whether or not the reference to the JSON value represents an array.
 *
 * Determine whether or not the reference to the JSON value represents an array.
 *
 * @param val The reference to test
 * @return True if it is an array, false otherwise.
 */
PJSON_API bool jis_array(jvalue_ref val);

/**
 * @brief Returns number of elements in the array.
 *
 * Returns number of elements in the array.
 *
 * @param arr The reference to the array
 * @return The number of elements in the array or -1 if this is not an array or there is some other problem.
 */
PJSON_API ssize_t jarray_size(jvalue_ref arr) NON_NULL(1);

/**
 * @brief Provides a reference to the index'th element of the array.
 *
 * Provides a reference to the index'th element of the array.
 *
 * NOTE: A JSON null is returned if arr is not an array or index is out of bounds
 *
 * @param arr The reference to the array
 * @param index The element number in the array to retrieve.
 * @return A reference to the value.  Ownership of this value remains with the parent array. jis_valid will return false on the
 * result if the index'th element is null or invalid parameters are provided.
 */
PJSON_API jvalue_ref jarray_get(jvalue_ref arr, ssize_t index) NON_NULL(1);

/**
 * @brief  Remove the element located at the position specified by index from the array.
 *
 * Remove the element located at the position specified by index from the array. arr[index] now no longer contains val, with all elements shifted appropriately.
 *
 * NOTE: It is unspecified what happens if an invalid index is passed.  Currently this will result in a log message being recorded
 *
 * @param arr The reference to the array
 * @param index The element to remove
 * @return True if the element was removed, false if arr is not an array, index is out of bounds, or some problem
 * occured in the removal.
 */
PJSON_API bool jarray_remove(jvalue_ref arr, ssize_t index) NON_NULL(1);

/**
 * @brief Set the index'th element of arr to val.
 *
 * Set the index'th element of arr to val. The array will
 * contain a copy of the new element.  Any element already at position index gets overwritten.
 *
 * Arrays can be implicitly resized by setting an index greater than the size of the array.
 *
 * NOTE: Do not modify var after calling this function as it may have unpredictable results.
 *
 * @param arr The array to add an element to
 * @param index The element to change to val.  If index is out of bounds of the current size (e.g. setting element 10 of a 0-length array), the
 *              indicies in between are initialized with null.
 * @param val The element of the array
 * @return True if the element was successfully set, false otherwise.
 * @see jarray_put
 */
PJSON_API bool jarray_set(jvalue_ref arr, ssize_t index, jvalue_ref val) NON_NULL(1);

/**
 * @brief Set the index'th element of arr to val.
 *
 * Set the index'th element of arr to val. Ownership of val is transferred to arr.
 *
 * Sparse arrays can be implicitly created by setting an index greater than the size of the array.
 *
 * NOTE: Do not modify var after calling this function as it may have unpredictable results.
 *
 * @param arr The array to add an element to
 * @param index The element to change to val
 * @param val Reference to the value to put into the array
 * @return True if the element was successfully changed, false otherwise.
 * @see jarray_set
 */
PJSON_API bool jarray_put(jvalue_ref arr, ssize_t index, jvalue_ref val) NON_NULL(1, 3);

/**
 * @brief Insert the value into the array before the specified position.
 *
 * Insert the value into the array before the specified position.
 *
 * arr[index] now contains val, with all elements shifted appropriately.
 *
 * NOTE: Do not modify var after calling this function as it may have unpredictable results.
 *
 * @param arr json array
 * @param index position to insert. The element will be insreted before it.
 * @param val json value to insert
 *
 * @see jarray_append
 * @see jarray_put
 */
PJSON_API bool jarray_insert(jvalue_ref arr, ssize_t index, jvalue_ref val) NON_NULL(1, 3);

/**
 * @brief Convenience method
 *
 * Convenience method that is equivalent to
 * @code jarray_put(arr, jarray_size(), val) @endcode
 * @return True if the value was successfully appended, false otherwise.
 * @see jarray_put
 * @see jarray_set
 */
PJSON_API bool jarray_append(jvalue_ref arr, jvalue_ref val) NON_NULL(1, 2);

/**
 * @brief Remove the specified number of elements from the given index, & then insert the array subset there.
 * Additional ownership of the copied elements is not retained.
 *
 * Similar syntax to the Javascript splice.  Remove the specified number of elements from the given index, & then insert the array subset there.
 * Additional ownership of the copied elements is not retained.
 *
 * @param array1 The array to insert into/remove from
 * @param index The index of array1 to make modifications to
 * @param toRemove The number of elements to remove prior to insertion
 * @param array2 The array to insert from
 * @param begin The beginning index in array2 to start insertion from
 * @param end The final index (exclusive) in array2 to end insertion at (jarray_size(array2) represents the last element).
 * @param ownership What to do with the elements going from array2 into array1.  You can transfer ownership (meaning the element is then owned
 *                  by array1) or copy (meaning a copy of the element is used to insert into array1 when splicing, but array2 retains ownership as well)
 *
 * @see jarray_splice_inject
 * @see jarray_splice_append
 */
PJSON_API bool jarray_splice(jvalue_ref array, ssize_t index, ssize_t toRemove, jvalue_ref array2, ssize_t begin, ssize_t end, JSpliceOwnership ownership) NON_NULL(1, 4);

/**
 * @brief Wrapper to insert all elements from the second array into the first at the given position.
 * Additional ownership of the copied elements is not retained.
 *
 * Simpler wrapper to insert all elements from the second array into the first at the given position.
 * Additional ownership of the copied elements is not retained.
 *
 * @param array1 The array to insert into
 * @param index The position to insert into (e.g. jarray_size(array) to append to array1)
 * @param array2 The array to insert from
 * @param ownership What to do with the elements going from array2 into array1.  You can transfer ownership or copy.
 *
 * @see jarray_splice
 * @see jarray_splice_append
 */
PJSON_API bool jarray_splice_inject(jvalue_ref array, ssize_t index, jvalue_ref arrayToInject, JSpliceOwnership ownership) NON_NULL(1, 3);

/**
 * @brief Append all the contents of array2 to array1.
 *
 * Append all the contents of array2 to array1.
 * Additional ownership of the copied elements is not retained.
 *
 * @param array The array to insert into
 * @param arrayToAppend The array to copy from
 * @param ownership What to do with the elements going from array2 into array1.  You can transfer ownership or copy.
 *
 * @see jarray_splice
 * @see jarray_splice_inject
 */
PJSON_API bool jarray_splice_append(jvalue_ref array, jvalue_ref arrayToAppend, JSpliceOwnership ownership) NON_NULL(1, 2);

/*** JSON String operations ***/
/**
 * @brief Create an empty JSON string.
 *
 * Create an empty JSON string.
 *
 * @return A reference to the JSON string
 */
PJSON_API jvalue_ref jstring_empty() PURE_FUNC;

/**
 * @brief Create a JSON string representing the ASCII or Unicode (UTF-8 encoded) string stored in the provided buffer.
 * Any encoding conversions are the responsibility of the caller.
 *
 * NOTE: Terminator character (1 byte) is appended to the end of value in the resultant copy
 * NOTE: The string need not be NULL-terminated since the length parameter has to have a valid value.
 *
 * @param toCopy The buffer to copy from.
 * @return A reference to the JSON string
 *
 * @see jstring_get_fast
 */
PJSON_API jvalue_ref jstring_create_copy(raw_buffer str);

/**
 * @brief Determine whether or not the JSON value is a string
 *
 * Determine whether or not the JSON value is a string
 *
 * @return True if this is a reference to a JSON value that is a string, false otherwise.
 */
PJSON_API bool jis_string(jvalue_ref str) NON_NULL(1);

/**
 * @brief Create JSON string so that it contains the same string.
 *
 * Create this JSON string so that it contains the same string (this will make a copy of the string).
 * If constant strings are used (or strings that outlive the life of the top-level JSON value) use jstring_create_fast.
 * This will potentially determine the size of the string using something like strlen,
 * so if this is unsafe, then use one of the other string APIs.
 *
 * This is implemented as a convenience wrapper for jstring_create_utf8
 *
 * NOTE: Terminator character (1 byte) is appended to the end of value in the resultant copy
 * NOTE: Undefined behaviour if this is a null pointer or the string is encoded with a format that isn't UTF-8 compatible
 *
 * @param cstring A null-terminated c-string.
 * @return A reference to the newly created JSON string
 *
 * @see jstring_create_utf8
 * @see jstring_create_copy
 */
PJSON_API jvalue_ref jstring_create(const char* cstring);

/**
 * @brief Creates a json string.
 *
 * Creates a json string. The string data is copied.
 *
 * Equivalent to jstring_create except safer in case you choose to change the encoding later on (you are responsible for
 * determining the length of the string).
 *
 * This is likely implemented as a convenience wrapper for jstring_create_copy
 *
 * NOTE: Terminator character (1 byte) is appended to the end of value in the resultant copy
 *
 * @param string An arbitrary Unicode (UTF-8 encoded) string.
 * @param length The number of bytes in the string.
 *
 * @see jstring_create
 * @see jstring_create_copy
 */
PJSON_API jvalue_ref jstring_create_utf8(const char *string, ssize_t length);

/**
 * @brief Convenience method to create an empty JSON string
 *
 * Convenience method to create an empty JSON string
 *
 * @return A reference to the JSON string
 */
PJSON_API jvalue_ref jstring_empty();

/**
 * @brief Convenience wrapper for jstring_create_nocopy_full
 *
 * Convenience wrapper for jstring_create_nocopy_full.  Equivalent to
 * @code
 * jstring_create_nocopy_full(j_str_to_buffer(str, strLen), NULL);
 * @endcode
 *
 * Typical usage might be
 * @code
 * jstring_create_nocopy(j_str_to_buffer(str, strLen))
 * @endcode
 *
 * @param val buffer with string
 * @return A reference to the JSON string
 *
 * @see jstring_create_full
 */
PJSON_API jvalue_ref jstring_create_nocopy(raw_buffer val);

/**
 * @brief Replace internal string buffer in jstring.
 *
 * Replace internal string buffer in jstring. buffer_dealloc function will be called to release buffer, when the jstring is released.
 * The function does not copy the buffer. The ownership is transfered.
 *
 * NOTE: It is unspecified whether references to JSON strings that are copies of JSON strings with raw backing buffers make copies
 * or simply maintain a Copy-On-Write reference to the raw backing buffer.
 *
 * @param val The UTF string (specific encoding is unimportant) to set.  Cannot be a value on the stack unless it outlives
 *              the lifetime of the created object.
 *              NOTE: Undefined behaviour if val.m_str is NULL
 * @param buffer_dealloc The deallocator to use on the provided string after it is no longer needed.  If NULL, then
 *                       the string is never de-allocated (potential memory leak if it was dynamically allocated).  Use NULL
 *                       if value is on the stack or a compile-time constant or you are managing it yourself.
 * @return A reference to the JSON string
 */
PJSON_API jvalue_ref jstring_create_nocopy_full(raw_buffer val, jdeallocator buffer_dealloc);

/**
 * @brief Return the number of bytes in the buffer backing the JSON string.
 *
 * Return the number of bytes in the buffer backing the JSON string.  Convenience method.
 * Equivalent to jstring_get(str).m_len
 *
 * @param str The JSON string reference to examine
 * @return The number of bytes in the buffer backing this string (including any terminating NULLs).
 * NOTE: A negative return value represents some unspecified error (should never happen - just for assert purposes to catch overflows).
 */
PJSON_API ssize_t jstring_size(jvalue_ref str) NON_NULL(1);

/**
 * @brief Get the C-string buffer representation for this JSON string.
 *
 * Get the C-string buffer representation for this JSON string.
 *
 * The caller is responsible for freeing the returned buffer.
 *
 * NOTE: if this is not a string, then NULL is assigned to the m_str value & m_len is undefined.
 *
 * @param str The JSON string reference
 * @return A C-String (null-terminated) copy of the backing buffer of this JSON string.
 *         This assumes the backing buffer is UTF-8.
 * @see jstring_free_buffer
  */
PJSON_API raw_buffer jstring_get(jvalue_ref str) NON_NULL(1);

/**
 * @brief Frees a string allocated by jstring_get().
 *
 * Frees a string allocated by jstring_get().  Equivalent to manually
 * calling free() on the m_str member of raw_buffer.
 *
 * @param str The string allocated by jstring_get()
 * @see jstring_get()
 */
static inline void jstring_free_buffer(raw_buffer str)
{
	free((void *)str.m_str);
}

/**
 * @brief Get the backing buffer for this JSON string.
 *
 * Get the backing buffer for this JSON string. There is no allocation, existing buffer is returned.
 *
 * NOTE: if this is not a string, then NULL is assigned to the m_str value & m_len is undefined.
 *
 * @param str The JSON string reference
 * @return The backing buffer of this JSON string (may not be NULL-terminated).
 * NOTE: result.m_str is NULL & m_len is undefined if this was not a string.
 */
PJSON_API raw_buffer jstring_get_fast(jvalue_ref str) NON_NULL(1);

/**
 * @brief Determines whether or not this JSON string matches another JSON string
 *
 * Determines whether or not this JSON string matches another JSON string
 *
 * @param str first json string to compare
 * @param other second json string to compare
 * @return True if and only if both are valid strings & are textually the same.
 */
PJSON_API bool jstring_equal(jvalue_ref str, jvalue_ref other);

/**
 * @brief Determines whether or not this JSON string matches a raw string
 *
 * Determines whether or not this JSON string matches a raw string
 *
 * @param str The JSON string to compare with
 * @param other The raw string to compare with
 * @return True if and only str is actually a JSON string & matches exactly the other string (ending null-terminator is optional - the length must be accurate and not including
 *           the terminating null)
 */
PJSON_API bool jstring_equal2(jvalue_ref str, raw_buffer other);

/**
 * @brief Creates a JSON number.
 *
 * Creates a JSON number that uses this decimal string-representation as the backing value.
 * This is safe, as compared with jnumber_create_unsafe, in that the string buffer can be modified or freed after this call
 * (the reference will maintain it's own distinct copy)
 *
 * @param raw The string buffer to use.  Need not be null-terminated.
 * @return The JSON number reference
 *
 * @see jnumber_create_unsafe
 */
PJSON_API jvalue_ref jnumber_create(raw_buffer str);

/**
 * @brief Like jnumber_create except the provided buffer is used directly.
 *
 * Like jnumber_create except the provided buffer is used directly.  It may not be freed or modified
 * unless there are no ways that this object or any copies could possibly access this buffer - this generally means
 * that all copies have their ownership released or no attempt is made to inspect this DOM node (converting this or any parent
 * JSON node into a string has the side-effect of causing inspection)
 *
 * @param str The buffer to use.  Need not be null-terminated.  Must be immutable for the lifetime of all copies of
 *               the created JSON reference.
 * @param strFree The deallocator to use on the string (if any) after this JSON object is freed (e.g. if you dynamically allocated
 *                the string and want this library to take care of automatically deallocating it).
 * @return The JSON number reference
 * @see jnumber_create
 */
PJSON_API jvalue_ref jnumber_create_unsafe(raw_buffer str, jdeallocator strFree);

/*** JSON Number operations ***/
/**
 * @brief Creates a reference to a JSON value representing the requested number.
 *
 * Creates a reference to a JSON value representing the requested number.
 *
 * @param number The number to use when initializing this JSON number.
 * @return A reference to a JSON number representing the requested value.
 *         <br>NOTE: It is considered an error to try to create a reference to a NaN or +/- infinity value.  A null JSON reference will be returned in this case.
 *         <br>NOTE: It is unspecified what happens if you try to use features of floating point such as subnormal numbers or -0.
 */
PJSON_API jvalue_ref jnumber_create_f64(double number);

/**
 * @brief Creates a reference to a JSON value representing the requested number.
 *
 * Creates a reference to a JSON value representing the requested number.
 *
 * @return A reference to a JSON number representing the requested value.
 */
PJSON_API jvalue_ref jnumber_create_i32(int32_t number);

/**
 * @brief Creates a reference to a JSON value representing the requested number.
 *
 * Creates a reference to a JSON value representing the requested number.
 *
 * @param number The number the JSON value should represent
 * @return A reference to a JSON number representing the requested value.
 */
PJSON_API jvalue_ref jnumber_create_i64(int64_t number);

/**
 * @brief Converts the string representation of the number into a DOM wrapper.
 *
 * Converts the string representation of the number into a DOM wrapper. This actually stores the converted value of raw.
 * If raw can be represented as a 64-bit integer then that is used otherwise a double is used
 *
 * NOTE: Behaviour is undefined if the raw value cannot be represented as a 64-bit integer
 *       or a double floating point number.
 *
 * @param raw string representation of a number
 * @return A reference to a JSON number representing the requested value.
 *
 * @see jnumber_create_i64
 * @see jnumber_create_f64
 */
PJSON_API jvalue_ref jnumber_create_converted(raw_buffer raw);

/**
 * @brief Compare this number against another JSON number.
 *
 * Compare this number against another JSON number.
 *
 * @param number Must be a JSON number.
 * 				 <br>NOTE: Behaviour is undefined if jis_number returns false
 *               <br>NOTE: Behaviour is undefined if this number cannot convert without error to a 64-bit
 *       			   integer or a double floating point number.
 * @param toCompare The number to compare against.
 * 				 <br>NOTE: Behaviour is undefined if jis_number returns false
 *               <br>NOTE: Behaviour is undefined if this number cannot convert without error to a 64-bit
 *       			   integer or a double floating point number.
 * @return Returns 1 if number is greater then toCompare, 0 if they are equal, -1 if number is less than toCompare.
 */
PJSON_API int jnumber_compare(jvalue_ref number, jvalue_ref toCompare) NON_NULL(1, 2);

/**
 * @brief Compare this number against an integer.
 *
 * Compare this number against an integer.
 *
 * @param number Must be a JSON number.
 * 				 <br>NOTE: Behaviour is undefined if jis_number returns false
 *               <br>NOTE: Behaviour is undefined if this number cannot convert without error to a 64-bit
 *       			   integer or a double floating point number.
 * @param toCompare The number to compare against.
 * @return Returns 1 if number is greater then toCompare, 0 if they are equal, -1 if number is less than toCompare.
 */
PJSON_API int jnumber_compare_i64(jvalue_ref number, int64_t toCompare) NON_NULL(1);

/**
 * @brief Compare this number against a floating point number.
 *
 * Compare this number against a floating point number.
 *
 * @param number Must be a JSON number.
 * 				 <br>NOTE: Behaviour is undefined if jis_number returns false
 *               <br>NOTE: Behaviour is undefined if this number cannot convert without error to a 64-bit
 *       			   integer or a double floating point number.
 * @param toCompare The number to compare against.
 * @return Returns 1 if number is greater then toCompare, 0 if they are equal, -1 if number is less than toCompare.
 */
PJSON_API int jnumber_compare_f64(jvalue_ref number, double toCompare) NON_NULL(1);

/**
 * @brief Determines whether or not the JSON value is an actual number type.
 *
 * Determines whether or not the JSON value is an actual number type.
 *
 * @code
 *jvalue_ref my_response = jobject_create_var(
		jkeyval(J_CSTR_TO_JVAL("int32_value"), jnumber_create_i32(-50)),
		jkeyval(J_CSTR_TO_JVAL("int64_value"), jnumber_create_i64(maxInt32 + 1)),
		jkeyval(J_CSTR_TO_JVAL("float_value"), jnumber_create_f64(0.1)),
		J_END_OBJ_DECL
	);
	jvalue_ref jnum = jobject_get(my_response, J_CSTR_TO_BUF("int64_value"));
	//prints is number: 1
	fprintf(stdout, "is number: %d", jis_number(jnum));
 * @endcode
 *
 * @param num The reference to the JSON value
 * @return True if num is some numeric type, false otherwise.
 */
PJSON_API bool jis_number(jvalue_ref num) NON_NULL(1);

/**
 * @brief Determines whether or not this is a reference to a JSON number that internally has some kind of error code
 * set.
 *
 * Determines whether or not this is a reference to a JSON number that internally has some kind of error code
 * set.
 *
 * @return True if there is some kind of error storing this number (i.e. not enough precision in the storage used, NaN or +/- infinity, etc).
 * Error details can be retrived by #jnumber_get_i32 or #jnumber_get_i64
 */
PJSON_API bool jnumber_has_error(jvalue_ref number) NON_NULL(1);

/**
 * @brief Retrieve the JSON number as a native numeric integer.
 *
 * Retrieve the JSON number as a native 32-bit signed integer. The goal is to get number to represent the number
 * closest to the num parameter type (which may be a raw string, a double, 32-bit integer, etc).
 *
 * The following return codes list the precise relationship if the double floating point cannot accurately represent
 * the value in the num parameter.  Multiple error codes may be set in the return code.
 *
 * CONV_OK --> the num parameter can be correctly represented using a 32-bit signed integer (number contains an accurate value)
 *
 * CONV_POSITIVE_OVERFLOW --> the num parameter contains a positive number outside what can be represented by a 32-bit signed integer.
 * The number parameter is set to the largest positive 32-bit signed integer
 *
 * CONV_NEGATIVE_OVERFLOW --> the num parameter contains a negative number outside what can be represented by a 32-bit signed integer.
 * The number parameter is set to the largest negative 32-bit signed integer
 *
 * CONV_OVERFLOW --> set if there is either a positive or negative overflow - binary OR of the two more specific return codes number
 * is clamped to the value closest to the actual number
 *
 * CONV_POSITIVE_INFINITY --> the num parameter contains a positive infinity.
 * The number parameter is set to the largest positive 32-bit signed integer
 *
 * CONV_NEGATIVE_INFINITY --> the num parameter contains a negative infinity.
 * The number parameter is set to the largest negative 32-bit signed integer
 *
 * CONV_INFINITY --> the num parameter contains a positive or negative infinity.
 * The number parameter is set to the largest negative 32-bit signed integer
 *
 * CONV_PRECISION_LOSS --> set if the underlying value is an integer value outside of the [-2∧53, 2∧53 range]
 * number is the closest value a double floating point can have to the integer value.
 *
 * CONV_NOT_A_NUM --> the num parameter doesn’t actually represent a number (or NaN somehow got into the floating point representation). The
 * number parameter is set to 0
 *
 * CONV_BAD_ARGS --> num is an invalid JSON number reference or number is NULL. The value of number is unspecified
 *
 * CONV_GENERIC_ERROR --> some other conversion error occurred (please contact the maintainer). The value of number is unspecified
 *
 * @param num The reference to the JSON number
 * @param number The pointer to where to write the value to
 * @return The conversion result of converting this JSON number value to a 32-bit signed integer.  CONV_OK (0) if
 *         there are no problems, otherwise there was a conversion error (binary OR of all the error bits).  In the error case,
 *         recommended to use the CONV_HAS or CONV_IS convenience macros.
 */
PJSON_API ConversionResultFlags jnumber_get_i32(jvalue_ref num, int32_t *number) NON_NULL(1, 2);

/**
 * @brief Retrieve the JSON number as a native numeric integer.
 *
 * Retrieve the JSON number as a native 64-bit signed integer. The goal is to get number to represent the number
 * closest to the num parameter type (which may be a raw string, a double, 32-bit integer, etc).
 *
 * The following return codes list the precise relationship if the double floating point cannot accurately represent
 * the value in the num parameter.  Multiple error codes may be set in the return code.
 *
 * CONV_OK --> the num parameter can be correctly represented using a 64-bit signed integer (number contains an accurate value)
 *
 * CONV_POSITIVE_OVERFLOW --> the num parameter contains a positive number outside what can be represented by a 64-bit signed integer.
 * The number parameter is set to the largest positive 64-bit signed integer
 *
 * CONV_NEGATIVE_OVERFLOW --> the num parameter contains a negative number outside what can be represented by a 64-bit signed integer.
 * The number parameter is set to the largest negative 64-bit signed integer
 *
 * CONV_OVERFLOW --> set if there is either a positive or negative overflow - binary OR of the two more specific return codes number
 * is clamped to the value closest to the actual number
 *
 * CONV_POSITIVE_INFINITY --> the num parameter contains a positive infinity.
 * The number parameter is set to the largest positive 64-bit signed integer
 *
 * CONV_NEGATIVE_INFINITY --> the num parameter contains a negative infinity.
 * The number parameter is set to the largest negative 64-bit signed integer
 *
 * CONV_INFINITY --> the num parameter contains a positive or negative infinity.
 * The number parameter is set to the largest negative 64-bit signed integer
 *
 * CONV_PRECISION_LOSS --> set if the underlying value is an integer value outside of the [-2∧53, 2∧53 range]
 * number is the closest value a double floating point can have to the integer value.
 *
 * CONV_NOT_A_NUM --> the num parameter doesn’t actually represent a number (or NaN somehow got into the floating point representation). The
 * number parameter is set to 0
 *
 * CONV_BAD_ARGS --> num is an invalid JSON number reference or number is NULL. The value of number is unspecified
 *
 * CONV_GENERIC_ERROR --> some other conversion error occurred (please contact the maintainer). The value of number is unspecified
 *
 * @param num The reference to the JSON number
 * @param number The pointer to where to write the value to
 * @return The conversion result of converting this JSON number value to a 64-bit signed integer.  CONV_OK (0) if
 *         there are no problems, otherwise there was a conversion error (binary OR of all the error bits).  In the error case,
 *         recommended to use the CONV_HAS or CONV_IS convenience macros.
 */
PJSON_API ConversionResultFlags jnumber_get_i64(jvalue_ref num, int64_t *number) NON_NULL(1, 2);

/**
 * @brief Retrieve the JSON number as a native floating point value.
 *
 * Retrieve the JSON number as a native floating point value. The goal is to get number to represent the number
 * closest to the num parameter type (which may be a raw string, a double, 32-bit integer, etc).
 *
 * The following return codes list the precise relationship if the double floating point cannot accurately represent
 * the value in the num parameter.  Multiple error codes may be set in the return code.
 *
 * CONV_OK --> the num parameter can be correctly represented using a 64-bit signed floating number (number contains an accurate value)
 *
 * CONV_POSITIVE_OVERFLOW --> the num parameter contains a positive number outside what can be represented by a 64-bit floating number.
 * The number parameter is set to the largest positive 64-bit floating number
 *
 * CONV_NEGATIVE_OVERFLOW --> the num parameter contains a negative number outside what can be represented by a 64-bit floating number.
 * The number parameter is set to the largest negative 64-bit floating number
 *
 * CONV_OVERFLOW --> set if there is either a positive or negative overflow - binary OR of the two more specific return codes number
 * is clamped to the value closest to the actual number
 *
 * CONV_POSITIVE_INFINITY --> the num parameter contains a positive infinity.
 * The number parameter is set to the largest positive 64-bit floating number
 *
 * CONV_NEGATIVE_INFINITY --> the num parameter contains a negative infinity.
 * The number parameter is set to the largest negative 64-bit floating number
 *
 * CONV_INFINITY --> the num parameter contains a positive or negative infinity.
 * The number parameter is set to the largest negative 64-bit floating number
 *
 * CONV_PRECISION_LOSS --> set if the underlying value is an integer value outside of the [-2∧53, 2∧53 range]
 * number is the closest value a double floating point can have to the integer value.
 *
 * CONV_NOT_A_NUM --> the num parameter doesn’t actually represent a number (or NaN somehow got into the floating point representation). The
 * number parameter is set to 0
 *
 * CONV_BAD_ARGS --> num is an invalid JSON number reference or number is NULL. The value of number is unspecified
 *
 * CONV_GENERIC_ERROR --> some other conversion error occurred (please contact the maintainer). The value of number is unspecified
 *
 * @param num The reference to the JSON number
 * @param number The pointer to where to write the value to
 * @return The conversion result of converting this JSON number value to a 64-bit floating number.  CONV_OK (0) if
 *         there are no problems, otherwise there was a conversion error.  In the error case,
 *         recommended to use the CONV_HAS or CONV_IS convenience macros.
 */
PJSON_API ConversionResultFlags jnumber_get_f64(jvalue_ref num, double *number) NON_NULL(1, 2);

/**
 * @brief Retrieve the raw string representation of the number.
 *
 * Retrieve the raw string representation of the number.
 *
 * NOTE: If jvalue is not a number or does not have string representation, the function fails.
 * If string representation of a number is needed, string generator(json serializer) should be used.
 *
 * @param num The reference to the JSON value.
 * @param result The pointer to the structure to fill in
 *                 NOTE: The lifetime of result->m_str is at most the lifetime of num
 *                 NOTE: This might not be a null-terminated string - use the length field as well
 *                 NOTE: It is unspecified what value this takes if there is no backing raw buffer (i.e.
 *                       the JSON number is backed using a native numeric type).  If the pointer returned is not null, then
 *                       memory was allocated for a conversion - in any case, to be safe, make sure to always free the resultant string
 *                       (malloc or calloc will have been used to create the buffer).
 * @return CONV_OK or CONV_NOT_A_RAW_NUM.
 */
PJSON_API ConversionResultFlags jnumber_get_raw(jvalue_ref num, raw_buffer *result);

/*** JSON Boolean operations ***/

/**
 * @brief Checks, if jvalue_ref points to a boolean node of JSON DOM tree
 *
 * Checks, if jvalue_ref points to a boolean node of JSON DOM tree
 *
 * @param jval A reference to a JSON value
 * @return true, if jvalue_ref is a boolean node
 */
PJSON_API bool jis_boolean(jvalue_ref jval) NON_NULL(1);

/**
 * Create a JSON boolean with the true value
 *
 * @return The reference to the boolean.
 */
PJSON_API jvalue_ref jboolean_true() PURE_FUNC;

/**
 * Create a JSON boolean with the false value
 *
 * @return The reference to the boolean.
 */
PJSON_API jvalue_ref jboolean_false() PURE_FUNC;

/**
 * Create a JSON boolean with the requested value
 *
 * @param value The value of the boolean
 * @return The reference to the boolean.
 */
PJSON_API jvalue_ref jboolean_create(bool value) PURE_FUNC;

/**
 * @brief Retrieve the native boolean representation of this reference.
 *
 * Retrieve the native boolean representation of this reference.
 *
 * The types below are converted to Boolean using the following conversion logic:
 *  - Numbers: 0, Nan will be treated as False, all other values will be treated as True
 *  - Strings: Empty strings will be treated as False, all other values will be treated as True
 *  - Null: Null values will be treated as False
 *  - Array: Arrays will be treated as True
 *  - OBJECT: OBJECTs will be treated as True
 *
 * @param val The reference to the JSON value
 * @param value Reference to a boolean where the converted value will be stored. NOTE: It is safe for this to be a NULL
 * @return CONV_OK if val represents a JSON boolean type, otherwise CONV_NOT_A_BOOLEAN.
 */
PJSON_API ConversionResultFlags jboolean_get(jvalue_ref val, bool *value) NON_NULL(1);

/**
 * @brief Convenience method to construct a jobject_key_value structure.
 *
 * Convenience method to construct a jobject_key_value structure.
 *
 * NOTE: Undefined behaviour if the key is NULL or not a string.
 *
 * NOTE TO IMPLEMENTOR: THIS MUST HAVE HIDDEN VISIBILITY TO INSURE THERE ARE NO CONFLICTS
 *     IN THE SYMBOL TABLE. IN THEORY, static inline SHOULD SUFFICE.
 *
 * @param key json string with a key
 * @param value json object/array/value
 * @return new json object with key/value pair
 */
static inline jobject_key_value jkeyval(jvalue_ref key, jvalue_ref value)
{
	assert(key != NULL);
	assert(jis_string(key));
	return (jobject_key_value){ (key), (value) };
}

/**
 * @brief Convenience inline function that casts the C-string constant/literal to a raw_buffer
 *
 * Convenience inline function that casts the C-string constant/literal to a raw_buffer
 * structure that is used for JSON strings.
 *
 * @param cstring A C string, that ends with 0
 * @return A raw_buffer struct containing { cstring, strlen(cstring) }
 */
static inline raw_buffer j_cstr_to_buffer(const char *cstring)
{
	return ((raw_buffer) { cstring, strlen(cstring) } );
}

/**
 * @brief Convenience inline function that creates a JSON string from a C-string constant/literal.
 *
 * Convenience inline function that creates a JSON string from a C-string constant/literal.
 *
 * @param cstring Must be a valid C-string with a lifetime longer than the resultant JSON value will have.
 *                 Safest to use with string literals or string constants (e.g. constant for the life of the program).
 */
static inline jvalue_ref j_cstr_to_jval(const char *cstring)
{
	return jstring_create_nocopy( j_cstr_to_buffer(cstring) );
}

/**
 * @brief Convenience macro to convert an arbitrary string to a JSON string. It can be used when you know length of the string
 *
 * Convenience macro to convert an arbitrary string to a JSON string. It can be used when you know length of the string
 *
 * @param string pointer to a string
 * @param length length of the string
  */
static inline raw_buffer j_str_to_buffer(const char *string, size_t length)
{
	return ((raw_buffer){ (string), (length) });
}

/**
 * @brief Creat a buffer from a C-string literal or char array.
 *
 * Optimized version for creating a buffer from a C-string literal or char array.
 *
 * @param string A constant string literal or char array (for which the compiler knows the size)
 *               <br>NOTE: Do not use this with variables unless you understand the lifetime requirements for the string.
 *               <br>NOTE: Assumes that it is equivalent to a NULL-terminated UTF-8-compatible string.
 * @return A raw_buffer structure
 */
#define J_CSTR_TO_BUF(string) j_str_to_buffer(string, sizeof(string) - 1)

/**
 * @brief  Converts a C-string literal to a jvalue_ref.
 *
 * Converts a C-string literal to a jvalue_ref.
 *
 * @param string Refer to J_CSTR_TO_BUF - same requirements
 * @return A JSON string reference
 *
 * @see J_CSTR_TO_BUF(string)
 * @see pj_cstr_to_jval
 */
#define J_CSTR_TO_JVAL(string) jstring_create_nocopy( j_str_to_buffer(string, sizeof(string) - 1) )

#ifdef __cplusplus
}
#endif

#endif /* JOBJECT_H_ */
