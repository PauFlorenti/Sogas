#pragma once

// C++
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

// external
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#pragma warning (disable : 4201 )
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
//#include <glm/gtx/quaternion.hpp>
#pragma warning (default : 4201 )

// application
#include "defines.h"
#include "logger.h"
#include "read_json.h"
#include "math_utils.h"
