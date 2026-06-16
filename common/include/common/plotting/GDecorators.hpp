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

#include "common/plotting/GPlotEnums.hpp"

namespace Gem::Common {

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
 */
template <dimensions dim, typename coordinate_type>
class GDecorator { /* nothing */
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is the specialization of GDecorator for 2D-plots (e.g. histograms, graphs, ...)
 */
template <typename coordinate_type>
class GDecorator<dimensions::Dim2, coordinate_type>
  : public GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize([[maybe_unused]] Archive & ar, const unsigned int) {
        using boost::serialization::make_nvp;

        /* nothing */
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic_v<coordinate_type>,
        "coordinate_type should either be a floating-point or an integer type"
    );

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
	  * Retrieves the decorator data. Plot boundaries are not taken into account.
	  */
    virtual std::string
    decoratorData(const std::string &, const std::size_t &) const = 0;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  */
    virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::string &indent,
        const std::size_t &
    ) const = 0;

protected:
    /***************************************************************************/
    /**
     * Loads the data of another object
     */
    void load_(const GDecorator<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // No local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecorator<dimensions::Dim2, coordinate_type>>(
        GDecorator<dimensions::Dim2, coordinate_type> const &,
        GDecorator<dimensions::Dim2, coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
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
        compare_base_t<GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>>>(
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
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDecorator<Dim2, coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object (this function is purely virtual)
	  */
    GDecorator<dimensions::Dim2, coordinate_type> *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Allows to add markers of different types to a plot. Note that this class
 * may only be used for 2D-plots.
 */
template <typename coordinate_type>
class GMarker : public GDecorator<dimensions::Dim2, coordinate_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDecorator2<dimensions::Dim2, coordinate_type>",
            boost::serialization::base_object<GDecorator<dimensions::Dim2, coordinate_type>>(*this)
        ) & BOOST_SERIALIZATION_NVP(coordinates_) &
            BOOST_SERIALIZATION_NVP(marker_) & BOOST_SERIALIZATION_NVP(color_) &
            BOOST_SERIALIZATION_NVP(size_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * The standard constructor, which takes all essential data for this
	  * decorator type.
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
	  * Retrieves the decorator data. Plot boundaries are not taken into account.
	  */
    std::string decoratorData(const std::string &indent, const std::size_t &pos) const override {
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
	  * Retrieves the decorator data. Plot boundaries are taken into account.
	  */
    std::string decoratorData(
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

        // Check if our coordinates are inside of the axis range
        if(marker_x < x_min || marker_x > x_max || marker_y < y_min || marker_y > y_max) {
            return this->decoratorData(indent, pos);
        }
                    return {};
       
    }

protected:
    /***************************************************************************/
    /**
     * The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     */
    auto localMembers() {
        return std::make_tuple(
            make_member("coordinates_", coordinates_),
            make_member("marker_", marker_),
            make_member("color_", color_),
            make_member("size_", size_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            make_member("coordinates_", coordinates_),
            make_member("marker_", marker_),
            make_member("color_", color_),
            make_member("size_", size_)
        );
    }

    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GDecorator<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GMarker reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent data ...
        GDecorator<dimensions::Dim2, coordinate_type>::load_(cp);

        // ... and then our local data, derived from the single localMembers() declaration
        g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GMarker<coordinate_type>>(
        GMarker<coordinate_type> const &,
        GMarker<coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
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
        compare_base_t<GDecorator<dimensions::Dim2, coordinate_type>>(*this, *p_load, token);

        // ... and then our local data, derived from the single localMembers() declaration
        g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    }
    /** @brief Performs self-tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self-tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GMarker<coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object.
	  */
    GMarker<coordinate_type> *clone_() const override {
        return new GMarker<coordinate_type>(*this);
    }

    /***************************************************************************/
    /**
	  * The default constructor -- intentionally private, as it is only needed
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
 */
template <typename coordinate_type>
class GDecorator<dimensions::Dim3, coordinate_type>
  : public GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize([[maybe_unused]] Archive & ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // nothing
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic_v<coordinate_type>,
        "coordinate_type should either be a floating-point or an integer type"
    );

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
	  * Retrieves the decorator data. Plot boundaries are not taken into account.
	  */
    virtual std::string decoratorData(const std::string &, const std::size_t &) const = 0;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  */
    virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &y_axis_range,
        const std::tuple<coordinate_type, coordinate_type> &z_axis_range,
        const std::string &indent,
        const std::size_t &pos
    ) const = 0;

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GDecorator<dimensions::Dim3, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // No local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecorator<dimensions::Dim3, coordinate_type>>(
        GDecorator<dimensions::Dim3, coordinate_type> const &,
        GDecorator<dimensions::Dim3, coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
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
        compare_base_t<GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>>>(
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
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDecorator<imensions::Dim3, coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object (this function is purely virtual)
	  */
    GDecorator<dimensions::Dim3, coordinate_type> *clone_() const override = 0;

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
 */
template <dimensions dim, typename coordinate_type>
class GDecoratorContainer { /* nothing */
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GDecoratorContainer for 2D-plots
 */
template <typename coordinate_type>
class GDecoratorContainer<dimensions::Dim2, coordinate_type>
  : public GCommonInterfaceT<GDecoratorContainer<dimensions::Dim2, coordinate_type>>
  , public GPtrContainerT<GDecorator<dimensions::Dim2, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GStdPtrVectorInterfaceT_GDecorator2",
            boost::serialization::base_object<
                GPtrContainerT<GDecorator<dimensions::Dim2, coordinate_type>>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic_v<coordinate_type>,
        "coordinate_type should either be a floating-point or an integer type"
    );

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
	  * Retrieves the decorator data of all decorators. Plot boundaries are
	  * not taken into account.
	  */
    virtual std::string decoratorData(const std::string &indent) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t pos = 0;
        for(auto const &decorator_ptr : *this) {
            result += decorator_ptr->decoratorData(indent, pos++);
        }

        return result;
    }

    /***************************************************************************/
    /**
	  * Retrieves the decorator data of all decorators, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  */
    virtual std::string decoratorData(
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
	  * Loads the data of another object
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
    friend void compare_base_t<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(
        GDecoratorContainer<dimensions::Dim2, coordinate_type> const &,
        GDecoratorContainer<dimensions::Dim2, coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
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
        compare_base_t<GCommonInterfaceT<GDecoratorContainer<dimensions::Dim2, coordinate_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then the local data. Actually this allows us to compare
        // the second parent class without directly calling it.
        compare_t(IDENTITY(this->data_cnt_, p_load->data_cnt_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDecoratorContainer<dimensions::Dim2, coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object.
	  */
    GDecoratorContainer<dimensions::Dim2, coordinate_type> *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization for 2D decorators
 */
template <typename coordinate_type>
class GDecoratorContainer_2D : public GDecoratorContainer<dimensions::Dim2, coordinate_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
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
	  * Loads the data of another object
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
    friend void compare_base_t<GDecoratorContainer_2D<coordinate_type>>(
        GDecoratorContainer_2D<coordinate_type> const &,
        GDecoratorContainer_2D<coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
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
        compare_base_t<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(
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
	 * Returns the name of this class
	 */
    std::string name_() const override {
        return std::string("GDecoratorContainer_2D<coordinate_type>");
    }

    /***************************************************************************/
    /**
	  * Creates a deep clone of this object.
	  */
    GDecoratorContainer<dimensions::Dim2, coordinate_type> *clone_() const override {
        return new GDecoratorContainer_2D<coordinate_type>(*this);
    }

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GDecoratorContainer for 3D-plots
 */
template <typename coordinate_type>
class GDecoratorContainer<dimensions::Dim3, coordinate_type>
  : public GCommonInterfaceT<GDecoratorContainer<dimensions::Dim3, coordinate_type>>
  , public GPtrContainerT<GDecorator<dimensions::Dim3, coordinate_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GStdPtrVectorInterfaceT_GDecorator3",
            boost::serialization::base_object<
                GPtrContainerT<GDecorator<dimensions::Dim3, coordinate_type>>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic_v<coordinate_type>,
        "coordinate_type should either be a floating-point or an integer type"
    );

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
	  * Retrieves the decorator data of all decorators. Plot boundaries are
	  * not taken into account.
	  */
    virtual std::string decoratorData(const std::string &indent) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t pos = 0;
        for(auto const &decorator_ptr : *this) {
            result += decorator_ptr->decoratorData(indent, pos++);
        }

        return result;
    }

    /***************************************************************************/
    /**
	  * Retrieves the decorator data of all decorators, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  */
    virtual std::string decoratorData(
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
	  * Loads the data of another object
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
    friend void compare_base_t<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(
        GDecoratorContainer<dimensions::Dim3, coordinate_type> const &,
        GDecoratorContainer<dimensions::Dim3, coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
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
        compare_base_t<GCommonInterfaceT<GDecoratorContainer<dimensions::Dim3, coordinate_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then the local data. This allows us to compare
        // the second parent class without directly calling it.
        compare_t(IDENTITY(this->data_cnt_, p_load->data_cnt_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDecoratorContainer<dimensions::Dim3, coordinate_type>");
    }

    /***************************************************************************/
    /**
	 * Creates a deep clone of this object.
	 */
    GDecoratorContainer<dimensions::Dim3, coordinate_type> *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization for DD decorators
 */
template <typename coordinate_type>
class GDecoratorContainer_3D : public GDecoratorContainer<dimensions::Dim3, coordinate_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
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
	  * Loads the data of another object
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
    friend void compare_base_t<GDecoratorContainer_3D<coordinate_type>>(
        GDecoratorContainer_3D<coordinate_type> const &,
        GDecoratorContainer_3D<coordinate_type> const &,
        GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
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

        GToken token("GDecoratorContainer_3D<dimensions::Dim2>", e);

        // Compare our parent data ...
        compare_base_t<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(
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
	 * Returns the name of this class
	 */
    std::string name_() const override {
        return std::string("GDecoratorContainer_3D<coordinate_type>");
    }

    /***************************************************************************/
    /**
	 * Creates a deep clone of this object.
	 */
    GDecoratorContainer<dimensions::Dim3, coordinate_type> *clone_() const override {
        return new GDecoratorContainer_3D<coordinate_type>(*this);
    }

    /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////

} /* namespace Gem::Common */
