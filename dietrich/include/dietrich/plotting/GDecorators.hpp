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

#include "common/GBoilerplateT.hpp"
#include "dietrich/plotting/GPlotEnums.hpp"

namespace Gem::Dietrich {


/******************************************************************************/
/**
 * This is the base class of a hierarchy of "decorator" classes that allow to
 * add features like markers, lines or text to plots. Plotters simply create a
 * container class of decorators, which in turn emit the code necessary to add the
 * desired decorations to the plots. Decorators (and their containers) are ordered
 * according to the plot dimension which they are supposed to cater for. The
 * dimension is provided as a template argument, so that it is not possible
 * to accidentally "mix" decorators for different dimensions. "Storage" of the
 * dimension (in the form of a compile-time template argument) is the main
 * purpose of this class. NOTE: As different access functions are needed for different
 * dimensions, some code duplication is unavoidable. C++ does not allow to add
 * "just" an additional function to a template specialization, unfortunately.
 *
 * @tparam dim The plot dimension (e.g. dimensions::Dim2, dimensions::Dim3) this decorator caters for
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <dimensions dim, Gem::Common::arithmetic coordinate_type>
class GDecorator { /* nothing */
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is the specialization of GDecorator for 2D-plots (e.g. histograms, graphs, ...)
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GDecorator<dimensions::Dim2, coordinate_type>
  : public Gem::Common::GBoilerplateBaseT<
        GDecorator<dimensions::Dim2, coordinate_type>,
        GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>>
    > {
    ///////////////////////////////////////////////////////////////////////
    // GBoilerplateAccess lets the GBoilerplateBaseT base reach this class's
    // (empty) localMembers_(); this abstract root is never Boost-constructed.
    friend struct Gem::Common::GBoilerplateAccess;

    /**
     * @brief This data-less base declares an *explicit* empty member list (a missing
     * one would inherit a parent's and is rejected at compile time by GBoilerplateBaseT).
     * @return An empty member tuple
     */
    template <typename Self>
    auto localMembers_(this Self &) {
        return std::make_tuple();
    }
    ///////////////////////////////////////////////////////////////////////

    // coordinate_type is constrained to be arithmetic via the Gem::Common::arithmetic
    // concept on the template parameter (clearer diagnostics than the former static_assert).

public:
    /** @brief The class name, consumed by the GBoilerplateBaseT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GDecorator<Dim2, coordinate_type>";

    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GDecorator() = default;
    GDecorator(GDecorator<dimensions::Dim2, coordinate_type> const &cp) = default;
    GDecorator(GDecorator<dimensions::Dim2, coordinate_type> &&cp) noexcept = default;
    ~GDecorator() override = default;

    GDecorator<dimensions::Dim2, coordinate_type> &
    operator=(GDecorator<dimensions::Dim2, coordinate_type> const &) = default;
    GDecorator<dimensions::Dim2, coordinate_type> &
    operator=(GDecorator<dimensions::Dim2, coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data. Plot boundaries are not taken into account.
	  *
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @param pos A running index that disambiguates the names of the generated plot objects
	  * @return A string holding the plotting code that renders this decoration
	  */
    [[nodiscard]] virtual std::string
    decoratorData(const std::string &indent, const std::size_t &pos) const = 0;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  *
	  * @param x_axis_range A (min, max) tuple delimiting the plot range along the x-axis
	  * @param y_axis_range A (min, max) tuple delimiting the plot range along the y-axis
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @param pos A running index that disambiguates the names of the generated plot objects
	  * @return A string holding the plotting code that renders this decoration within the boundaries
	  */
    [[nodiscard]] virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::string &indent,
        const std::size_t &pos
    ) const = 0;

