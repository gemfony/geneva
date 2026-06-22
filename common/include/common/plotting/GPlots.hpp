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

#include "common/plotting/GBasePlotter.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * A variadic data collector for N-d data of user-defined component types. It is
 * the single, generic implementation backing the named GDataCollector{1T,2T,2ET,
 * 3T,4T} collectors (see the alias declarations below). The data is stored in
 * columnar (struct-of-arrays) form: one std::vector per axis, i.e.
 * std::tuple<std::vector<Ts>...>. This matches the per-axis C arrays the emitter
 * builds and removes the std::get<>() churn of the previous array-of-tuples
 * layout. A logical data item is the cross-section of all columns at one index;
 * the per-axis machinery (the narrowing operator& overloads, the projections,
 * sorting and min/max extraction) is generated from the parameter pack via fold
 * expressions / std::index_sequence.
 *
 * The class is assumed to be movable, hence we use all defaulted constructors and
 * assignment operators.
 *
 * @tparam Ts The numeric component types of each stored data item (one per axis)
 */
template <typename... Ts>
class GDataCollectorT : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) & BOOST_SERIALIZATION_NVP(columns_);
    }
    ///////////////////////////////////////////////////////////////////////

    /** @brief The number of axes / components of each stored data item */
    static constexpr std::size_t n_axes = sizeof...(Ts);
    /** @brief The component type of axis I (e.g. axis_t<0> is the x-component type) */
    template <std::size_t I>
    using axis_t = std::tuple_element_t<I, std::tuple<Ts...>>;
    /** @brief The logical type of a single data item (the cross-section of all columns) */
    using item_t = std::tuple<Ts...>;
    /** @brief The columnar storage type: one std::vector per axis */
    using columns_t = std::tuple<std::vector<Ts>...>;

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollectorT() = default;
    GDataCollectorT(GDataCollectorT<Ts...> const &) = default;
    GDataCollectorT(GDataCollectorT<Ts...> &&) = default;

    ~GDataCollectorT() override = default;

    GDataCollectorT<Ts...> &operator=(GDataCollectorT<Ts...> const &) = default;
    GDataCollectorT<Ts...> &operator=(GDataCollectorT<Ts...> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to retrieve information about the amount of data sets stored in
	  * this object
	  *
	  * @return The number of data items currently stored in this collector
	  */
    [[nodiscard]] std::size_t currentSize() const {
        return std::get<0>(columns_).size();
    }

    /***************************************************************************/
    /**
	  * Reserves capacity in every column, so a known number of subsequent
	  * insertions does not repeatedly reallocate.
	  *
	  * @param n The number of data items to reserve room for
	  */
    void reserve(std::size_t n) {
        reserveImpl(n, std::make_index_sequence<n_axes>{});
    }

    /***************************************************************************/
    /**
	  * Alias for reserve(), provided for call sites that pass a non-binding
	  * size hint.
	  *
	  * @param n The number of data items to reserve room for
	  */
    void reserveHint(std::size_t n) {
        this->reserve(n);
    }

    /***************************************************************************/
    /**
	  * This very simple functions allows derived classes
	  * to add data easily to their data sets, when called through a
	  * pointer. I.e., this makes "object_ptr->add(data)" instead of
	  * "*object_ptr & data" possible.
	  *
	  * @tparam data_type The type of the data item being added
	  * @param item The data item to be added to the collection
	  */
    template <typename data_type>
    void add(const data_type &item) {
        *this &item;
    }

    /***************************************************************************/
    /**
	  * For multi-axis (1<) collectors: convenience add() taking one argument per
	  * axis, assembling them into a tuple before forwarding to operator&.
	  *
	  * @param items One value per axis, to be combined into a single data item
	  */
    template <typename... Args, std::size_t M = n_axes, std::enable_if_t<(M > 1) && sizeof...(Args) == M, int> = 0>
    void add(const Args &...items) {
        *this &std::make_tuple(items...);
    }

    /***************************************************************************/
    // The data-insertion operators. For a single-axis collector (n_axes == 1)
    // values are passed bare; for multi-axis collectors they are passed as a
    // std::tuple. Both the exactly-typed and the convertible ("undetermined")
    // forms are provided, the latter performing a checked narrow<> conversion.

    /**
	  * Adds a single, exactly-typed data item.
	  *
	  * @param item The data item to be added to the collection
	  */
    void operator&(const item_t &item) {
        pushItem(item, std::make_index_sequence<n_axes>{});
    }

    /**
	  * Adds a single data item of undetermined component type(s), provided it can
	  * be converted safely to the target component type(s).
	  *
	  * @tparam Us The source component type(s), narrowed to the target type(s)
	  * @param item_undet The data item to be added to the collection
	  */
    template <typename... Us, std::enable_if_t<sizeof...(Us) == sizeof...(Ts), int> = 0>
    void operator&(const std::tuple<Us...> &item_undet) {
        pushItem(narrowItem(item_undet), std::make_index_sequence<n_axes>{});
    }

    /**
	  * For a single-axis collector only: adds a bare value of undetermined type,
	  * provided it can be converted safely to the target component type.
	  *
	  * @tparam U The source type of the data item, narrowed to the target type
	  * @param x_undet The data item to be added to the collection
	  */
    template <typename U, std::size_t M = n_axes, std::enable_if_t<M == 1, int> = 0>
    void operator&(const U &x_undet) {
        axis_t<0> x = axis_t<0>(0);

        // Make sure the data can be converted to doubles
        try {
            x = Gem::Common::narrow<axis_t<0>>(x_undet);
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollectorT::operator&(const T&): Error!" << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }

        // Add the converted data to our collection
        std::get<0>(columns_).push_back(x);
    }

    /**
	  * Adds a collection of exactly-typed data items in one go.
	  *
	  * @param cnt A vector of data items to be added to the collection
	  */
    void operator&(const std::vector<item_t> &cnt) {
        this->reserve(this->currentSize() + cnt.size());
        for(auto const &item : cnt) {
            // Add the data item to our collection
            pushItem(item, std::make_index_sequence<n_axes>{});
        }
    }

    /**
	  * Adds a contiguous range of exactly-typed data items in one go. This span
	  * overload sits alongside the std::vector overload above and lets callers
	  * pass arrays / sub-ranges without first materializing a std::vector.
	  *
	  * @param cnt A span of data items to be added to the collection
	  */
    void operator&(std::span<const item_t> cnt) {
        this->reserve(this->currentSize() + cnt.size());
        for(auto const &item : cnt) {
            // Add the data item to our collection
            pushItem(item, std::make_index_sequence<n_axes>{});
        }
    }

    /**
	  * Adds a collection of data items of undetermined component type(s) in one
	  * go, provided they can be converted safely into the target component type(s).
	  *
	  * @tparam Us The source component type(s), narrowed to the target type(s)
	  * @param cnt_undet A vector of data items of undetermined type, to be added
	  */
    template <typename... Us, std::enable_if_t<sizeof...(Us) == sizeof...(Ts), int> = 0>
    void operator&(const std::vector<std::tuple<Us...>> &cnt_undet) {
        this->reserve(this->currentSize() + cnt_undet.size());
        for(auto const &item_undet : cnt_undet) {
            pushItem(narrowItem(item_undet), std::make_index_sequence<n_axes>{});
        }
    }

    /**
	  * Adds a contiguous range of data items of undetermined component type(s) in
	  * one go (span counterpart of the std::vector overload above), provided they
	  * can be converted safely into the target component type(s).
	  *
	  * @tparam Us The source component type(s), narrowed to the target type(s)
	  * @param cnt_undet A span of data items of undetermined type, to be added
	  */
    template <typename... Us, std::enable_if_t<sizeof...(Us) == sizeof...(Ts), int> = 0>
    void operator&(std::span<const std::tuple<Us...>> cnt_undet) {
        this->reserve(this->currentSize() + cnt_undet.size());
        for(auto const &item_undet : cnt_undet) {
            pushItem(narrowItem(item_undet), std::make_index_sequence<n_axes>{});
        }
    }

    /**
	  * For a single-axis collector only: adds a collection of bare values of
	  * undetermined type, provided they can be converted safely to the target type.
	  *
	  * @tparam U The source element type of the vector, narrowed to the target type
	  * @param x_cnt_undet A collection of data items of undetermined type, to be added
	  */
    template <typename U, std::size_t M = n_axes, std::enable_if_t<M == 1, int> = 0>
    void operator&(const std::vector<U> &x_cnt_undet) {
        axis_t<0> x = axis_t<0>(0);

        std::get<0>(columns_).reserve(std::get<0>(columns_).size() + x_cnt_undet.size());
        for(auto const &src : x_cnt_undet) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<axis_t<0>>(src);
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollectorT::operator&(const std::vector<T>&): Error!" << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            // Add the converted data to our collection
            std::get<0>(columns_).push_back(x);
        }
    }

    /**
	  * For a single-axis collector only: adds a contiguous range of bare values of
	  * undetermined type (span counterpart of the std::vector overload above),
	  * provided they can be converted safely to the target type.
	  *
	  * @tparam U The source element type of the span, narrowed to the target type
	  * @param x_cnt_undet A span of data items of undetermined type, to be added
	  */
    template <typename U, std::size_t M = n_axes, std::enable_if_t<M == 1, int> = 0>
    void operator&(std::span<const U> x_cnt_undet) {
        axis_t<0> x = axis_t<0>(0);

        std::get<0>(columns_).reserve(std::get<0>(columns_).size() + x_cnt_undet.size());
        for(auto const &src : x_cnt_undet) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<axis_t<0>>(src);
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollectorT::operator&(std::span<const T>): Error!" << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            // Add the converted data to our collection
            std::get<0>(columns_).push_back(x);
        }
    }

    /**
	  * Adds a collection of exactly-typed data items in one go, stealing the
	  * incoming buffer for a single-axis collector (where the buffer matches the
	  * sole column directly). For multi-axis collectors there is no single column
	  * to move into, so the items are appended element-wise.
	  *
	  * @param cnt A vector of data items to be moved into the collection
	  */
    void operator&(std::vector<item_t> &&cnt) {
        if constexpr(n_axes == 1) {
            auto &col = std::get<0>(columns_);
            if(col.empty()) {
                // Steal the incoming buffer wholesale (item_t is a 1-tuple of axis_t<0>;
                // it is layout-equivalent to its sole element, but we move element-wise
                // to stay strictly within well-defined behavior).
                col.reserve(cnt.size());
                for(auto &item : cnt) {
                    col.push_back(std::move(std::get<0>(item)));
                }
            }
            else {
                col.reserve(col.size() + cnt.size());
                for(auto &item : cnt) {
                    col.push_back(std::move(std::get<0>(item)));
                }
            }
        }
        else {
            this->reserve(this->currentSize() + cnt.size());
            for(auto &item : cnt) {
                pushItem(std::move(item), std::make_index_sequence<n_axes>{});
            }
        }
    }

    /***************************************************************************/
    /**
	  * Projects the data onto axis I, returning a 1-d histogram. This generic
	  * version is a trap to catch calls with un-implemented types -- it is only
	  * meaningfully specialized for all-double collectors (see project<I>()
	  * below and the named projectX/Y/Z/W wrappers).
	  *
	  * @tparam I The axis to project onto
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto axis I (only in specializations)
	  */
    template <std::size_t I>
    std::shared_ptr<GDataCollectorT<axis_t<I>>>
    project([[maybe_unused]] std::size_t nBins, [[maybe_unused]] std::tuple<axis_t<I>, axis_t<I>> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollectorT<>::project<I>(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollectorT<axis_t<I>>>();
    }

    /***************************************************************************/
    // Named projection wrappers, preserved from the original per-arity API. They
    // are only available for the axes actually present in this collector.

    // The named wrappers are templated on their (fixed-by-default) axis index so
    // that the axis_t<I> appearing in their signature is a dependent type, only
    // instantiated when the wrapper is actually called -- otherwise a low-arity
    // collector (e.g. the single-axis histogram base) would hard-error merely by
    // forming projectY/Z/W's parameter type. enable_if then keeps each wrapper
    // available only for collectors that actually have the requested axis.

    /** @brief Projects the data onto the x-axis (see project<0>()) */
    template <std::size_t I = 0, std::enable_if_t<(I == 0) && (n_axes >= 1), int> = 0>
    auto projectX(std::size_t nBins, std::tuple<axis_t<I>, axis_t<I>> range) const {
        return this->template project<I>(nBins, range);
    }

    /** @brief Projects the data onto the y-axis (see project<1>()) */
    template <std::size_t I = 1, std::enable_if_t<(I == 1) && (n_axes >= 2), int> = 0>
    auto projectY(std::size_t nBins, std::tuple<axis_t<I>, axis_t<I>> range) const {
        return this->template project<I>(nBins, range);
    }

    /** @brief Projects the data onto the z-axis (see project<2>()) */
    template <std::size_t I = 2, std::enable_if_t<(I == 2) && (n_axes >= 3), int> = 0>
    auto projectZ(std::size_t nBins, std::tuple<axis_t<I>, axis_t<I>> range) const {
        return this->template project<I>(nBins, range);
    }

    /** @brief Projects the data onto the w-axis (see project<3>()) */
    template <std::size_t I = 3, std::enable_if_t<(I == 3) && (n_axes >= 4), int> = 0>
    auto projectW(std::size_t nBins, std::tuple<axis_t<I>, axis_t<I>> range) const {
        return this->template project<I>(nBins, range);
    }

    /***************************************************************************/
    /**
	  * Sorts the data according to its x-component (axis 0)
	  */
    void sortX() {
        const std::size_t n = this->currentSize();
        if(n < 2) {
            return;
        }

        // Reproduce the previous behaviour byte-for-byte: the old storage was a
        // std::vector<item_t> and was sorted with std::sort comparing only the
        // x-component (axis 0). std::sort is not stable, so the exact order of
        // x-ties is an artefact of the algorithm operating on that very element
        // sequence. We therefore materialize the item tuples, run the identical
        // sort, and scatter the result back into the columns -- this co-sorts all
        // axes (a row stays together) and matches the old ordering exactly.
        std::vector<item_t> items = asTuples();
        std::ranges::sort(items, [](const item_t &a, const item_t &b) -> bool {
            return std::get<0>(a) < std::get<0>(b);
        });
        assignFromTuples(items, std::make_index_sequence<n_axes>{});
    }

    /***************************************************************************/
    /**
	  * Retrieves the per-axis minimum and maximum values in data_, interleaved as
	  * (min_0, max_0, min_1, max_1, ...). For a single-axis collector this is the
	  * familiar (min, max); for a two-axis collector it is (min_x, max_x, min_y,
	  * max_y), matching the original per-arity return type.
	  *
	  * @return A tuple holding the per-axis (min, max) pairs of the stored data
	  */
    [[nodiscard]] auto getMinMaxElements() const {
        if(this->currentSize() == 0) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollectorT::getMinMaxElements(): Error!" << '\n'
                << "Cannot determine the data range of an empty collector." << '\n'
            );
        }
        return minMaxImpl(std::make_index_sequence<n_axes>{});
    };

    /***************************************************************************/
    /**
	  * Const, read-only access to the per-axis value vector (the column) for axis
	  * I. This exposes the columnar data to out-of-hierarchy consumers (the
	  * pluggable plot emitters) without granting any mutation path; the writable
	  * column() overload remains protected / internal to the collector hierarchy.
	  *
	  * @tparam I The axis whose value vector should be returned
	  * @return A const reference to the std::vector holding axis I's values
	  */
    template <std::size_t I>
    const std::vector<axis_t<I>> &column() const {
        return std::get<I>(columns_);
    }

