#pragma once

// C++
#include <algorithm>
#include <assert.h> // Should be temporal and include our own assertion with logs.
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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/matrix_decompose.hpp>
//#include <glm/gtx/quaternion.hpp>

// application
#include "defines.h"
#include "logger.h"