    // load_(), compare_(), name_() and clone_() are generated by the
    // Gem::Common::GBoilerplateBaseT base (clone_ stays pure -- this is abstract)
    // from class_name and the empty localMembers_() declaration above.

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to add markers of different types to a plot. Note that this class
 * may only be used for 2D-plots.
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GMarker
  : public Gem::Common::GBoilerplateT<
        GMarker<coordinate_type>,
        GDecorator<dimensions::Dim2, coordinate_type>
        // CloneReturn defaults to the hierarchy root (GDecorator<Dim2>): a covariant
        // return to the CRTP-self would need GMarker complete at the base's clone_
        // declaration, which it is not. clone_ is private, so the narrower return is
        // not observable and this is purely a formality.
    > {
    ///////////////////////////////////////////////////////////////////////
    // boost::serialization::access default-constructs this concrete type on load;
    // GBoilerplateAccess lets the GBoilerplateT base reach this class's localMembers_().
    friend class boost::serialization::access;
    friend struct Gem::Common::GBoilerplateAccess;

public:
    /** @brief The class name, consumed by the GBoilerplateT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GMarker<coordinate_type>";

    /***************************************************************************/
    /**
	  * @brief The standard constructor, which takes all essential data for this
	  * decorator type.
	  *
	  * @param coordinates The (x, y) position at which the marker is drawn
	  * @param marker The type/shape of the marker to be drawn (e.g. a closed circle)
	  * @param color The color of the marker
	  * @param size The size of the marker
	  */
    GMarker(
        const std::tuple<coordinate_type, coordinate_type> &coordinates,
        const gMarker &marker,
        const gColor &color,
        const double &size
    )
      : coordinates_(coordinates)
      , marker_(marker)
      , color_(color)
      , size_(size) { /* nothing */
    }

    /***************************************************************************/
    // Defaulted constructos, destructor and assignment operators. The default-
    // constructor is private, as it is only needed for (de)serialization.

    GMarker(GMarker<coordinate_type> const &cp) = default;
    GMarker(GMarker<coordinate_type> &&cp) noexcept = default;
    ~GMarker() override = default;

    GMarker<coordinate_type> &operator=(GMarker<coordinate_type> const &) = default;
    GMarker<coordinate_type> &operator=(GMarker<coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data. Plot boundaries are not taken into account.
	  *
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @param pos A running index that disambiguates the names of the generated plot objects
	  * @return A string holding the plotting code that draws this marker
	  */
    [[nodiscard]] std::string decoratorData(const std::string &indent, const std::size_t &pos) const override {
        std::ostringstream data; // NOLINT(cppcoreguidelines-init-variables)

        data << indent << "TMarker * tm_" << pos << " = new TMarker("
             << Gem::Common::narrow<double>(std::get<0>(coordinates_)) << ", "
             << Gem::Common::narrow<double>(std::get<1>(coordinates_)) << ", " << marker_ << ");"
             << '\n'
             << indent << "tm_" << pos << "->SetMarkerColor(" << color_ << ");" << '\n'
             << indent << "tm_" << pos << "->SetMarkerSize(" << size_ << ");" << '\n'
             << indent << "tm_" << pos << "->Draw();" << '\n'
             << '\n';

        return data.str();
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data. Plot boundaries are taken into account.
	  *
	  * The marker is drawn only when its coordinates lie inside the supplied axis
	  * ranges (clipping); an out-of-range marker yields an empty string.
	  *
	  * @param x_axis_range A (min, max) tuple delimiting the plot range along the x-axis
	  * @param y_axis_range A (min, max) tuple delimiting the plot range along the y-axis
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @param pos A running index that disambiguates the names of the generated plot objects
	  * @return Plotting code for the marker, or an empty string if it falls outside the boundaries
	  */
    [[nodiscard]] std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::string &indent,
        const std::size_t &pos
    ) const override {
        coordinate_type marker_x = std::get<0>(coordinates_);
        coordinate_type marker_y = std::get<1>(coordinates_);
        coordinate_type x_min = std::get<0>(x_axis_range);
        coordinate_type x_max = std::get<1>(x_axis_range);
        coordinate_type y_min = std::get<0>(y_axis_range);
        coordinate_type y_max = std::get<1>(y_axis_range);

        // Clip: a marker outside the axis ranges is not drawn
        if(marker_x < x_min || marker_x > x_max || marker_y < y_min || marker_y > y_max) {
            return {};
        }
        return this->decoratorData(indent, pos);
    }

protected:
    /***************************************************************************/
    /**
     * @brief The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     *
     * @return A tuple of named handles to this object's local data members
     */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(
            make_member("coordinates_", self.coordinates_),
            make_member("marker_", self.marker_),
            make_member("color_", self.color_),
            make_member("size_", self.size_)
        );
    }

    // load_(), compare_(), name_() and clone_() are generated by the
    // Gem::Common::GBoilerplateT base from class_name and localMembers_().

    /***************************************************************************/
    /** @brief Applies test-only modifications: this marker's coordinates and size. Never invoked on
     *  rendered objects. */
    bool modify_GUnitTests_() override {
        coordinates_ = std::tuple<coordinate_type, coordinate_type>(
            static_cast<coordinate_type>(1), static_cast<coordinate_type>(2)
        );
        size_ += 1.0;
        return true;
    }
    /** @brief Performs self-tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self-tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /***************************************************************************/
    /**
	  * @brief The default constructor -- intentionally private, as it is only needed
	  * for de-serialization.
	  */
    GMarker() = default;

    /***************************************************************************/
    // Local data ...

    std::tuple<coordinate_type, coordinate_type> coordinates_; ///< The coordinates of the marker

    gMarker marker_ = gMarker::closedCircle; ///< Denotes the type of markers to be drawn
    gColor color_ = gColor::black;           ///< The color of the marker
    double size_ = 0.05;                     ///< The size of the marker
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is the specialization of GDecorator for 3D-plots (e.g. 2D-histograms, 3D-graphs, ...)
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GDecorator<dimensions::Dim3, coordinate_type>
  : public Gem::Common::GBoilerplateBaseT<
        GDecorator<dimensions::Dim3, coordinate_type>,
        GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>>
    > {
    ///////////////////////////////////////////////////////////////////////
    // GBoilerplateAccess lets the GBoilerplateBaseT base reach this class's
    // (empty) localMembers_(); this abstract root is never Boost-constructed.
    friend struct Gem::Common::GBoilerplateAccess;

    /**
     * @brief This data-less base declares an *explicit* empty member list (a missing
     * one would inherit a parent's and is rejected at compile time by GBoilerplateBaseT).
     * @return An empty member tuple
     */
    template <typename Self>
    auto localMembers_(this Self &) {
        return std::make_tuple();
    }
    ///////////////////////////////////////////////////////////////////////

    // coordinate_type is constrained to be arithmetic via the Gem::Common::arithmetic
    // concept on the template parameter (clearer diagnostics than the former static_assert).

public:
    /** @brief The class name, consumed by the GBoilerplateBaseT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GDecorator<dimensions::Dim3, coordinate_type>";

    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators.

    GDecorator() = default;
    GDecorator(GDecorator<dimensions::Dim3, coordinate_type> const &cp) = default;
    GDecorator(GDecorator<dimensions::Dim3, coordinate_type> &&cp) noexcept = default;
    ~GDecorator() override = default;

    GDecorator<dimensions::Dim3, coordinate_type> &
    operator=(GDecorator<dimensions::Dim3, coordinate_type> const &) = default;
    GDecorator<dimensions::Dim3, coordinate_type> &
    operator=(GDecorator<dimensions::Dim3, coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data. Plot boundaries are not taken into account.
	  *
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @param pos A running index that disambiguates the names of the generated plot objects
	  * @return A string holding the plotting code that renders this decoration
	  */
    [[nodiscard]] virtual std::string decoratorData(const std::string &indent, const std::size_t &pos) const = 0;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  *
	  * @param x_axis_range A (min, max) tuple delimiting the plot range along the x-axis
	  * @param y_axis_range A (min, max) tuple delimiting the plot range along the y-axis
	  * @param z_axis_range A (min, max) tuple delimiting the plot range along the z-axis
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @param pos A running index that disambiguates the names of the generated plot objects
	  * @return A string holding the plotting code that renders this decoration within the boundaries
	  */
    [[nodiscard]] virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &z_axis_range,
        const std::string &indent,
        const std::size_t &pos
    ) const = 0;

    // load_(), compare_(), name_() and clone_() are generated by the
    // Gem::Common::GBoilerplateBaseT base (clone_ stays pure -- this is abstract)
    // from class_name and the empty localMembers_() declaration above.

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class acts as a container of decorator objects. In its specializations,
 * it is derived from GPtrContainerT and may thus be treated like a
 * std::vector of std::shared_ptr<GDecorator<dim>> . Note that the actual work
 * is done in the specializations for different dimensions. Hence some code
 * duplications for the different template specializations cannot be avoided.
 *
 * @tparam dim The plot dimension (e.g. dimensions::Dim2, dimensions::Dim3) of the held decorators
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <dimensions dim, Gem::Common::arithmetic coordinate_type>
class GDecoratorContainer { /* nothing */
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GDecoratorContainer for 2D-plots
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GDecoratorContainer<dimensions::Dim2, coordinate_type>
  : public GCommonInterfaceT<GDecoratorContainer<dimensions::Dim2, coordinate_type>>
  , public GPtrContainerT<GDecorator<dimensions::Dim2, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this container (forwarding to the GPtrContainerT base) via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize the base container to / from
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GStdPtrVectorInterfaceT_GDecorator2",
            boost::serialization::base_object<
                GPtrContainerT<GDecorator<dimensions::Dim2, coordinate_type>>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

    // coordinate_type is constrained to be arithmetic via the Gem::Common::arithmetic
    // concept on the template parameter (clearer diagnostics than the former static_assert).

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GDecoratorContainer() = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim2, coordinate_type> const &cp) = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim2, coordinate_type> &&cp) noexcept =
        default;

