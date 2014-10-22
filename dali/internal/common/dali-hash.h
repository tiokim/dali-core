#ifndef __DALI_HASH__
#define __DALI_HASH__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string>

namespace Dali
{


/**
 * @brief Create a hash code for the string
 * @param toHash string to hash
 * @return hash code
 */
unsigned long CalculateHash(const std::string& toHash);


/**
 * @brief Create a hash code for 2 strings combined
 * @param string1 first string
 * @param string2 second string
 * @return hash code
 */
unsigned long CalculateHash(const std::string& string1, const std::string& string2);




} // namespace Dali

#endif // header
