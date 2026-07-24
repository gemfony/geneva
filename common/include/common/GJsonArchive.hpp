/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// Boost headers go here
#include <boost/json.hpp>

// Geneva headers go here
#include "common/GArchive.hpp"

namespace Gem::Common::archive {

/******************************************************************************/
/**
 * @file GJsonArchive.hpp
 * @brief The human-readable JSON codec of the @c GArchive family (the
 * inspectable checkpoint format), built on @c boost::json (already a
 * dependency).
 *
 * @par Format
 * A serializable class becomes a JSON object keyed by member name; a container
 * becomes a JSON array; @c pair and @c map entries become @c {"first","second"}
 * / @c {"key","value"} objects. @c float / @c double are stored as JSON numbers
 * (boost::json's shortest-round-trip serializer reproduces them exactly on
 * re-parse). @c long @c double, which a JSON number cannot hold, is stored as an
 * exact hexadecimal-float string (@c "%La") so it round-trips bit-exactly.
 *
 * @par Tree assembly (save)
 * Values are built bottom-up on an owned-node stack: @c begin_object /
 * @c begin_seq push an empty node, members/elements are attached into the top
 * node, and @c end_object / @c end_seq pop the completed node and move it into
 * its parent. Nothing holds a pointer into a @c boost::json container across an
 * insertion, so container reallocation cannot dangle. Each stack frame captures
 * the member key in force when it was opened, so a nested node is attached under
 * the right key even though inner @c member() calls overwrite the pending key.
 *
 * @par Navigation (load)
 * The loader walks the parsed tree in the same fixed member order: object
 * members are looked up by name (order-tolerant), array elements by an advancing
 * per-level cursor.
 */
/******************************************************************************/

/**
 * @brief The saving (output) JSON codec. Assembles a @c boost::json::value,
 * retrievable as a tree (@ref value) or serialized text (@ref str).
 */
class GJsonOArchive : public GOArchiveT<GJsonOArchive> {
public:
    GJsonOArchive() = default;

    /** @brief The assembled JSON tree. */
    [[nodiscard]] const boost::json::value &value() const { return root_; }
    /** @brief The assembled tree serialized to compact JSON text. */
    [[nodiscard]] std::string str() const { return boost::json::serialize(root_); }

    // --- primitive hooks --------------------------------------------------

    /** @brief Stores a boolean. @param b The value. */
    void put_bool(bool b) { place(boost::json::value(b)); }

    /** @brief Stores an integer as a signed or unsigned JSON number by its signedness. @param v The value. */
    template <typename Int>
    void put_scalar(Int v) {
        static_assert(std::is_integral_v<Int>);
        if constexpr (std::is_signed_v<Int>) {
            place(boost::json::value(static_cast<std::int64_t>(v)));
        } else {
            place(boost::json::value(static_cast<std::uint64_t>(v)));
        }
    }

    /** @brief Stores a float/double as a JSON number, a long double as an exact hex-float string. @param v The value. */
    template <typename Float>
    void put_fp(Float v) {
        static_assert(std::is_floating_point_v<Float>);
        if constexpr (std::is_same_v<Float, long double>) {
            place(boost::json::value(long_double_to_hex(v)));
        } else {
            place(boost::json::value(static_cast<double>(v)));
        }
    }

    /** @brief Stores a string. @param s The string. */
    void put_string(std::string_view s) { place(boost::json::value(s)); }

    // --- framing hooks ----------------------------------------------------

    void begin_object() { open(boost::json::value(boost::json::object{})); }
    void end_object() { close(); }
    /** @brief Records the key the next attached value/node uses inside an object. @param name The member name. */
    void member(const char *name) { pending_key_ = name; }
    void begin_seq(std::size_t /*n*/) { open(boost::json::value(boost::json::array{})); }
    void end_seq() { close(); }
    void begin_elem() {}
    void end_elem() {}

private:
    struct Frame {
        boost::json::value node;
        std::string key; // the key this node occupies in its parent (if the parent is an object)
    };

