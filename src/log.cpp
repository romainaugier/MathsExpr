// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/log.hpp"

MATHSEXPR_NAMESPACE_BEGIN

Logger& Logger::get_instance() { static Logger l; return l; }

MATHSEXPR_NAMESPACE_END