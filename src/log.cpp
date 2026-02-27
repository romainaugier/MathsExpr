// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/log.hpp"

MATHEXPR_NAMESPACE_BEGIN

Logger& Logger::get_instance() { static Logger l; return l; }

MATHEXPR_NAMESPACE_END