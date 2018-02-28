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

#ifndef JRESOLVER_H_
#define JRESOLVER_H_

#include "JSchema.h"

namespace pbnjson {

/**
 * Class provides an interface for external link resolver.
 */
class PJSONCXX_API JResolver {
public:
	class ResolutionRequest {
	public:
		/**
		 * Create new resolution request
		 *
		 * @param schema Schema, which contains link
		 * @param resource Name, which needs the resolution
		 */
		ResolutionRequest(const JSchema &schema, const std::string &resource);

		/// returns the schema that generated the resolution request
		JSchema schema() const;
		/// returns the resource name that needs resolution
		std::string resource() const;
	private:
		/// the schema that generated the resolution request
		JSchema m_ctxt;
		/// the resource name that needs to be resolved with the current request
		std::string m_resource;
	};

	//TODO Nikolay Orliuk: consider possibility to provide JSchemaResolverRef as
	//                     an implicit cast from JResolver

	JResolver();
	virtual ~JResolver();

	/**
	 * An external reference was made within a schema - the user
	 * of the library is responsible for converting this abstract concept
	 * of a reference to an actual schema.
	 */
	//TODO add simple default resolve method
	virtual JSchema resolve(const ResolutionRequest &request, JSchemaResolutionResult &resolutionResult) = 0;
};

}

#endif /* JRESOLVER_H_ */