    void place(boost::json::value v) {
        if (stack_.empty()) {
            root_ = std::move(v);
        } else {
            attach(stack_.back().node, std::move(v), pending_key_);
        }
    }

    void open(boost::json::value container) {
        stack_.push_back(Frame{std::move(container), pending_key_});
    }

    void close() {
        Frame f = std::move(stack_.back());
        stack_.pop_back();
        if (stack_.empty()) {
            root_ = std::move(f.node);
        } else {
            attach(stack_.back().node, std::move(f.node), f.key);
        }
    }

    static void attach(boost::json::value &parent, boost::json::value v, const std::string &key) {
        if (parent.is_object()) {
            parent.as_object()[key] = std::move(v);
        } else {
            parent.as_array().push_back(std::move(v));
        }
    }

    static std::string long_double_to_hex(long double v) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%La", v); // hex float: exact and re-parseable
        return std::string{buf};
    }

    boost::json::value root_;
    std::string pending_key_;
    std::vector<Frame> stack_;
};

/******************************************************************************/
/**
 * @brief The loading (input) JSON codec. Reads from parsed JSON text or an
 * existing @c boost::json::value.
 */
class GJsonIArchive : public GIArchiveT<GJsonIArchive> {
public:
    /** @brief Parses @p text as JSON to read from. @param text The JSON document. */
    explicit GJsonIArchive(std::string_view text) : root_(boost::json::parse(text)) {}
    /** @brief Reads from an existing JSON tree. @param v The JSON tree (copied). */
    explicit GJsonIArchive(boost::json::value v) : root_(std::move(v)) {}

    // --- primitive hooks --------------------------------------------------

    /** @brief Reads a boolean. @param b The value to fill. */
    void get_bool(bool &b) { b = slot().as_bool(); }

    /** @brief Reads an integer with a checked numeric conversion. @param v The value to fill. */
    template <typename Int>
    void get_scalar(Int &v) {
        static_assert(std::is_integral_v<Int>);
        v = slot().to_number<Int>();
    }

    /** @brief Reads a float/double from a JSON number, a long double from its hex-float string. @param v The value. */
    template <typename Float>
    void get_fp(Float &v) {
        static_assert(std::is_floating_point_v<Float>);
        if constexpr (std::is_same_v<Float, long double>) {
            const boost::json::string &s = slot().as_string();
            v = std::strtold(s.c_str(), nullptr);
        } else {
            v = static_cast<Float>(slot().to_number<double>());
        }
    }

    /** @brief Reads a string. @param s The string to fill. */
    void get_string(std::string &s) {
        const boost::json::string &js = slot().as_string();
        s.assign(js.begin(), js.end());
    }

    // --- framing hooks ----------------------------------------------------

    void enter_object() {
        const boost::json::value &s = slot();
        stack_.push_back(Cursor{&s, 0});
    }
    void leave_object() { stack_.pop_back(); }
    /** @brief Reads a container's element count. @return The number of elements. */
    std::size_t begin_seq() {
        const boost::json::value &s = slot();
        std::size_t n = s.as_array().size();
        stack_.push_back(Cursor{&s, 0});
        return n;
    }
    void end_seq() { stack_.pop_back(); }
    /** @brief Selects the object member the next read targets. @param name The member name. */
    void member(const char *name) { pending_key_ = name; }
    void begin_elem() {}
    void end_elem() {}

private:
    struct Cursor {
        const boost::json::value *node;
        std::size_t idx; // next array index to read (unused for objects)
    };

    // The next value to read at the current level: an object member by key, an
    // array element by advancing cursor, or the whole tree at top level.
    const boost::json::value &slot() {
        if (stack_.empty()) {
            return root_;
        }
        Cursor &c = stack_.back();
        if (c.node->is_object()) {
            return c.node->as_object().at(pending_key_);
        }
        return c.node->as_array().at(c.idx++);
    }

    boost::json::value root_;
    std::string pending_key_;
    std::vector<Cursor> stack_;
};

/******************************************************************************/

} // namespace Gem::Common::archive
