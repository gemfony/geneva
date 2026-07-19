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
  : public GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this (data-less) base class via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize to / from (unused: no local data)
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize([[maybe_unused]] Archive & ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        /* nothing */
    }
    ///////////////////////////////////////////////////////////////////////

    // coordinate_type is constrained to be arithmetic via the Gem::Common::arithmetic
    // concept on the template parameter (clearer diagnostics than the former static_assert).

public:
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

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another object
     *
     * @param cp A pointer to another GDecorator object, camouflaged as the base type
     */
    void load_(const GDecorator<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // No local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDecorator<dimensions::Dim2, coordinate_type>>(
        GDecorator<dimensions::Dim2, coordinate_type> const &,
        GDecorator<dimensions::Dim2, coordinate_type> const &,
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
        const GDecorator<dimensions::Dim2, coordinate_type> &cp // the other object
        ,
        const expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDecorator<dimensions::Dim2, coordinate_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>>>(
            *this,
            *p_load,
            token
        );

        // ... no local data

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
        return std::string("GDecorator<Dim2, coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object (this function is purely virtual)
	  *
	  * @return A deep clone of this object, allocated on the heap
	  */
    [[nodiscard]] GDecorator<dimensions::Dim2, coordinate_type> *clone_() const override = 0;

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
class GMarker : public GDecorator<dimensions::Dim2, coordinate_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this class (and its base) via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize the base class and local members to / from
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            // The NVP tag becomes an XML element name in the XML archive, so it must be a valid
            // XML name: no angle brackets, commas or spaces (the former tag broke XML round-trips).
            "GDecorator2_Dim2",
            boost::serialization::base_object<GDecorator<dimensions::Dim2, coordinate_type>>(*this)
        );
        // ... and then our own data, derived from the single localMembers_() declaration
        Gem::Common::serialize_members(ar, this->localMembers_());
    }
    ///////////////////////////////////////////////////////////////////////

