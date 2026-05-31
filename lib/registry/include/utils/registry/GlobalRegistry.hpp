/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2020 Kuba Sejdak (kuba.sejdak@gmail.com)
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

#include <cassert>
#include <concepts>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace utils::registry {
namespace detail {

/// Helper class that wrapping object and the id describing it into one entity.
/// @tparam IdType              Type representing id of the object.
/// @tparam ObjectType          Type representing the object.
/// @note This type constructs std::shared_ptr by moving given object to the smart pointer.
/// @note This helper class is necessary, because there is no STL container that is only move-constructible.
template <typename IdType, typename ObjectType>
class Instance {
public:
    /// Constructor. Creates std::shared_ptr out of the given object.
    /// @tparam T                Type of the object to be held within this Instance.
    /// @param id               Id of the object.
    /// @param object           Object to be held.
    template <std::move_constructible T>
    Instance(IdType&& id, T&& object)
        : m_id(std::forward<IdType>(id))
        , m_object(std::make_shared<T>(std::forward<T>(object)))
    {}

    /// Returns id of the underlying object.
    /// @return Id of the underlying object.
    [[nodiscard]] IdType id() const { return m_id; }

    /// Returns std::shared_ptr with the underlying object.
    /// @return std::shared_ptr with the underlying object.
    [[nodiscard]] std::shared_ptr<ObjectType> object() const { return m_object; }

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
        append(std::move(instances));
    }

    /// Appends given set of id-object pairs into GlobalRegistry (wrapper in detail::Instance).
    /// @param instances        T objects, that should be stored within GlobalRegistry.
    /// @note This method can be called multiple times for every type ObjectType-IdType pair.
    static void append(std::vector<detail::Instance<IdType, ObjectType>>&& instances)
    {
        for (auto& instance : instances)
            m_instances.try_emplace(instance.id(), instance.object());
    }

    /// Returns std::shared_ptr with instance of the T type, that is identified with the given id.
    /// @param id               Id of the instance, that should be returned.
    /// @return std::shared_ptr with instance of the T type, that is identified with the given id.
    static std::shared_ptr<ObjectType> get(const IdType& id)
    {
        auto it = m_instances.find(id);
        if (it == m_instances.end())
            return nullptr;

        return it->second;
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

} // namespace utils::registry
