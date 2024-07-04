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

#ifndef JPARSE_STREAM_H_
#define JPARSE_STREAM_H_

#include <stdbool.h>
#include "japi.h"
#include "jschema.h"
#include "jobject.h"
#include "jcallbacks.h"
#include "jparse_types.h"
#include "jerror.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file jparse_stream.h
 * The file jparse_stream.h contains API to parse JSON string from stream. APIs is able to process incoming data chunk-by-chunk, saving state between
 * calls of parse function. There are two types of parser - stream SAX parser and stream DOM parser. The main logic line:
 *  - create a parser
 *  - pass incoming data chunk-by-chunk
 *  - retrive parsing results
 *  - release parser
 *
 * Such parsers can be used, when whole JSON data could not be acquired or loaded.
 */

/**
 * @brief Returns the DOM structure of the JSON document contained within the given file.
 *
 * Returns the DOM structure of the JSON document contained within the given file.
 *
 * @param file The c-string representing the path to parse.
 * @param schema The schema to use for validation of the input.
 * @param err Error pointer. Will be set to non-null value in case of failure.
 * @return An opaque reference handle to the DOM.  Use jis_valid to determine whether or
 *         not parsing succeeded.
 */
PJSON_API jvalue_ref jdom_fcreate(const char *file, const jschema_ref schema, jerror **err) NON_NULL(1, 2);

/**
 * @brief Returns the DOM structure of the JSON document contained within the given file.
 *
 * Returns the DOM structure of the JSON document contained within the given file.
 *
 * @param file The c-string representing the path to parse.
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param opts The optimization mode to use when parsing the file.
 * @return An opaque reference handle to the DOM.  Use jis_valid to determine whether or
 *         not parsing succeeded.
 * @deprecated Use jdom_fcreate instead
 */
PJSON_API jvalue_ref jdom_parse_file(const char *file, JSchemaInfoRef schemaInfo, JFileOptimizationFlags flags) NON_NULL(1, 2);

/**
 * @brief Returns the DOM structure of the JSON document.
 *
 * Returns the DOM structure of the JSON document.
 *
 * @param input The input string to parse.
 *              NOTE: It is unspecified if a DOM will be constructed if the input does not contain a top-level
 *              object or array.
 *              NOTE: Need not be a null-terminated string
 * @param schema The schema to use for validation of the input.
 * @param err Error pointer. Will be set to non-null value in case of failure.
 * @return An opaque reference handle to the DOM.  Use jis_valid to determine whether or
 *         not parsing succeeded.
 */
PJSON_API jvalue_ref jdom_create(raw_buffer input, const jschema_ref schema, jerror **err) NON_NULL(2);

/**
 * @brief Returns the DOM structure of the JSON document.
 *
 * Returns the DOM structure of the JSON document.
 *
 * @param input The input string to parse.
 *              NOTE: It is unspecified if a DOM will be constructed if the input does not contain a top-level
 *              object or array.
 *              NOTE: Need not be a null-terminated string
 * @param optimizationMode Additional information about the input string that lets us optimize the creation process of the DOM.
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @return An opaque reference handle to the DOM.  Use jis_valid to determine whether or
 *         not parsing succeeded.
 * @deprecated Use jdom_create instead
 */
PJSON_API jvalue_ref jdom_parse(raw_buffer input, JDOMOptimizationFlags optimizationMode, JSchemaInfoRef schemaInfo) NON_NULL(3);

/**
 * @brief Parse the input using SAX callbacks.  Much faster in that no memory is allocated for a DOM & data is processed on the fly
 *
 * Parse the input using SAX callbacks.  Much faster in that no memory is allocated for a DOM & data is processed on the fly
 *
 * @param input The input string to parse
 *              NOTE: It is unspecified if the error handlers will be called if the input does not contain a top-level
 *              object or array.
 * @param schema The schema to use for validation of the input.
 * @param callbacks A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *               parsing events.
 * @param context The ctxt parameter during parsing.
 *                After a successful parse, set to whatever the ctxt parameter was changed to during the parse.
 * @param err pbnjson error information
 * @return True if parsing succeeded with no unrecoverable errors, false otherwise.
 *
 * @see jsax_getContext
 * @see jsax_changeContextd
 */
PJSON_API bool jsax_parse_with_callbacks(raw_buffer input, const jschema_ref schema,
                                         PJSAXCallbacks *callbacks, void *callback_ctxt,
                                         jerror **err) NON_NULL(2);

/**
 * @brief Parse the input using SAX callbacks.  Much faster in that no memory is allocated for a DOM & data is
 * processed on the fly, but less flexible & more complicated to handle in some cases.
 *
 * Parse the input using SAX callbacks.  Much faster in that no memory is allocated for a DOM & data is
 * processed on the fly, but less flexible & more complicated to handle in some cases.
 *
 * @param parser A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *               parsing events.
 * @param input The input string to parse
 *              NOTE: It is unspecified if the error handlers will be called if the input does not contain a top-level
 *              object or array.
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *              error handler).
 * @param data The ctxt parameter during parsing.
 *             After a successful parse, set to whatever the ctxt parameter was changed to during the parse.
 * @return True if parsing succeeded with no unrecoverable errors, false otherwise.
 *
 * @see jsax_getContext
 * @see jsax_changeContext
 * @deprecated Use jsax_parse_with_callbacks instead
 */
PJSON_API bool jsax_parse_ex(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schemaInfo, void **ctxt) NON_NULL(3);