public:
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

    /***************************************************************************/
    /**
	  * @brief Loads the data of another object
	  *
	  * @param cp A pointer to another GMarker object, camouflaged as the base type
	  */
    void load_(const GDecorator<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GMarker reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent data ...
        GDecorator<dimensions::Dim2, coordinate_type>::load_(cp);

        // ... and then our local data, derived from the single localMembers() declaration
        g_load_members(this->localMembers_(), p_load->localMembers_());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GMarker<coordinate_type>>(
        GMarker<coordinate_type> const &,
        GMarker<coordinate_type> const &,
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
        const GDecorator<dimensions::Dim2, coordinate_type> &cp // the other object
        ,
        const expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GMarker reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GMarker<coordinate_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GDecorator<dimensions::Dim2, coordinate_type>>(*this, *p_load, token);

        // ... and then our local data, derived from the single localMembers() declaration
        g_compare_members(this->localMembers_(), p_load->localMembers_(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

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
	  * @brief Returns the name of this class
	  *
	  * @return The mnemonic name of this class
	  */
    [[nodiscard]] std::string name_() const override {
        return std::string("GMarker<coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object.
	  *
	  * @return A deep clone of this object, allocated on the heap
	  */
    [[nodiscard]] GMarker<coordinate_type> *clone_() const override {
        return new GMarker<coordinate_type>(*this);
    }

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
  : public GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this (data-less) base class via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize to / from (unused: no local data)
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize([[maybe_unused]] Archive & ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        // nothing
    }
    ///////////////////////////////////////////////////////////////////////

    // coordinate_type is constrained to be arithmetic via the Gem::Common::arithmetic
    // concept on the template parameter (clearer diagnostics than the former static_assert).

public:
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

protected:
    /***************************************************************************/
    /**
	  * @brief Loads the data of another object
	  *
	  * @param cp A pointer to another GDecorator object, camouflaged as the base type
	  */
    void load_(const GDecorator<dimensions::Dim3, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // No local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDecorator<dimensions::Dim3, coordinate_type>>(
        GDecorator<dimensions::Dim3, coordinate_type> const &,
        GDecorator<dimensions::Dim3, coordinate_type> const &,
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
        const GDecorator<dimensions::Dim3, coordinate_type> &cp // the other object
        ,
        const expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDecorator<dimensions::Dim3, coordinate_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>>>(
            *this,
            *p_load,
            token
        );

        // ... no local data

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
        return std::string("GDecorator<dimensions::Dim3, coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object (this function is purely virtual)
	  *
	  * @return A deep clone of this object, allocated on the heap
	  */
    [[nodiscard]] GDecorator<dimensions::Dim3, coordinate_type> *clone_() const override = 0;

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
class GDecoratorContainer_2D : public GDecoratorContainer<dimensions::Dim2, coordinate_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this container (forwarding to its base) via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize the base container to / from
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDecoratorContainer_2D",
            boost::serialization::base_object<
                GDecoratorContainer<dimensions::Dim2, coordinate_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    // Relay to the parent class'es constructors
    using GDecoratorContainer<dimensions::Dim2, coordinate_type>::GDecoratorContainer;

protected:
    /***************************************************************************/
    /**
	  * @brief Loads the data of another object
	  *
	  * @param cp A pointer to another GDecoratorContainer_2D object, camouflaged as the base type
	  */
    void load_(const GDecoratorContainer<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecoratorContainer_2D reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent data ...
        GDecoratorContainer<dimensions::Dim2, coordinate_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDecoratorContainer_2D<coordinate_type>>(
        GDecoratorContainer_2D<coordinate_type> const &,
        GDecoratorContainer_2D<coordinate_type> const &,
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

        GToken token("GDecoratorContainer_2D<dimensions::Dim2>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(
            *this,
            *p_load,
            token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

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

private:
    /***************************************************************************/
    /**
	 * @brief Returns the name of this class
	 *
	 * @return The mnemonic name of this class
	 */
    [[nodiscard]] std::string name_() const override {
        return std::string("GDecoratorContainer_2D<coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object.
	  *
	  * @return A deep clone of this object, allocated on the heap
	  */
    [[nodiscard]] GDecoratorContainer<dimensions::Dim2, coordinate_type> *clone_() const override {
        return new GDecoratorContainer_2D<coordinate_type>(*this);
    }

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
class GDecoratorContainer_3D : public GDecoratorContainer<dimensions::Dim3, coordinate_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * Serializes this container (forwarding to its base) via Boost.Serialization.
     *
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive to serialize the base container to / from
     * @param version The serialization version (unused)
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDecoratorContainer_Dim3",
            boost::serialization::base_object<
                GDecoratorContainer<dimensions::Dim3, coordinate_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    // Relay to the parent class'es constructors
    using GDecoratorContainer<dimensions::Dim3, coordinate_type>::GDecoratorContainer;

protected:
    /***************************************************************************/
    /**
	  * @brief Loads the data of another object
	  *
	  * @param cp A pointer to another GDecoratorContainer_3D object, camouflaged as the base type
	  */
    void load_(const GDecoratorContainer<dimensions::Dim3, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecoratorContainer_3D reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent data ...
        GDecoratorContainer<dimensions::Dim3, coordinate_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GDecoratorContainer_3D<coordinate_type>>(
        GDecoratorContainer_3D<coordinate_type> const &,
        GDecoratorContainer_3D<coordinate_type> const &,
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

        GToken token("GDecoratorContainer_3D<dimensions::Dim3>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(
            *this,
            *p_load,
            token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /***************************************************************************/
    /**
	 * @brief Returns the name of this class
	 *
	 * @return The mnemonic name of this class
	 */
    [[nodiscard]] std::string name_() const override {
        return std::string("GDecoratorContainer_3D<coordinate_type>");
    }

    /***************************************************************************/
    /**
	 * @brief Creates a deep clone of this object.
	 *
	 * @return A deep clone of this object, allocated on the heap
	 */
    [[nodiscard]] GDecoratorContainer<dimensions::Dim3, coordinate_type> *clone_() const override {
        return new GDecoratorContainer_3D<coordinate_type>(*this);
    }

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////

} /* namespace Gem::Dietrich */
