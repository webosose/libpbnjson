// Copyright (c) 2009-2018 LG Electronics, Inc.
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

#ifndef PJSONCXX_H_
#define PJSONCXX_H_

/**
 * @defgroup API_SUMARY API_SUMARY
 * @{
 * This API allows an easier C++ abstraction over the core PBNJSON library that is written in C. PBNJSON (Palm's Better Native JSON) library is a JSON engine that offers a number of improvements over other JSON libraries: a native C or C++ interface, fixes for errors commonly found in other libraries (especially around number handling), better parsing and object manipulation performance, and basic JSON schema support
 * @}
 */

/*!
 @mainpage PBNJSON C++
 @section PBNJSONCPP_INTRO C++ API Introduction

PbnJson is short for “Palm's Better Native JSON library”. This library allows for encoding objects to JSON and decoding (parsing) text encoded in JSON.
JSON is short for Javascript object notation, it is a lightweight format based on Javascript which can be used to encode text. It is widely used and easy to read and write.

DOM : short for “document object model”, this represents the JSON text in a structured manner. The nodes of the JSON text are organized in a tree structure called the DOM tree and commonly referred to as simply “dom” in the rest of this documentation. This approach can be useful if the engineer wants to access the entire JSON text in a hierarchical manner.

SAX : short for “simple API for xml”, This is a event driven method for parsing JSON text. Events are fired while parsing the JSON text and the engineer can take action on these events (by setting callbacks). DOM parsers operate on the document as a whole whereas SAX parsers operate on each piece of the XML document sequentially.
This approach can be useful if the engineer wants to act upon specific parts of the JSON text for example on all numbers present in the text.

The PbnJson API provides the following features:
 - DOM parser: Given a JSON string, it creates a JSON DOM object to work with
SAX parser: Given a JSON string, it generates proper callbacks on detected JSON objects
 - DOM/SAX parser from stream: The parser processes incoming string chunk-by-chunk
JSON string generation: Generation of JSON string from DOM object
Schema validation: Validation of JSON DOM object against JSON schema. The JSON schema defines the structure of the JSON document for example: which fields are required/optional, what are the types of objects, etc.

The advantages of this API over the other APIs used in the past:
	- Numbers are parsed correctly regardless of how they are sent along the wire (supports generic number format as defined by the JSON spec).
    - Much faster than other libraries
       - Uses a faster (& correct) parser
       - Much fewer DOM nodes created
       - Number parsing is delayed until a conversion request is made.
    - First implementation of schemas in C (more of the spec implemented than any other implementation posted on the internet).
       - Schemas are integral to using pbnsjon.
       - Schemas define what input is accepted/reject when parsing
           - No more unnecessary checks of valid parameter passing; simply write the schema
       - Schemas can be created from files & use mmap for minimal memory overhead & optimal behaviour (no swapping necessary)
    - First class C++ bindings.  C++ bindings are maintained as part of this library and are treated
      as important as the core C library.

 NOTE:
The DOM parser creates a tree of JSON objects/array in memory that represents the JSON text. Memory needs to be allocated
for creating and storing this entire JSON tree in memory. The memory required can be substantial for large JSON text. The created tree
also referred to as the dom can be used for further processing like queries, modification, serialization, validation of JSON data.
In contrast to the DOM parser the SAX parser follows a event driven model. It allows the user to set callback functions which
will be invoked when specific JSON types and conditions are encountered.  While parsing the JSON text the parser calls the user’s callbacks which can modify or collect information from the JSON text. This approach does not require as much memory as the DOM parser.


 @subsection PBNJSONCPP_GEN_OVERVIEW Generating JSON & serializing to a string
 This is an example of how to create a JSON value and then convert it to a string.

 @subsubsection PBNJSON_CPP_GEN_OVERVIEW_SNIPPET Code snippet:
 @snippet TestExample++.cpp generate

 @section PBNJSONCPP_OVERVIEW C++ API Overview
 @subsection PBNJSONCPP_PARSE_OVERVIEW Parsing JSON serialized within a string
 This is an example of how to take a JSON value serialized into a string, parse it into an in-memory format,
 and interact with it.

 @subsubsection PBNJSONCPP_PARSE_OVERVIEW_SCHEMA The schema used in the code snippet below
 @include input.schema

 @subsubsection PBNJSONCPP_PARSE_OVERVIEW_SNIPPET Code snippet:
 @snippet TestExample++.cpp parse schema

@subsubsection PBNJSONCPP_PARSE_OVERVIEW_SNIPPET2 Once more with a default schema:
@snippet TestExample++.cpp parse all

@section PBNJSONCPP_STREAM_PARSERS Stream parsers
The library is able to parse input JSON data from stream e.g chuck by chunk
It could be used when there is no possibility to load entire JSON into memory.
The following examples will show how to use it.

@subsection PBNJSONCPP_STREAM_PARSERS_DOM Example of usage of stream DOM parser:
@snippet TestExample++.cpp parse stream dom

 */

#include "pbnjson/cxx/japi.h"
#include "pbnjson/cxx/JValue.h"
#include "pbnjson/cxx/JGenerator.h"
#include "pbnjson/cxx/JSchema.h"
#include "pbnjson/cxx/JDomParser.h"
#include "pbnjson/cxx/JSchemaFile.h"
#include "pbnjson/cxx/JSchemaFragment.h"
#include "pbnjson/cxx/JResolver.h"
#include "pbnjson/cxx/JErrorHandler.h"
#include "pbnjson/cxx/JResult.h"
#include "pbnjson/cxx/JValidator.h"
#include "pbnjson/cxx/JQuery.h"

#endif /* PJSONCXX_H_ */