    GDecoratorContainer<dimensions::Dim2, coordinate_type> &
    operator=(GDecoratorContainer<dimensions::Dim2, coordinate_type> const &) = default;
    GDecoratorContainer<dimensions::Dim2, coordinate_type> &
    operator=(GDecoratorContainer<dimensions::Dim2, coordinate_type> &&) noexcept = default;

    ~GDecoratorContainer() override = default;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data of all decorators. Plot boundaries are
	  * not taken into account.
	  *
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @return The concatenated plotting code of all contained decorators
	  */
    [[nodiscard]] virtual std::string decoratorData(const std::string &indent) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t pos = 0;
        for(auto const &decorator_ptr : *this) {
            result += decorator_ptr->decoratorData(indent, pos++);
        }

        return result;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data of all decorators, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  *
	  * @param x_axis_range A (min, max) tuple delimiting the plot range along the x-axis
	  * @param y_axis_range A (min, max) tuple delimiting the plot range along the y-axis
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @return The concatenated plotting code of all contained decorators, clipped to the boundaries
	  */
    [[nodiscard]] virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::string &indent
    ) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t pos = 0;
        for(auto const &decorator_ptr : *this) {
            result += decorator_ptr->decoratorData(x_axis_range, y_axis_range, indent, pos++);
        }

        return result;
    }

