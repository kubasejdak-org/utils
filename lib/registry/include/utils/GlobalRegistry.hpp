/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2020, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace utils {
namespace detail {

/// Helper class that wrapping object and the id describing it into one entity.
/// @tparam IdType              Type representing id of the object.
/// @tparam ObjectType          Type representing the object.
/// @note This type constructs std::shared_ptr by moving given object to the smart pointer.
/// @note This helper class is necessary, because there is no STL container that is only move-constructible.
template <typename IdType, typename ObjectType>
class Instance {
    static_assert(std::is_move_constructible_v<ObjectType>);

public:
    /// Constructor. Creates std::shared_ptr out of the given object.
    /// @param id               Id of the object.
    /// @param object           Object to be held.
    Instance(IdType&& id, ObjectType&& object)
        : m_id(std::forward<IdType>(id))
        , m_object(std::make_shared<ObjectType>(std::forward<ObjectType>(object)))
    {}

    /// Returns id of the underlying object.
    /// @return Id of the underlying object.
    [[nodiscard]] IdType id() const { return m_id; }

    /// Returns std::shared_ptr with the underlying object.
    /// @return std::shared_ptr with the underlying object.
    std::shared_ptr<ObjectType> object() const { return m_object; }

private:
    IdType m_id{};
    std::shared_ptr<ObjectType> m_object;
};

} // namespace detail

/// Provides an easy to use way of registering set of global objects of the same type ObjectType and accessible via
/// IdType from everywhere.
/// @tparam ObjectType          Type representing the object.
/// @tparam IdType              Type representing id of the object.
/// @note In order to create a GlobalRegistry, use the following code:
///
/// 1) For default IdType = std::string:
///     using MyTestRegistry = GlobalRegistry<MyTest>;
///     MyTestRegistry::init({{"id1", MyTest(1, 1, 1)}, {"id2", MyTest(2, 2, 2)}});
///
/// 2) For custom IdType:
///     using MyTestRegistry2 = GlobalRegistry<MyTest, int>;
///     MyTestRegistry2::init({{1, MyTest(1, 1, 1)}, {2, MyTest(2, 2, 2)}});
template <typename ObjectType, typename IdType = std::string>
class GlobalRegistry {
public:
    /// Initializes GlobalRegistry with a given set of id-object pairs (wrapper in detail::Instance).
    /// @param instances        T objects, that should be stored within GlobalRegistry.
    /// @note This method can be called only once for every type ObjectType-IdType pair.
    static void init(std::vector<detail::Instance<IdType, ObjectType>>&& instances)
    {
        assert(m_instances.empty());

        for (auto& instance : instances)
            m_instances.try_emplace(instance.id(), instance.object());
    }

    /// Returns std::shared_ptr with instance of the T type, that is identified with the given id.
    /// @param id               Id of the instance, that should be returned.
    /// @return std::shared_ptr with instance of the T type, that is identified with the given id.
    static std::shared_ptr<ObjectType> get(const IdType& id)
    {
        if (m_instances.count(id) == 0)
            return nullptr;

        return m_instances[id];
    }

    /// Returns size of the GlobalRegistry.
    /// @return Size of the GlobalRegistry.
    static std::size_t size() { return m_instances.size(); }

    /// Clears global registry.
    /// @note After call to this function GlobalRegistry can be initialized once again.
    static void clear() { m_instances.clear(); }

private:
    /// Default constructor.
    GlobalRegistry() = default;

private:
    static inline std::map<IdType, std::shared_ptr<ObjectType>> m_instances;
};

} // namespace utils
