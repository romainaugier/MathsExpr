// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include <cmath>

static constexpr double EPSILON = 0.00001;

#define DOUBLE_EQ(a, b) ((::fabs(a) - ::fabs(b)) < EPSILON)