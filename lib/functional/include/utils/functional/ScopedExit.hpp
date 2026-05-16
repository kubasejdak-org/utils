/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2021 Kuba Sejdak (kuba.sejdak@gmail.com)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "utils/preprocessor/symbols.hpp"

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

    /// Move constructor.
    /// @note This constructor is deleted, because ScopedExit is not meant to be move-constructed.
    ScopedExit(ScopedExit&&) noexcept = delete;

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

#define ON_EXIT(callback) utils::functional::ScopedExit UNIQUE_NAME(onExit)(callback)