/**
 * @brief Convenience method for the more extended version.  No error message generated.
 *
 * Convenience method for the more extended version.  No error message generated.
 *
 * @deprecated Use jsax_parse_with_callbacks instead or jsaxparser_new + jsaxparser_feed + jsaxparser_end + jsaxparser_release
 * @see jsax_parse_ex
 */
PJSON_API bool jsax_parse(PJSAXCallbacks *parser, raw_buffer input, JSchemaInfoRef schema) NON_NULL(3);

/**
 * @brief Changes user defined context
 *
 * Changes user defined context
 *
 * @see jparse_stream.c for an example of how the library uses it to implement the dom_parse functionality.
 */
PJSON_API void jsax_changeContext(JSAXContextRef saxCtxt, void *userCtxt);

/**
 * @brief Returns user defined context
 *
 * Returns user defined context
 *
 * @see jparse_stream.c for an example of how the library uses it to implement the dom_parse functionality.
 */
PJSON_API void* jsax_getContext(JSAXContextRef saxCtxt);

/**
 * @brief Create and initialize SAX stream parser
 *
 * Create and initialize SAX stream parser
 *
 * @param schema The schema to use for validation of the input.
 * @param callbacks A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *                 parsing events.
 * @param context Context that will be returned in callbacks
 * @return pointer to SAX parser
 */
jsaxparser_ref jsaxparser_new(const jschema_ref schema, PJSAXCallbacks *callbacks, void *context);

/**
 * @brief Create and initialize SAX stream parser
 *
 * Create and initialize SAX stream parser
 *
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param callback A pointer to a SAXCallbacks structure with pointers to functions that handle the appropriate
 *                 parsing events.
 * @param callback_ctxt Context that will be returned in callbacks
 * @return pointer to SAX parser
 * @deprecated Use jsaxparser_new instead
 */
jsaxparser_ref jsaxparser_create(JSchemaInfoRef schemaInfo, PJSAXCallbacks *callback, void *callback_ctxt);

/**
 * @brief Parse part of JSON from input buffer
 *
 * Parse part of JSON from input buffer
 *
 * @param parser Pointer to SAX parser
 * @param buf Input buffer
 * @param buf_len Input buffer length
 * @return false on error
 */
PJSON_API bool jsaxparser_feed(jsaxparser_ref parser, const char *buf, int buf_len);

/**
 * @brief Finalize stream parsing
 *
 * Finalize stream parsing
 *
 * @param parser Pointer to SAX parser
 * @return false on error
 */
PJSON_API bool jsaxparser_end(jsaxparser_ref parser);

/**
 * @brief Release SAX parser created by jsaxparser_create
 *
 * Release SAX parser created by jsaxparser_create
 *
 * @param parser Pointer to SAX parser
  */
PJSON_API void jsaxparser_release(jsaxparser_ref *parser);

/**
 * @brief Return error description. It can be called when jsaxparser_feed/jsaxparser_end has returned false
 *
 * Return error description. It can be called when jsaxparser_feed/jsaxparser_end has returned false
 *
 * @param parser Pointer to SAX parser
 * @return Pointer to string with error description. The pointer should not be released manually. It will be released in jsaxparser_release.
 */
PJSON_API const char* jsaxparser_get_error(jsaxparser_ref parser);

/**
 * @brief Create and initialize DOM stream parser
 *
 * Create and initialize DOM stream parser
 *
 * @param schema The schema to use for validation of the input.
 * @return Pointer to DOM parser
 */
jdomparser_ref jdomparser_new(const jschema_ref schema);

/**
 * @brief Create and initialize DOM stream parser
 *
 * Create and initialize DOM stream parser
 *
 * @param schemaInfo The schema to use for validation of the input, along with any other callbacks necessary (such as schema resolver,
 *                   error handler).
 * @param optimizationMode Optimization flags
 * @return Pointer to DOM parser
 * @deprecated Use jdomparser_new instead
 */
jdomparser_ref jdomparser_create(JSchemaInfoRef schemaInfo, JDOMOptimizationFlags optimizationMode);

/**
 * @brief Parse part of JSON from input buffer
 *
 * Parse part of JSON from input buffer
 *
 * @param parser Pointer to DOM parser
 * @param buf Input buffer
 * @param buf_len Input buffer length
 * @return false on error
 */
PJSON_API bool jdomparser_feed(jdomparser_ref parser, const char *buf, int buf_len);

/**
 * @brief Finalize stream parsing
 *
 * Finalize stream parsing
 *
 * @param parser Pointer to DOM parser
 * @return false on error
 */
PJSON_API bool jdomparser_end(jdomparser_ref parser);

/**
 * @brief Release DOM parser created by jdomparser_create
 *
 * Release DOM parser created by jdomparser_create
 *
 * @param parser Pointer to DOM parser
  */
PJSON_API void jdomparser_release(jdomparser_ref *parser);

/**
 * @brief Return error description. It can be called when jdomparser_feed/jdomparser_feed has returned false
 *
 * Return error description. It can be called when jdomparser_feed/jdomparser_feed has returned false
 *
 * @param parser Pointer to DOM parser
 * @return Pointer to string with error description. The pointer should not be released manually. It will be released in jdomparser_deinit
 */
PJSON_API const char* jdomparser_get_error(jdomparser_ref parser);

/**
 * @brief Return root jvalue for parsed JSON
 *
 * Return root jvalue for parsed JSON
 *
 * @param parser Pointer to DOM parser
 * @return Root jvalue for parsed JSON or jinvalid if any error occured
 */
PJSON_API jvalue_ref jdomparser_get_result(jdomparser_ref parser);

#ifdef __cplusplus
}
#endif

#endif /* JPARSE_STREAM_H_ */