protected:
    /***************************************************************************/
    /**
	  * @brief Loads the data of another object
	  *
	  * @param cp A pointer to another GDecoratorContainer object, camouflaged as the base type
	  */
    void load_(const GDecoratorContainer<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent data ...
        GPtrContainerT<GDecorator<dimensions::Dim2, coordinate_type>>::operator=(*p_load);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(
        GDecoratorContainer<dimensions::Dim2, coordinate_type> const &,
        GDecoratorContainer<dimensions::Dim2, coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * @brief Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another object of the same type
	  * @param e The expectation for the comparison (e.g. equality or inequality)
	  * @param limit The maximum allowed deviation for comparisons of floating point types
	  */
    void compare_(
        const GDecoratorContainer<dimensions::Dim2, coordinate_type> &cp // the other object
        ,
        const expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDecoratorContainer<dimensions::Dim2>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<GDecoratorContainer<dimensions::Dim2, coordinate_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then the local data. Actually this allows us to compare
        // the second parent class without directly calling it.
        compare_t(Gem::Common::getIdentity(this->data_cnt_, p_load->data_cnt_, "this->data_cnt_", "p_load->data_cnt_"), token);

        // React on deviations from the expectation
        token.evaluate();
    }

private:
    /***************************************************************************/
    /**
	  * @brief Returns the name of this class
	  *
	  * @return The mnemonic name of this class
	  */
    [[nodiscard]] std::string name_() const override {
        return std::string("GDecoratorContainer<dimensions::Dim2, coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object.
	  *
	  * @return A deep clone of this object, allocated on the heap
	  */
    [[nodiscard]] GDecoratorContainer<dimensions::Dim2, coordinate_type> *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization for 2D decorators
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GDecoratorContainer_2D
  : public Gem::Common::GBoilerplateT<
        GDecoratorContainer_2D<coordinate_type>,
        GDecoratorContainer<dimensions::Dim2, coordinate_type>
    > {
    ///////////////////////////////////////////////////////////////////////
    // boost::serialization::access default-constructs this concrete type on load;
    // GBoilerplateAccess lets the GBoilerplateT base reach this class's (empty) localMembers_().
    friend class boost::serialization::access;
    friend struct Gem::Common::GBoilerplateAccess;

    /**
	  * @brief This wrapper adds no own members; the *explicit* empty declaration is
	  * required (a missing one would inherit the parent's and is rejected at compile
	  * time by GBoilerplateT).
	  * @return An empty member tuple
	  */
    template <typename Self>
    auto localMembers_(this Self &) {
        return std::make_tuple();
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The class name, consumed by the GBoilerplateT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GDecoratorContainer_2D<coordinate_type>";

    // Relay to the parent class'es constructors (propagated down through the mixin's inherited ctors).
    using Gem::Common::GBoilerplateT<
        GDecoratorContainer_2D<coordinate_type>,
        GDecoratorContainer<dimensions::Dim2, coordinate_type>
    >::GBoilerplateT;

    // load_(), compare_(), name_() and clone_() are generated by the
    // Gem::Common::GBoilerplateT base from class_name and the empty localMembers_() declaration above.

protected:
    /***************************************************************************/
    /** @brief Applies test-only modifications: appends a marker so the decorator list is non-empty.
     *  Never invoked on rendered objects. */
    bool modify_GUnitTests_() override {
        this->push_back(std::make_shared<GMarker<coordinate_type>>(
            std::tuple<coordinate_type, coordinate_type>(
                static_cast<coordinate_type>(1), static_cast<coordinate_type>(2)
            ),
            gMarker::closedCircle,
            gColor::black,
            0.1
        ));
        return true;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GDecoratorContainer for 3D-plots
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GDecoratorContainer<dimensions::Dim3, coordinate_type>
  : public GCommonInterfaceT<GDecoratorContainer<dimensions::Dim3, coordinate_type>>
  , public GPtrContainerT<GDecorator<dimensions::Dim3, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this container (forwarding to the GPtrContainerT base) via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize the base container to / from
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GStdPtrVectorInterfaceT_GDecorator3",
            boost::serialization::base_object<
                GPtrContainerT<GDecorator<dimensions::Dim3, coordinate_type>>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

    // coordinate_type is constrained to be arithmetic via the Gem::Common::arithmetic
    // concept on the template parameter (clearer diagnostics than the former static_assert).

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GDecoratorContainer() = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim3, coordinate_type> const &cp) = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim3, coordinate_type> &&cp) noexcept =
        default;
    ~GDecoratorContainer() override = default;

    GDecoratorContainer<dimensions::Dim3, coordinate_type> &
    operator=(GDecoratorContainer<dimensions::Dim3, coordinate_type> const &) = default;
    GDecoratorContainer<dimensions::Dim3, coordinate_type> &
    operator=(GDecoratorContainer<dimensions::Dim3, coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data of all decorators. Plot boundaries are
	  * not taken into account.
	  *
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @return The concatenated plotting code of all contained decorators
	  */
    [[nodiscard]] virtual std::string decoratorData(const std::string &indent) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t pos = 0;
        for(auto const &decorator_ptr : *this) {
            result += decorator_ptr->decoratorData(indent, pos++);
        }

        return result;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the decorator data of all decorators, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  *
	  * @param x_axis_range A (min, max) tuple delimiting the plot range along the x-axis
	  * @param y_axis_range A (min, max) tuple delimiting the plot range along the y-axis
	  * @param z_axis_range A (min, max) tuple delimiting the plot range along the z-axis
	  * @param indent The leading whitespace prepended to each emitted line of plotting code
	  * @return The concatenated plotting code of all contained decorators, clipped to the boundaries
	  */
    [[nodiscard]] virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &z_axis_range,
        const std::string &indent
    ) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t pos = 0;
        for(auto const &decorator_ptr : *this) {
            result += decorator_ptr
                          ->decoratorData(x_axis_range, y_axis_range, z_axis_range, indent, pos++);
        }

        return result;
    }

protected:
    /***************************************************************************/
    /**
	  * @brief Loads the data of another object
	  *
	  * @param cp A pointer to another GDecoratorContainer object, camouflaged as the base type
	  */
    void load_(const GDecoratorContainer<dimensions::Dim3, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent data ...
        GPtrContainerT<GDecorator<dimensions::Dim3, coordinate_type>>::operator=(*p_load);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(
        GDecoratorContainer<dimensions::Dim3, coordinate_type> const &,
        GDecoratorContainer<dimensions::Dim3, coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * @brief Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another object of the same type
	  * @param e The expectation for the comparison (e.g. equality or inequality)
	  * @param limit The maximum allowed deviation for comparisons of floating point types
	  */
    void compare_(
        const GDecoratorContainer<dimensions::Dim3, coordinate_type> &cp // the other object
        ,
        const expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDecoratorContainer<dimensions::Dim3>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<GDecoratorContainer<dimensions::Dim3, coordinate_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then the local data. This allows us to compare
        // the second parent class without directly calling it.
        compare_t(Gem::Common::getIdentity(this->data_cnt_, p_load->data_cnt_, "this->data_cnt_", "p_load->data_cnt_"), token);

        // React on deviations from the expectation
        token.evaluate();
    }

private:
    /***************************************************************************/
    /**
	  * @brief Returns the name of this class
	  *
	  * @return The mnemonic name of this class
	  */
    [[nodiscard]] std::string name_() const override {
        return std::string("GDecoratorContainer<dimensions::Dim3, coordinate_type>");
    }

    /***************************************************************************/
    /**
	 * @brief Creates a deep clone of this object.
	 *
	 * @return A deep clone of this object, allocated on the heap
	 */
    [[nodiscard]] GDecoratorContainer<dimensions::Dim3, coordinate_type> *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization for 3D decorators
 *
 * @tparam coordinate_type The arithmetic type used for plot coordinates
 */
template <Gem::Common::arithmetic coordinate_type>
class GDecoratorContainer_3D
  : public Gem::Common::GBoilerplateT<
        GDecoratorContainer_3D<coordinate_type>,
        GDecoratorContainer<dimensions::Dim3, coordinate_type>
    > {
    ///////////////////////////////////////////////////////////////////////
    // boost::serialization::access default-constructs this concrete type on load;
    // GBoilerplateAccess lets the GBoilerplateT base reach this class's (empty) localMembers_().
    friend class boost::serialization::access;
    friend struct Gem::Common::GBoilerplateAccess;

    /**
	  * @brief This wrapper adds no own members; the *explicit* empty declaration is
	  * required (a missing one would inherit the parent's and is rejected at compile
	  * time by GBoilerplateT).
	  * @return An empty member tuple
	  */
    template <typename Self>
    auto localMembers_(this Self &) {
        return std::make_tuple();
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The class name, consumed by the GBoilerplateT-generated name_() / compare token. */
    static constexpr std::string_view class_name = "GDecoratorContainer_3D<coordinate_type>";

    // Relay to the parent class'es constructors (propagated down through the mixin's inherited ctors).
    using Gem::Common::GBoilerplateT<
        GDecoratorContainer_3D<coordinate_type>,
        GDecoratorContainer<dimensions::Dim3, coordinate_type>
    >::GBoilerplateT;

    // load_(), compare_(), name_() and clone_() are generated by the
    // Gem::Common::GBoilerplateT base from class_name and the empty localMembers_() declaration above.

protected:
    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////

} /* namespace Gem::Dietrich */
