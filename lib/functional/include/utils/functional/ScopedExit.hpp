/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utils/preprocessor/symbols.hpp>

#include <functional>
#include <utility>

namespace utils::functional {

/// Function prototype that can be registered in ScopedExit callback. This method will be called when ScopedExit
/// is actually destroyed.
using ScopedExitCallback = std::function<void()>;

/// Helper class that allows registering custom callback function to be called when given scope is finished.
/// Scope end is determined by the lifetime of the ScopedExit object.
class ScopedExit {
public:
    /// Constructor.
    /// @param callback         Callback to be executed one this object is destroyed.
    explicit ScopedExit(ScopedExitCallback callback)
        : m_callback(std::move(callback))
    {}

    /// Copy constructor.
    /// @note This constructor is deleted, because ScopedExit is not meant to be copy-constructed.
    ScopedExit(const ScopedExit&) = delete;

    /// Copy constructor.
    /// @note This constructor is deleted, because ScopedExit is not meant to be move-constructed.
    ScopedExit(ScopedExit&&) = delete;

    /// Destructor. Executes user-defined callback.
    ~ScopedExit()
    {
        if (m_callback)
            m_callback();
    }

    /// Copy assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because ScopedExit is not meant to be copy-assigned.
    ScopedExit& operator=(const ScopedExit&) = delete;

    /// Move assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because ScopedExit is not meant to be move-assigned.
    ScopedExit& operator=(ScopedExit&&) = delete;

private:
    ScopedExitCallback m_callback;
};

} // namespace utils::functional

#define ON_EXIT(callback) utils::functional::ScopedExit UNIQUE_NAME(onExit)(callback) // NOLINT
