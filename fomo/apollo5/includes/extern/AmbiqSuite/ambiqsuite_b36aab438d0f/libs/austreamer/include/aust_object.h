//*****************************************************************************
//
// Copyright (c) 2022, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#ifndef __AUST_OBJECT_H__
#define __AUST_OBJECT_H__

#include <map>
#include <string>

#include "aust_type.h"

using namespace std;

/// @brief A container of string to string map.
typedef std::map<std::string, std::string> Properties;

/// @brief AustObject has properties<key, value>, which is for instance to store
/// the attributes.
class AustObject
{
  private:
    /// Properties is a map<key, value>.
    Properties properties_;

  public:
    AustObject();
    ~AustObject();

    /// @brief Set one property to container.
    /// @param key Key word.
    /// @param value Description of the key word.
    /// @return true if success, otherwise false.
    bool setProperty(std::string key, std::string value);

    /// @brief Get the specific property value from the container.
    /// @param key Key word.
    /// @return The value of the key word.
    std::string getPropertyValue(std::string key);

    /// @brief Get all of the properties from the container.
    /// @return Properties reference.
    Properties *getProperties(void);
};

#endif /* __AUST_OBJECT_H__ */