protected:
    /***************************************************************************/
    /**
	  * Single declaration of this class'es local data members, used by load_() and compare_()
	  *
	  * @return A tuple of named members (the data vector) of this object
	  */
    template <typename Self>
    static auto localMembers_(Self &self) {
        // Expose one named member per column, so each column is compared via the
        // existing sequence-container comparison path (exactly as the former single
        // data_ vector was). The columns are the serialized / compared state.
        return localMembersImpl_(self, std::make_index_sequence<n_axes>{});
    }

    /** @brief Stable per-axis member name ("column_0", "column_1", ...) for diagnostics */
    template <std::size_t I>
    static constexpr const char *columnName_() {
        constexpr const char *names[] = {
            "column_0", "column_1", "column_2", "column_3",
            "column_4", "column_5", "column_6", "column_7"
        };
        static_assert(I < (sizeof(names) / sizeof(names[0])), "GDataCollectorT: too many axes for column naming");
        return names[I];
    }

    template <typename Self, std::size_t... Is>
    static auto localMembersImpl_(Self &self, std::index_sequence<Is...>) {
        return std::make_tuple(make_member(columnName_<Is>(), std::get<Is>(self.columns_))...);
    }

    /**
	  * Loads the data of another object
	  *
	  * @param cp A pointer to another GDataCollectorT<Ts...> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollectorT<Ts...> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own, derived from the single localMembers() declaration
        g_load_members(localMembers_(*this), localMembers_(*p_load));
    }

    /***************************************************************************/

    friend void compare_base_t<GDataCollectorT<Ts...>>(
        GDataCollectorT<Ts...> const &,
        GDataCollectorT<Ts...> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	  * @param e The expectation for this object (e.g. equality or inequality)
	  * @param limit The maximum allowed deviation for floating point comparisons (unused here)
	  */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        // Check that we are dealing with a GDataCollectorT<Ts...> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollectorT<Ts...>", e);

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * Returns the column (the per-axis value vector) for axis I, for use by the
	  * concrete plotters' emission code. Both const and non-const overloads are
	  * provided; they are protected, i.e. internal to the collector hierarchy.
	  *
	  * @tparam I The axis whose value vector should be returned
	  * @return A reference to the std::vector holding axis I's values
	  */
    template <std::size_t I>
    std::vector<axis_t<I>> &column() {
        return std::get<I>(columns_);
    }

    /***************************************************************************/

    columns_t columns_; ///< Holds the actual data in columnar (struct-of-arrays) form

private:
    /***************************************************************************/
    /**
	  * Appends one logical item (one value per axis) to the columns.
	  */
    template <typename Tuple, std::size_t... Is>
    void pushItem(Tuple &&item, std::index_sequence<Is...>) {
        (std::get<Is>(columns_).push_back(std::get<Is>(std::forward<Tuple>(item))), ...);
    }

    /***************************************************************************/
    /**
	  * Reserves capacity n in every column.
	  */
    template <std::size_t... Is>
    void reserveImpl(std::size_t n, std::index_sequence<Is...>) {
        (std::get<Is>(columns_).reserve(n), ...);
    }

    /***************************************************************************/
    /**
	  * Materializes the columnar data into a vector of item tuples (an
	  * array-of-tuples view). Used by the few legacy call sites (project<>() and
	  * the GGraph4D footer) that drive the free getMinMax(vector-of-tuples) helper
	  * and by sortX().
	  *
	  * @return A vector of item tuples reconstructed from the columns
	  */
    std::vector<item_t> asTuples() const {
        return asTuplesImpl(std::make_index_sequence<n_axes>{});
    }

    template <std::size_t... Is>
    std::vector<item_t> asTuplesImpl(std::index_sequence<Is...>) const {
        const std::size_t n = this->currentSize();
        std::vector<item_t> out;
        out.reserve(n);
        for(std::size_t i = 0; i < n; ++i) {
            out.emplace_back(std::get<Is>(columns_)[i]...);
        }
        return out;
    }

    /***************************************************************************/
    /**
	  * Replaces the columns with the contents of a vector of item tuples,
	  * de-interleaving each axis into its own column.
	  */
    template <std::size_t... Is>
    void assignFromTuples(const std::vector<item_t> &items, std::index_sequence<Is...>) {
        (std::get<Is>(columns_).clear(), ...);
        (std::get<Is>(columns_).reserve(items.size()), ...);
        for(const auto &item : items) {
            (std::get<Is>(columns_).push_back(std::get<Is>(item)), ...);
        }
    }

    /***************************************************************************/
    /**
	  * Narrows a tuple of undetermined component types into the target item type,
	  * wrapping every per-component narrow<> in the original try/catch contract.
	  */
    template <typename... Us>
    item_t narrowItem(const std::tuple<Us...> &src) const {
        return narrowItemImpl(src, std::make_index_sequence<n_axes>{});
    }

    template <typename Src, std::size_t... Is>
    item_t narrowItemImpl(const Src &src, std::index_sequence<Is...>) const {
        try {
            return item_t(Gem::Common::narrow<axis_t<Is>>(std::get<Is>(src))...);
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollectorT::operator&(tuple): Error!" << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }
    }

    /***************************************************************************/
    /**
	  * Computes the per-axis (min, max) of a single axis I across the data vector.
	  */
    template <std::size_t I>
    std::pair<axis_t<I>, axis_t<I>> minMaxAxis() const {
        const auto &col = std::get<I>(columns_);
        auto [min_it, max_it] = std::ranges::minmax_element(col);
        return {*min_it, *max_it};
    }

    template <std::size_t... Is>
    auto minMaxImpl(std::index_sequence<Is...>) const {
        // Interleave the per-axis (min, max) pairs into a single flat tuple
        return std::tuple_cat(
            std::make_tuple(minMaxAxis<Is>().first, minMaxAxis<Is>().second)...
        );
    }

    /***************************************************************************/
    /**
	  * Returns the name of this class
	  *
	  * @return The name of this class as a string
	  */
    std::string name_() const override {
        return std::string("GDataCollectorT<Ts...>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	  */
    GBasePlotter *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
/**
 * The five named data collectors are thin aliases over the single variadic
 * GDataCollectorT. They preserve the historical names, arities and the exact
 * stored layout (std::vector<std::tuple<Ts...>>) the concrete plotters rely on.
 *
 * - GDataCollector1T<X>        : 1-d data (e.g. for histograms)
 * - GDataCollector2T<X,Y>      : 2-d data (e.g. for a TGraph)
 * - GDataCollector2ET<X,Y>     : 2-d data with x- and y-errors (x, error_x, y, error_y)
 * - GDataCollector3T<X,Y,Z>    : 3-d data
 * - GDataCollector4T<X,Y,Z,W>  : 4-d data
 */
template <typename x_type>
using GDataCollector1T = GDataCollectorT<x_type>;

template <typename x_type, typename y_type>
using GDataCollector2T = GDataCollectorT<x_type, y_type>;

template <typename x_type, typename y_type, typename z_type>
using GDataCollector3T = GDataCollectorT<x_type, y_type, z_type>;

template <typename x_type, typename y_type, typename z_type, typename w_type>
using GDataCollector4T = GDataCollectorT<x_type, y_type, z_type, w_type>;

/******************************************************************************/
/**
 * The 2-d data collector with error bars stores a 4-tuple (x, error_x, y,
 * error_y). Its tuple layout (X, X, Y, Y) would otherwise be indistinguishable
 * from a four-axis GDataCollector4T<X, X, Y, Y>, so it is realized as a distinct,
 * trivial subclass of the variadic base rather than as a bare alias. This keeps
 * its identity (and its is_abstract<> Boost.Serialization marker) separate while
 * inheriting all of the shared collector machinery unchanged.
 *
 * @tparam x_type The numeric type of the x-component and its error
 * @tparam y_type The numeric type of the y-component and its error
 */
template <typename x_type, typename y_type>
class GDataCollector2ET : public GDataCollectorT<x_type, x_type, y_type, y_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollectorT_base",
            boost::serialization::base_object<GDataCollectorT<x_type, x_type, y_type, y_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors -- all machinery is inherited

    GDataCollector2ET() = default;
    GDataCollector2ET(GDataCollector2ET<x_type, y_type> const &) = default;
    GDataCollector2ET(GDataCollector2ET<x_type, y_type> &&) = default;

    ~GDataCollector2ET() override = default;

    GDataCollector2ET<x_type, y_type> &operator=(GDataCollector2ET<x_type, y_type> const &) = default;
    GDataCollector2ET<x_type, y_type> &operator=(GDataCollector2ET<x_type, y_type> &&) = default;

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object. This class adds no own members, so it
	  * simply forwards to the variadic base.
	  *
	  * @param cp A pointer to another GDataCollector2ET<x_type, y_type> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Ensure the camouflaged pointer is an independent object of our own type
        (void) g_convert_and_compare(cp, this);
        // No own members -- defer entirely to the base
        GDataCollectorT<x_type, x_type, y_type, y_type>::load_(cp);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector2ET<x_type, y_type>>(
        GDataCollector2ET<x_type, y_type> const &,
        GDataCollector2ET<x_type, y_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type. This class adds no own members, so it simply compares
	  * the variadic base.
	  *
	  * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	  * @param e The expectation for this object (e.g. equality or inequality)
	  * @param limit The maximum allowed deviation for floating point comparisons (unused here)
	  */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        // Ensure the camouflaged pointer is an independent object of our own type
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollector2ET<x_type, y_type>", e);

        // No own members -- compare the variadic base only
        compare_base_t<GDataCollectorT<x_type, x_type, y_type, y_type>>(*this, *p_load, token);

        // React on deviations from the expectation
        token.evaluate();
    }

private:
    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	  */
    GBasePlotter *clone_() const override = 0;
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH1D class (1-d double data). This will result in a 2D-plot.
 */
class GHistogram1D : public GDataCollector1T<double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector1T_double",
            boost::serialization::base_object<GDataCollector1T<double>>(*this)
        ) & BOOST_SERIALIZATION_NVP(n_bins_x_) &
            BOOST_SERIALIZATION_NVP(min_x_) & BOOST_SERIALIZATION_NVP(max_x_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
	 * @brief Initialization with the number of bins and automatic range detection
	 * @param nBinsX The number of bins in x-direction
	 */
    explicit GHistogram1D(const std::size_t &n_bins_x);

    /**
	 * @brief Initialization with the number of bins and an explicit range
	 * @param nBinsX The number of bins in x-direction
	 * @param minX The lower boundary of the histogram
	 * @param maxX The upper boundary of the histogram
	 */
    GHistogram1D(const std::size_t &n_bins_x, const double &min_x, const double &max_x);
    /**
	 * @brief Initialization with the number of bins and a range in the form of a tuple
	 * @param nBinsX The number of bins in x-direction
	 * @param rangeX The lower and upper boundaries of the histogram, as a tuple
	 */
    GHistogram1D(const std::size_t &n_bins_x, const std::tuple<double, double> &range_x);

    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GHistogram1D(GHistogram1D const &) = default;
    GHistogram1D(GHistogram1D &&) = default;
    ~GHistogram1D() override = default;

    GHistogram1D &operator=(GHistogram1D const &) = default;
    GHistogram1D &operator=(GHistogram1D &&) = default;

    // Defaulted default-constructor in private section

    /**********************************************************************/

    /**
	 * @brief Retrieve the number of bins in x-direction
	 * @return The number of bins in x-direction
	 */
    std::size_t getNBinsX() const;

    /**
	 * @brief Retrieve the lower boundary of the plot
	 * @return The lower boundary of the histogram in x-direction
	 */
    double getMinX() const;
    /**
	 * @brief Retrieve the upper boundary of the plot
	 * @return The upper boundary of the histogram in x-direction
	 */
    double getMaxX() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("n_bins_x_", self.n_bins_x_),
            make_member("min_x_", self.min_x_),
            make_member("max_x_", self.max_x_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another GHistogram1D object, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /***************************************************************************/

    friend void compare_base_t<GHistogram1D>(GHistogram1D const &, GHistogram1D const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    GHistogram1D() =
        default; ///< The default constructor -- intentionally private as it is only needed for (de-)serialization

    std::size_t n_bins_x_ = 10; ///< The number of bins in the histogram

    double min_x_ = 0;     ///< The lower boundary of the histogram
    double max_x_ = min_x_; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH1I class (1-d integer data)
 */
class GHistogram1I : public GDataCollector1T<std::int32_t> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector1T_int32_t",
            boost::serialization::base_object<GDataCollector1T<std::int32_t>>(*this)
        ) & BOOST_SERIALIZATION_NVP(n_bins_x_) &
            BOOST_SERIALIZATION_NVP(min_x_) & BOOST_SERIALIZATION_NVP(max_x_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
	 * @brief The standard constructor
	 * @param nBinsX The number of bins in x-direction
	 * @param minX The lower boundary of the histogram
	 * @param maxX The upper boundary of the histogram
	 */
    GHistogram1I(const std::size_t &n_bins_x, const double &min_x, const double &max_x);
    /**
	 * @brief Initialization with a range in the form of a tuple
	 * @param nBinsX The number of bins in x-direction
	 * @param rangeX The lower and upper boundaries of the histogram, as a tuple
	 */
    GHistogram1I(const std::size_t &n_bins_x, const std::tuple<double, double> &range_x);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operator

    GHistogram1I(GHistogram1I const &) = default;
    GHistogram1I(GHistogram1I &&) = default;

    // Defaulted default-constructor in private section

    ~GHistogram1I() override = default;

    GHistogram1I &operator=(GHistogram1I const &) = default;
    GHistogram1I &operator=(GHistogram1I &&) = default;

    /*********************************************************************/

    /**
	 * @brief Retrieve the number of bins in x-direction
	 * @return The number of bins in x-direction
	 */
    std::size_t getNBinsX() const;

    /**
	 * @brief Retrieve the lower boundary of the plot
	 * @return The lower boundary of the histogram in x-direction
	 */
    double getMinX() const;
    /**
	 * @brief Retrieve the upper boundary of the plot
	 * @return The upper boundary of the histogram in x-direction
	 */
    double getMaxX() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("n_bins_x_", self.n_bins_x_),
            make_member("min_x_", self.min_x_),
            make_member("max_x_", self.max_x_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another GHistogram1I object, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GHistogram1I>(GHistogram1I const &, GHistogram1I const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    GHistogram1I() =
        default; ///< The default constructor -- intentionally private as it is only needed for (de-)serialization

    std::size_t n_bins_x_ = 0; ///< The number of bins in the histogram

    double min_x_ = 0.; ///< The lower boundary of the histogram
    double max_x_ = 0.; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_x The number of bins of the histogram
 * @param range_x The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the x-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>> GDataCollectorT<double, double>::project<0>(
    std::size_t n_bins_x,
    std::tuple<double, double> range_x
) const {
    std::tuple<double, double> my_range_x;
    std::tuple<double, double> default_range;
    if(range_x == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double> extremes = getMinMax(this->asTuples());
        my_range_x = std::tuple<double, double>(std::get<0>(extremes), std::get<1>(extremes));
    }
    else {
        my_range_x = range_x;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_x, my_range_x));
    result->setXAxisLabel(this->xAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / x-projection");

    // Add data to the object
    for(auto const &v : this->template column<0>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_y The number of bins of the histogram
 * @param range_y The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the y-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>> GDataCollectorT<double, double>::project<1>(
    std::size_t n_bins_y,
    std::tuple<double, double> range_y
) const {
    std::tuple<double, double> my_range_y;
    std::tuple<double, double> default_range;
    if(range_y == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double> extremes = getMinMax(this->asTuples());
        my_range_y = std::tuple<double, double>(std::get<2>(extremes), std::get<3>(extremes));
    }
    else {
        my_range_y = range_y;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_y, my_range_y));
    result->setXAxisLabel(this->yAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / y-projection");

    // Add data to the object
    for(auto const &v : this->template column<1>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * A wrapper for ROOT's TH2D class (2-d double data). This will result in a
 * 3D plot.
 */
class GHistogram2D : public GDataCollector2T<double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector2T_double_double",
            boost::serialization::base_object<GDataCollector2T<double, double>>(*this)
        ) & BOOST_SERIALIZATION_NVP(n_bins_x_) &
            BOOST_SERIALIZATION_NVP(n_bins_y_) & BOOST_SERIALIZATION_NVP(min_x_) &
            BOOST_SERIALIZATION_NVP(max_x_) & BOOST_SERIALIZATION_NVP(min_y_) &
            BOOST_SERIALIZATION_NVP(max_y_) & BOOST_SERIALIZATION_NVP(dropt_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
	 * @brief The standard constructor
	 * @param nBinsX The number of bins in x-direction
	 * @param nBinsY The number of bins in y-direction
	 * @param minX The lower boundary of the histogram in x-direction
	 * @param maxX The upper boundary of the histogram in x-direction
	 * @param minY The lower boundary of the histogram in y-direction
	 * @param maxY The upper boundary of the histogram in y-direction
	 */
    GHistogram2D(
        const std::size_t &n_bins_x,
        const std::size_t &n_bins_y,
        const double &min_x,
        const double &max_x,
        const double &min_y,
        const double &max_y
    );
    /**
	 * @brief Initialization with ranges given as tuples
	 * @param nBinsX The number of bins in x-direction
	 * @param nBinsY The number of bins in y-direction
	 * @param rangeX The lower and upper boundaries in x-direction, as a tuple
	 * @param rangeY The lower and upper boundaries in y-direction, as a tuple
	 */
    GHistogram2D(
        const std::size_t &n_bins_x,
        const std::size_t &n_bins_y,
        const std::tuple<double, double> &range_x,
        const std::tuple<double, double> &range_y
    );
    /**
	 * @brief Initialization with automatic range detection
	 * @param nBinsX The number of bins in x-direction
	 * @param nBinsY The number of bins in y-direction
	 */
    GHistogram2D(const std::size_t &n_bins_x, const std::size_t &n_bins_y);

    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    GHistogram2D(GHistogram2D const &) = default;
    GHistogram2D(GHistogram2D &&) = default;
    ~GHistogram2D() override = default;

    GHistogram2D &operator=(GHistogram2D const &) = default;
    GHistogram2D &operator=(GHistogram2D &&) = default;

    /**********************************************************************/

    /**
	 * @brief Retrieve the number of bins in x-direction
	 * @return The number of bins in x-direction
	 */
    std::size_t getNBinsX() const;
    /**
	 * @brief Retrieve the number of bins in y-direction
	 * @return The number of bins in y-direction
	 */
    std::size_t getNBinsY() const;

    /**
	 * @brief Retrieve the lower boundary of the plot in x-direction
	 * @return The lower boundary in x-direction
	 */
    double getMinX() const;
    /**
	 * @brief Retrieve the upper boundary of the plot in x-direction
	 * @return The upper boundary in x-direction
	 */
    double getMaxX() const;
    /**
	 * @brief Retrieve the lower boundary of the plot in y-direction
	 * @return The lower boundary in y-direction
	 */
    double getMinY() const;
    /**
	 * @brief Retrieve the upper boundary of the plot in y-direction
	 * @return The upper boundary in y-direction
	 */
    double getMaxY() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

    /**
	 * @brief Allows to specify 2d-drawing options
	 * @param dropt The 2-d drawing option to be used for this histogram
	 */
    void set2DOpt(tddropt dropt);
    /**
	 * @brief Allows to retrieve 2d-drawing options
	 * @return The currently set 2-d drawing option
	 */
    tddropt get2DOpt() const;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("n_bins_x_", self.n_bins_x_),
            make_member("n_bins_y_", self.n_bins_y_),
            make_member("min_x_", self.min_x_),
            make_member("max_x_", self.max_x_),
            make_member("min_y_", self.min_y_),
            make_member("max_y_", self.max_y_),
            make_member("dropt_", self.dropt_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GHistogram2D>(GHistogram2D const &, GHistogram2D const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    GHistogram2D() =
        default; ///< The default constructor -- intentionally private, as it is only needed for (de-)serialization

    std::size_t n_bins_x_ = 0; ///< The number of bins in the x-direction of the histogram
    std::size_t n_bins_y_ = 0; ///< The number of bins in the y-direction of the histogram

    double min_x_ = 0.; ///< The lower boundary of the histogram in x-direction
    double max_x_ = 0.; ///< The upper boundary of the histogram in x-direction
    double min_y_ = 0.; ///< The lower boundary of the histogram in y-direction
    double max_y_ = 0.; ///< The upper boundary of the histogram in y-direction

    tddropt dropt_ = tddropt::BOX; ///< The drawing options for 2-d histograms
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TGraph class (2d data and curve-like structures). It
 * also adds the option to draw arrows between consecutive points. This results
 * in a 2D plot.
 */
class GGraph2D : public GDataCollector2T<double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector2T_double_double",
            boost::serialization::base_object<GDataCollector2T<double, double>>(*this)
        ) & BOOST_SERIALIZATION_NVP(p_m_) &
            BOOST_SERIALIZATION_NVP(draw_arrows_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GGraph2D() = default;
    GGraph2D(GGraph2D const &) = default;
    GGraph2D(GGraph2D &&) = default;
    ~GGraph2D() override = default;

    GGraph2D &operator=(GGraph2D const &) = default;
    GGraph2D &operator=(GGraph2D &&) = default;

    /**********************************************************************/

    /**
	 * @brief Adds arrows to the plots between consecutive points
	 * @param drawArrows Whether arrows should be drawn between consecutive points (default true)
	 */
    void setDrawArrows(bool d_a = true);
    /**
	 * @brief Retrieves the value of the draw_arrows_ variable
	 * @return Whether arrows are drawn between consecutive points
	 */
    bool getDrawArrows() const;

    /**
	 * @brief Determines whether a scatter plot or a curve is created
	 * @param pm The plotting mode (scatter plot or connected curve) to be used
	 */
    void setPlotMode(graphPlotMode p_m);
    /**
	 * @brief Allows to retrieve the current plotting mode
	 * @return The currently set plotting mode
	 */
    graphPlotMode getPlotMode() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("p_m_", self.p_m_),
            make_member("draw_arrows_", self.draw_arrows_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph2D>(GGraph2D const &, GGraph2D const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    graphPlotMode p_m_ =
        DEFPLOTMODE;          ///< Whether to create scatter plots or a curve, connected by lines
    bool draw_arrows_ = false; ///< When set to true, arrows will be drawn between consecutive points
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TGraphErrors class (2d data and curve-like structures).
 * This results in a 2D plot.
 */
class GGraph2ED : public GDataCollector2ET<double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector2ET_double_double",
            boost::serialization::base_object<GDataCollector2ET<double, double>>(*this)
        ) & BOOST_SERIALIZATION_NVP(p_m_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GGraph2ED() = default;
    GGraph2ED(GGraph2ED const &) = default;
    GGraph2ED(GGraph2ED &&) = default;
    ~GGraph2ED() override = default;

    GGraph2ED &operator=(GGraph2ED const &) = default;
    GGraph2ED &operator=(GGraph2ED &&) = default;

    /**********************************************************************/

    /**
	 * @brief Determines whether a scatter plot or a curve is created
	 * @param pm The plotting mode (scatter plot or connected curve) to be used
	 */
    void setPlotMode(graphPlotMode p_m);
    /**
	 * @brief Allows to retrieve the current plotting mode
	 * @return The currently set plotting mode
	 */
    graphPlotMode getPlotMode() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(make_member("p_m_", self.p_m_));
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph2ED>(GGraph2ED const &, GGraph2ED const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    graphPlotMode p_m_ =
        DEFPLOTMODE; ///< Whether to create scatter plots or a curve, connected by lines
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_x The number of bins of the histogram
 * @param range_x The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the x-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>> GDataCollectorT<double, double, double>::project<0>(
    std::size_t n_bins_x,
    std::tuple<double, double> range_x
) const {
    std::tuple<double, double> my_range_x;
    std::tuple<double, double> default_range;
    if(range_x == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double> extremes =
            getMinMax(this->asTuples());
        my_range_x = std::tuple<double, double>(std::get<0>(extremes), std::get<1>(extremes));
    }
    else {
        my_range_x = range_x;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_x, my_range_x));
    result->setXAxisLabel(this->xAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / x-projection");

    // Add data to the object
    for(auto const &v : this->template column<0>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_y The number of bins of the histogram
 * @param range_y The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the y-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>> GDataCollectorT<double, double, double>::project<1>(
    std::size_t n_bins_y,
    std::tuple<double, double> range_y
) const {
    std::tuple<double, double> my_range_y;
    std::tuple<double, double> default_range;
    if(range_y == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(this->asTuples());
        my_range_y = std::tuple<double, double>(std::get<2>(extremes), std::get<3>(extremes));
    }
    else {
        my_range_y = range_y;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_y, my_range_y));
    result->setXAxisLabel(this->yAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / y-projection");

    // Add data to the object
    for(auto const &v : this->template column<1>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_z The number of bins of the histogram
 * @param range_z The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the z-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>> GDataCollectorT<double, double, double>::project<2>(
    std::size_t n_bins_z,
    std::tuple<double, double> range_z
) const {
    std::tuple<double, double> my_range_z;
    std::tuple<double, double> default_range;
    if(range_z == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(this->asTuples());
        my_range_z = std::tuple<double, double>(std::get<4>(extremes), std::get<5>(extremes));
    }
    else {
        my_range_z = range_z;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_z, my_range_z));
    result->setXAxisLabel(this->zAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / z-projection");

    // Add data to the object
    for(auto const &v : this->template column<2>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TGraph2D class (3d data). It
 * also adds the option to draw lines between consecutive points. This class
 * only allows a single plot mode. This results in a 3D plot.
 */
class GGraph3D : public GDataCollector3T<double, double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector3T_3double",
            boost::serialization::base_object<GDataCollector3T<double, double, double>>(*this)
        ) & BOOST_SERIALIZATION_NVP(draw_lines_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GGraph3D() = default;
    GGraph3D(GGraph3D const &) = default;
    GGraph3D(GGraph3D &&) = default;
    ~GGraph3D() override = default;

    GGraph3D &operator=(GGraph3D const &) = default;
    GGraph3D &operator=(GGraph3D &&) = default;

    /*********************************************************************/

    /**
	 * @brief Adds lines to the plots between consecutive points
	 * @param drawLines Whether lines should be drawn between consecutive points (default true)
	 */
    void setDrawLines(bool d_l = true);
    /**
	 * @brief Retrieves the value of the draw_lines_ variable
	 * @return Whether lines are drawn between consecutive points
	 */
    bool getDrawLines() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(make_member("draw_lines_", self.draw_lines_));
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph3D>(GGraph3D const &, GGraph3D const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    bool draw_lines_ = false; ///< When set to true, lines will be drawn between consecutive points
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_x The number of bins of the histogram
 * @param range_x The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the x-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>>
GDataCollectorT<double, double, double, double>::project<0>(
    std::size_t n_bins_x,
    std::tuple<double, double> range_x
) const {
    std::tuple<double, double> my_range_x;
    std::tuple<double, double> default_range;
    if(range_x == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->asTuples());
        my_range_x = std::tuple<double, double>(std::get<0>(extremes), std::get<1>(extremes));
    }
    else {
        my_range_x = range_x;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_x, my_range_x));
    result->setXAxisLabel(this->xAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / x-projection");

    // Add data to the object
    for(auto const &v : this->template column<0>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * Specialization of projectY for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_y The number of bins of the histogram
 * @param range_y The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the y-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>>
GDataCollectorT<double, double, double, double>::project<1>(
    std::size_t n_bins_y,
    std::tuple<double, double> range_y
) const {
    std::tuple<double, double> my_range_y;
    std::tuple<double, double> default_range;
    if(range_y == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->asTuples());
        my_range_y = std::tuple<double, double>(std::get<2>(extremes), std::get<3>(extremes));
    }
    else {
        my_range_y = range_y;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_y, my_range_y));
    result->setXAxisLabel(this->yAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / y-projection");

    // Add data to the object
    for(auto const &v : this->template column<1>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * Specialization of projectZ for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_z The number of bins of the histogram
 * @param range_z The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the z-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>>
GDataCollectorT<double, double, double, double>::project<2>(
    std::size_t n_bins_z,
    std::tuple<double, double> range_z
) const {
    std::tuple<double, double> my_range_z;
    std::tuple<double, double> default_range;
    if(range_z == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->asTuples());
        my_range_z = std::tuple<double, double>(std::get<4>(extremes), std::get<5>(extremes));
    }
    else {
        my_range_z = range_z;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_z, my_range_z));
    result->setXAxisLabel(this->zAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / z-projection");

    // Add data to the object
    for(auto const &v : this->template column<2>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * Specialization of projectW for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param n_bins_w The number of bins of the histogram
 * @param range_w The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the w-projection of the data
 */
template <>
template <>
inline std::shared_ptr<GDataCollectorT<double>>
GDataCollectorT<double, double, double, double>::project<3>(
    std::size_t n_bins_w,
    std::tuple<double, double> range_w
) const {
    std::tuple<double, double> my_range_w;
    std::tuple<double, double> default_range;
    if(range_w == default_range) {
        // Find out about the minimum and maximum values in the data
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->asTuples());
        my_range_w = std::tuple<double, double>(std::get<6>(extremes), std::get<7>(extremes));
    }
    else {
        my_range_w = range_w;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(new GHistogram1D(n_bins_w, my_range_w));
    result->setXAxisLabel("w");
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / w-projection");

    // Add data to the object
    for(auto const &v : this->template column<3>()) {
        (*result) & v;
    }

    // Return the data
    return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TPolyMarker3D class, intended for 4D data. The fourth
 * data component is represented as the size of the markers. The class will by
 * default only draw a selection of items. This results in a 3D plot.
 */
class GGraph4D : public GDataCollector4T<double, double, double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector4T_4double",
            boost::serialization::base_object<GDataCollector4T<double, double, double, double>>(
                *this
            )
        ) & BOOST_SERIALIZATION_NVP(min_marker_size_) &
            BOOST_SERIALIZATION_NVP(max_marker_size_) & BOOST_SERIALIZATION_NVP(small_w_large_marker_) &
            BOOST_SERIALIZATION_NVP(n_best_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GGraph4D() = default;
    GGraph4D(const GGraph4D &) = default;
    GGraph4D(GGraph4D &&) = default;
    ~GGraph4D() override = default;

    GGraph4D &operator=(GGraph4D const &) = default;
    GGraph4D &operator=(GGraph4D &&) = default;

    /*********************************************************************/

    /**
	 * @brief Allows to set the minimum marker size
	 * @param minMarkerSize The minimum marker size to be used when drawing the w-component
	 */
    void setMinMarkerSize(const double &min_marker_size);
    /**
	 * @brief Allows to set the maximum marker size
	 * @param maxMarkerSize The maximum marker size to be used when drawing the w-component
	 */
    void setMaxMarkerSize(const double &max_marker_size);

    /**
	 * @brief Allows to retrieve the minimum marker size
	 * @return The currently set minimum marker size
	 */
    double getMinMarkerSize() const;
    /**
	 * @brief Allows to retrieve the maximum marker size
	 * @return The currently set maximum marker size
	 */
    double getMaxMarkerSize() const;

    /**
	 * @brief Allows to specify whether small w yield large markers
	 * @param smallWLargeMarker If true, small w-values are mapped to large markers
	 */
    void setSmallWLargeMarker(const bool &swlm);
    /**
	 * @brief Allows to check whether small w yield large markers
	 * @return Whether small w-values are mapped to large markers
	 */
    bool getSmallWLargeMarker() const;

    /**
	 * @brief Allows to set the number of solutions the class should show
	 * @param nBest The number of (best) solutions to display; 0 means all
	 */
    void setNBest(const std::size_t &n_best);
    /**
	 * @brief Allows to retrieve the number of solutions the class should show
	 * @return The number of (best) solutions to display
	 */
    std::size_t getNBest() const;

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool isSecondary, std::size_t pId, std::size_t ownId, const std::string &indention) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool isSecondary, std::size_t pId, std::size_t ownId, const std::string &indention) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool isSecondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("min_marker_size_", self.min_marker_size_),
            make_member("max_marker_size_", self.max_marker_size_),
            make_member("small_w_large_marker_", self.small_w_large_marker_),
            make_member("n_best_", self.n_best_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph4D>(GGraph4D const &, GGraph4D const &, GToken &);

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    double min_marker_size_ = DEFMINMARKERSIZE; ///< The minimum allowed size of the marker
    double max_marker_size_ = DEFMAXMARKERSIZE; ///< The maximum allowed size of the marker

    bool small_w_large_marker_ = true; ///< Indicates whether a small w value yields a large marker

    std::size_t n_best_ = 0; ///< Determines the number of items the class should show
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TF1 1d-function plotter.
 * TODO: Add ability to add markers!
 */
class GFunctionPlotter1D : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) &
            BOOST_SERIALIZATION_NVP(function_description_) & BOOST_SERIALIZATION_NVP(x_extremes_) &
            BOOST_SERIALIZATION_NVP(n_samples_x_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
	 * @brief The standard constructor
	 * @param fD A textual description of the 1-d function to be plotted (in ROOT TF1 syntax)
	 * @param xExtremes The minimum and maximum value of the x-axis, as a tuple
	 */
    GFunctionPlotter1D(const std::string &f_d, const std::tuple<double, double> &x_extremes);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    GFunctionPlotter1D(GFunctionPlotter1D const &) = default;
    GFunctionPlotter1D(GFunctionPlotter1D &&) = default;
    ~GFunctionPlotter1D() override = default;

    GFunctionPlotter1D &operator=(GFunctionPlotter1D const &) = default;
    GFunctionPlotter1D &operator=(GFunctionPlotter1D &&) = default;

    /*********************************************************************/

    /**
	 * @brief Allows to set the number of sampling points in x-direction
	 * @param nSamplesX The number of sampling points used to evaluate the function in x-direction
	 */
    void setNSamplesX(std::size_t n_samples_x);

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool isSecondary, std::size_t pId, std::size_t ownId, const std::string &indention) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("function_description_", self.function_description_),
            make_member("x_extremes_", self.x_extremes_),
            make_member("n_samples_x_", self.n_samples_x_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GFunctionPlotter1D>(
        GFunctionPlotter1D const &,
        GFunctionPlotter1D const &,
        GToken &
    );

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    GFunctionPlotter1D() =
        default; ///< The default constructor. Intentionally private, as it is only needed for (de-)serialization

    std::string function_description_; ///< A textual description of the function to be plotted

    std::tuple<double, double> x_extremes_; ///< Minimum and maximum values for the x-axis
    std::size_t n_samples_x_ = DEFNSAMPLES;  ///< The number of sampling points of the function
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TF2 2d-function plotter
 */
class GFunctionPlotter2D : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) &
            BOOST_SERIALIZATION_NVP(function_description_) & BOOST_SERIALIZATION_NVP(x_extremes_) &
            BOOST_SERIALIZATION_NVP(y_extremes_) & BOOST_SERIALIZATION_NVP(n_samples_x_) &
            BOOST_SERIALIZATION_NVP(n_samples_y_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
	 * @brief The standard constructor
	 * @param fD A textual description of the 2-d function to be plotted (in ROOT TF2 syntax)
	 * @param xExtremes The minimum and maximum value of the x-axis, as a tuple
	 * @param yExtremes The minimum and maximum value of the y-axis, as a tuple
	 */
    GFunctionPlotter2D(
        const std::string &f_d,
        const std::tuple<double, double> &x_extremes,
        const std::tuple<double, double> &y_extremes
    );

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    GFunctionPlotter2D(GFunctionPlotter2D const &) = default;
    GFunctionPlotter2D(GFunctionPlotter2D &&) = default;
    ~GFunctionPlotter2D() override = default;

    GFunctionPlotter2D &operator=(GFunctionPlotter2D const &) = default;
    GFunctionPlotter2D &operator=(GFunctionPlotter2D &&) = default;

    /*********************************************************************/

    /**
	 * @brief Allows to set the number of sampling points in x-direction
	 * @param nSamplesX The number of sampling points used to evaluate the function in x-direction
	 */
    void setNSamplesX(std::size_t n_samples_x);
    /**
	 * @brief Allows to set the number of sampling points in y-direction
	 * @param nSamplesY The number of sampling points used to evaluate the function in y-direction
	 */
    void setNSamplesY(std::size_t n_samples_y);

    /**
	 * @brief Retrieves a unique name for this plotter
	 * @return A unique name for this plotter
	 */
    std::string getPlotterName() const override;

protected:
    /**
	 * @brief Retrieve specific header settings for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The header section of the ROOT code for this plot
	 */
    std::string headerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool isSecondary, std::size_t pId, std::size_t ownId, const std::string &indention) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, std::size_t own_id, const std::string &indent) const override;

    /**
	 * @brief Retrieve the current drawing arguments
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @return The drawing arguments to be passed to ROOT's Draw() call
	 */
    std::string drawingArguments(bool is_secondary) const override;

    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            make_member("function_description_", self.function_description_),
            make_member("x_extremes_", self.x_extremes_),
            make_member("y_extremes_", self.y_extremes_),
            make_member("n_samples_x_", self.n_samples_x_),
            make_member("n_samples_y_", self.n_samples_y_)
        );
    }

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GFunctionPlotter2D>(
        GFunctionPlotter2D const &,
        GFunctionPlotter2D const &,
        GToken &
    );

    /**
	 * @brief Searches for compliance with expectations with respect to another object of the same type
	 * @param cp A constant reference to another object, camouflaged as a GBasePlotter
	 * @param e The expectation for this object (e.g. equality)
	 * @param limit The maximum allowed deviation for floating point comparisons
	 */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

private:
    /**
	 * @brief Returns the name of this class
	 * @return The name of this class as a string
	 */
    std::string name_() const override;
    /**
	 * @brief Creates a deep clone of this object
	 * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	 */
    GBasePlotter *clone_() const override;

    GFunctionPlotter2D() =
        default; ///< The default constructor -- intentionally private, as it is only needed for (de-)serialization

    std::string function_description_; ///< A textual description of the function to be plotted

    std::tuple<double, double> x_extremes_; ///< Minimum and maximum values for the x-axis
    std::tuple<double, double> y_extremes_; ///< Minimum and maximum values for the y-axis

    std::size_t n_samples_x_ = DEFNSAMPLES; ///< The number of sampling points of the function
    std::size_t n_samples_y_ = DEFNSAMPLES; ///< The number of sampling points of the function
};


} /* namespace Gem::Common */
