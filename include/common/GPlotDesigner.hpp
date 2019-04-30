/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>
#include <tuple>
#include <algorithm>
#include <type_traits>

// Boost headers go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/adapted/std_tuple.hpp> // Compare http://stackoverflow.com/questions/18158376/getting-boostspiritqi-to-use-stl-containers
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPtrVectorT.hpp"
#include "common/GTypeTraitsT.hpp"
#include "common/GTupleIO.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GSerializeTupleT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An enum for some basic colors (to be extended over time)
 */
enum class gColor :
    ENUMBASETYPE
{
    white = 0
    , black = 1
    , red = 2
    , green = 3
    , blue = 4
    , grey = 14 // note the id of this color, compared to preceding values
};

/** @brief Puts a gColor into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const gColor &);

/** @brief Reads a gColor item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, gColor &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An enum for basic marker types (to be extended over time)
 */
enum class gMarker :
    ENUMBASETYPE
{
    none = 0
    , openCircle = 4
    , closedCircle = 20
    , closedTriangle = 22
    , openTriangle = 26
    , closedStar = 29
    , openStar = 30
};

/** @brief Puts a gMarker into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, const gMarker &);

/** @brief Reads a gMarker item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, gMarker &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************	***********/
/**
 * An enum for basic line styles (to be extended over time)
 */
enum class gLineStyle :
    ENUMBASETYPE
{
    straight = 1
    , shortdashed = 2
    , dotted = 3
    , shortdashdot = 4
    , longdashdot = 4
    , longdashed = 7
};

/** @brief Puts a gLineStyle into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, gLineStyle const &);

/** @brief Reads a gLineStyle item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, gLineStyle &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Determines whether a scatter plot or a curve should be recorded
 */
enum class graphPlotMode :
    ENUMBASETYPE
{
    SCATTER = 0
    , CURVE = 1
};

/** @brief Puts a graphPlotMode into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, graphPlotMode const &);

/** @brief Reads a graphPlotMode item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, graphPlotMode &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An enum for 2D-drawing options
 */
enum class tddropt :
    ENUMBASETYPE
{
    TDEMPTY = 0, SURFONE = 1, SURFTWOZ = 2, SURFTHREE = 3, SURFFOUR = 4, CONTZ = 5, CONTONE = 6, CONTTWO = 7,
    CONTTHREE = 8, TEXT = 9, SCAT = 10, BOX = 11, ARR = 12, COLZ = 13, LEGO = 14, LEGOONE = 15, SURFONEPOL = 16,
    SURFONECYL = 17
};

/** @brief Puts a tddropt into a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::ostream &operator<<(std::ostream &, tddropt const &);

/** @brief Reads a tddropt item from a stream. Needed also for boost::lexical_cast<> */
G_API_COMMON std::istream &operator>>(std::istream &, tddropt &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

//Some default values

const std::uint32_t DEFCXDIM = 1024;
const std::uint32_t DEFCYDIM = 768;

const std::uint32_t DEFCXDIV = 1;
const std::uint32_t DEFCYDIV = 1;

const std::size_t DEFNINDENTIONSPACES = 3;

const std::size_t DEFNSAMPLES = 100;

const graphPlotMode DEFPLOTMODE = graphPlotMode::CURVE;

const double DEFMINMARKERSIZE = 0.001;
const double DEFMAXMARKERSIZE = 1.;

// Easier access to the header-, body- and footer-data
using plotData = std::tuple<std::string, std::string, std::string>;

// Easier acces to lines
using pointData = std::tuple<double, double, double>;
using line = std::tuple<pointData, pointData>;

// Forward declaration in order to allow a friend statement in GBasePlotter
class GPlotDesigner;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
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
template<dimensions dim, typename coordinate_type>
class GDecorator
{ /* nothing */ };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is the specialization of GDecorator for 2D-plots (e.g. histograms, graphs, ...)
 */
template<typename coordinate_type>
class GDecorator<dimensions::Dim2, coordinate_type>
    : public GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        /* nothing */
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic<coordinate_type>::value
        , "coordinate_type should either be a floating-point or an integer type"
    );

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GDecorator() = default;
    GDecorator(GDecorator<dimensions::Dim2, coordinate_type> const &cp) = default;
    GDecorator(GDecorator<dimensions::Dim2, coordinate_type> &&cp) noexcept = default;
    virtual ~GDecorator() = default;

    GDecorator<dimensions::Dim2, coordinate_type> &operator=(GDecorator<dimensions::Dim2, coordinate_type> const&) = default;
    GDecorator<dimensions::Dim2, coordinate_type> &operator=(GDecorator<dimensions::Dim2, coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data. Plot boundaries are not taken into account.
	  */
    virtual std::string decoratorData(const std::string &, const std::size_t &) const BASE = 0;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  */
    virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &y_axis_range
        , const std::string &indent
        , const std::size_t &
    ) const BASE = 0;

protected:
    /***************************************************************************/
    /**
     * Loads the data of another object
     */
    void load_(const GDecorator<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // No parent class with loadable data

        // No local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecorator<dimensions::Dim2, coordinate_type>>(
        GDecorator<dimensions::Dim2, coordinate_type> const &
        , GDecorator<dimensions::Dim2, coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 */
    void compare_(
        const GDecorator<dimensions::Dim2, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDecorator<dimensions::Dim2, coordinate_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GCommonInterfaceT<GDecorator<dimensions::Dim2, coordinate_type>>>(
            *this
            , *p_load
            , token
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
template<typename coordinate_type>
class GMarker
    : public GDecorator<dimensions::Dim2, coordinate_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDecorator2<dimensions::Dim2, coordinate_type>"
            , boost::serialization::base_object<GDecorator<dimensions::Dim2, coordinate_type>>(*this))
        & BOOST_SERIALIZATION_NVP(m_coordinates)
        & BOOST_SERIALIZATION_NVP(m_marker)
        & BOOST_SERIALIZATION_NVP(m_color)
        & BOOST_SERIALIZATION_NVP(m_size);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * The standard constructor, which takes all essential data for this
	  * decorator type.
	  */
    GMarker(
        const std::tuple<coordinate_type, coordinate_type> &coordinates
        , const gMarker &marker
        , const gColor &color
        , const double &size
    )
        : m_coordinates(coordinates)
        , m_marker(marker)
        , m_color(color)
        , m_size(size)
    { /* nothing */ }

    /***************************************************************************/
    // Defaulted constructos, destructor and assignment operators. The default-
    // constructor is private, as it is only needed for (de)serialization.

    GMarker(GMarker<coordinate_type> const &cp) = default;
    GMarker(GMarker<coordinate_type> &&cp) noexcept = default;
    ~GMarker() override = default;

    GMarker<coordinate_type> & operator=(GMarker<coordinate_type> const&) = default;
    GMarker<coordinate_type> & operator=(GMarker<coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data. Plot boundaries are not taken into account.
	  */
    std::string decoratorData(const std::string &indent, const std::size_t &pos) const override {
        std::ostringstream data;

        data
            << indent << "TMarker * tm_" << pos << " = new TMarker("
            << boost::numeric_cast<double>(std::get<0>(m_coordinates)) << ", "
            << boost::numeric_cast<double>(std::get<1>(m_coordinates)) << ", "
            << m_marker << ");"
            << std::endl
            << indent << "tm_" << pos << "->SetMarkerColor(" << m_color << ");" << std::endl
            << indent << "tm_" << pos << "->SetMarkerSize(" << m_size << ");" << std::endl
            << indent << "tm_" << pos << "->Draw();" << std::endl
            << std::endl;

        return data.str();
    }

    /***************************************************************************/
    /**
	  * Retrieves the decorator data. Plot boundaries are taken into account.
	  */
    std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &y_axis_range
        , const std::string &indent
        , const std::size_t &pos
    ) const override {
        coordinate_type marker_x = std::get<0>(m_coordinates);
        coordinate_type marker_y = std::get<1>(m_coordinates);
        coordinate_type x_min = std::get<0>(x_axis_range);
        coordinate_type x_max = std::get<1>(x_axis_range);
        coordinate_type y_min = std::get<0>(y_axis_range);
        coordinate_type y_max = std::get<1>(y_axis_range);

        // Check if our coordinates are inside of the axis range
        if (
            marker_x < x_min || marker_x > x_max
            || marker_y < y_min || marker_y > y_max
            ) {
            return this->decoratorData(
                indent
                , pos
            );
        } else {
            return std::string();
        }
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GDecorator<dimensions::Dim2, coordinate_type> *cp) override {
        // Check that we are dealing with a GMarker reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent data ...
        GDecorator<dimensions::Dim2, coordinate_type>::load_(cp);

        // ... and then our local data
        m_coordinates = p_load->m_coordinates;
        m_marker = p_load->m_marker;
        m_color = p_load->m_color;
        m_size = p_load->m_size;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GMarker<coordinate_type>>(
        GMarker<coordinate_type> const &
        , GMarker<coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GDecorator<dimensions::Dim2, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GMarker reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GMarker<coordinate_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GDecorator<dimensions::Dim2, coordinate_type>>(
            *this
            , *p_load
            , token
        );

        // ... and then our local data
        compare_t(
            IDENTITY(this->m_coordinates
                     , p_load->m_coordinates)
            , token
        );
        compare_t(
            IDENTITY(this->m_marker
                     , p_load->m_marker)
            , token
        );
        compare_t(
            IDENTITY(this->m_color
                     , p_load->m_color)
            , token
        );
        compare_t(
            IDENTITY(this->m_size
                     , p_load->m_size)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }


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

    std::tuple<coordinate_type, coordinate_type> m_coordinates; ///< The coordinates of the marker

    gMarker m_marker = gMarker::closedCircle; ///< Denotes the type of markers to be drawn
    gColor m_color = gColor::black; ///< The color of the marker
    double m_size = 0.05; ///< The size of the marker
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This is the specialization of GDecorator for 3D-plots (e.g. 2D-histograms, 3D-graphs, ...)
 */
template<typename coordinate_type>
class GDecorator<dimensions::Dim3, coordinate_type>
    : public GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // nothing
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic<coordinate_type>::value
        , "coordinate_type should either be a floating-point or an integer type"
    );

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators.

    GDecorator() = default;
    GDecorator(GDecorator<dimensions::Dim3, coordinate_type> const &cp) = default;
    GDecorator(GDecorator<dimensions::Dim3, coordinate_type> &&cp) noexcept = default;
    virtual ~GDecorator() = default;

    GDecorator<dimensions::Dim3, coordinate_type> &operator=(GDecorator<dimensions::Dim3, coordinate_type> const&) = default;
    GDecorator<dimensions::Dim3, coordinate_type> &operator=(GDecorator<dimensions::Dim3, coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data. Plot boundaries are not taken into account.
	  */
    virtual std::string decoratorData(const std::string &, const std::size_t &) const BASE = 0;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data, taking into account externally supplied
	  * plot boundaries. Decorators will usually not be drawn if they would "live" outside
	  * of the plot boundaries. Lines will be cut at the boundaries. Text, however, will
	  * not be affected by the boundaries. This function needs to be implemented by derived
	  * classes.
	  */
    virtual std::string decoratorData(
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &y_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &z_axis_range
        , const std::string &indent
        , const std::size_t &pos
    ) const BASE = 0;

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GDecorator<dimensions::Dim3, coordinate_type> *cp) override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // No parent class with loadable data

        // No local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecorator<dimensions::Dim3, coordinate_type>>(
        GDecorator<dimensions::Dim3, coordinate_type> const &
        , GDecorator<dimensions::Dim3, coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GDecorator<dimensions::Dim3, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecorator reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDecorator<dimensions::Dim3, coordinate_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GCommonInterfaceT<GDecorator<dimensions::Dim3, coordinate_type>>>(
            *this
            , *p_load
            , token
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
 * it is derived from GPtrVectorT and may thus be treated like a
 * std::vector of std::shared_ptr<GDecorator<dim>> . Note that the actual work
 * is done in the specializations for different dimensions. Hence some code
 * duplications for the different template specializations cannot be avoided.
 */
template<dimensions dim, typename coordinate_type>
class GDecoratorContainer
{ /* nothing */ };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GDecoratorContainer for 2D-plots
 */
template<typename coordinate_type>
class GDecoratorContainer<dimensions::Dim2, coordinate_type>
    : public GCommonInterfaceT<GDecoratorContainer<dimensions::Dim2, coordinate_type>>
    , public GPtrVectorT<GDecorator<dimensions::Dim2, coordinate_type>, GDecorator<dimensions::Dim2, coordinate_type>>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GStdPtrVectorInterfaceT_GDecorator2"
            , boost::serialization::base_object<GPtrVectorT<GDecorator<dimensions::Dim2, coordinate_type>, GDecorator<dimensions::Dim2, coordinate_type>>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic<coordinate_type>::value
        , "coordinate_type should either be a floating-point or an integer type"
    );

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GDecoratorContainer() = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim2, coordinate_type> const &cp) = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim2, coordinate_type> &&cp) noexcept = default;

    GDecoratorContainer<dimensions::Dim2, coordinate_type> &operator=(GDecoratorContainer<dimensions::Dim2, coordinate_type> const&) = default;
    GDecoratorContainer<dimensions::Dim2, coordinate_type> &operator=(GDecoratorContainer<dimensions::Dim2, coordinate_type> &&) noexcept = default;

    virtual ~GDecoratorContainer() = default;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data of all decorators. Plot boundaries are
	  * not taken into account.
	  */
    virtual std::string decoratorData(const std::string &indent) const BASE {
        std::string result;

        std::size_t pos = 0;
        for (auto const& decorator_ptr: *this) {
            result += decorator_ptr->GDecorator<dimensions::Dim2, coordinate_type>::decoratorData(
                indent
                , pos++
            );
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
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &y_axis_range
        , const std::string &indent
    ) const BASE {
        std::string result;

        std::size_t pos = 0;
        for (auto const& decorator_ptr: *this) {
            result += decorator_ptr->GDecorator<dimensions::Dim2, coordinate_type>::decoratorData(
                x_axis_range
                , y_axis_range
                , indent
                , pos++
            );
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
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent data ...
        GPtrVectorT<GDecorator<dimensions::Dim2, coordinate_type>, GDecorator<dimensions::Dim2, coordinate_type>>::operator=(*p_load);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(
        GDecoratorContainer<dimensions::Dim2, coordinate_type> const &
        , GDecoratorContainer<dimensions::Dim2, coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GDecoratorContainer<dimensions::Dim2, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDecoratorContainer<dimensions::Dim2>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GCommonInterfaceT<GDecoratorContainer<dimensions::Dim2, coordinate_type>>>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data. Actually this allows us to compare
        // the second parent class without directly calling it.
        compare_t(
            IDENTITY(this->m_data_cnt
                     , p_load->m_data_cnt)
            , token
        );

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
class GDecoratorContainer_2D
    : public GDecoratorContainer<dimensions::Dim2, coordinate_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDecoratorContainer_2D"
            , boost::serialization::base_object<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(*this));
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
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent data ...
        GDecoratorContainer<dimensions::Dim2, coordinate_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecoratorContainer_2D<coordinate_type>>(
        GDecoratorContainer_2D<coordinate_type> const &
        , GDecoratorContainer_2D<coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GDecoratorContainer<dimensions::Dim2, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDecoratorContainer_2D<dimensions::Dim2>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GDecoratorContainer<dimensions::Dim2, coordinate_type>>(
            *this
            , *p_load
            , token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override { return false; }
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
template<typename coordinate_type>
class GDecoratorContainer<dimensions::Dim3, coordinate_type>
    : public GCommonInterfaceT<GDecoratorContainer<dimensions::Dim3, coordinate_type>>
    , public GPtrVectorT<GDecorator<dimensions::Dim3, coordinate_type>, GDecorator<dimensions::Dim3, coordinate_type>>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GStdPtrVectorInterfaceT_GDecorator3"
            , boost::serialization::base_object<GPtrVectorT<GDecorator<dimensions::Dim3, coordinate_type>, GDecorator<dimensions::Dim3, coordinate_type>>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(
        std::is_arithmetic<coordinate_type>::value
        , "coordinate_type should either be a floating-point or an integer type"
    );

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GDecoratorContainer() = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim3, coordinate_type> const &cp) = default;
    GDecoratorContainer(GDecoratorContainer<dimensions::Dim3, coordinate_type> &&cp) noexcept = default;
    ~GDecoratorContainer() override = default;

    GDecoratorContainer<dimensions::Dim3, coordinate_type> &operator=(GDecoratorContainer<dimensions::Dim3, coordinate_type> const&) = default;
    GDecoratorContainer<dimensions::Dim3, coordinate_type> &operator=(GDecoratorContainer<dimensions::Dim3, coordinate_type> &&) noexcept = default;

    /***************************************************************************/
    /**
	  * Retrieves the decorator data of all decorators. Plot boundaries are
	  * not taken into account.
	  */
    virtual std::string decoratorData(const std::string &indent) const BASE {
        std::string result;

        std::size_t pos = 0;
        for (auto const& decorator_ptr: *this) {
            result += decorator_ptr->GDecorator<dimensions::Dim3, coordinate_type>::decoratorData(
                indent
                , pos++
            );
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
        const std::tuple<coordinate_type, coordinate_type> &x_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &y_axis_range
        , const std::tuple<coordinate_type, coordinate_type> &z_axis_range
        , const std::string &indent
    ) const BASE {
        std::string result;

        std::size_t pos = 0;
        for (auto const& decorator_ptr: *this) {
            result += decorator_ptr->GDecorator<dimensions::Dim3, coordinate_type>::decoratorData(
                x_axis_range
                , y_axis_range
                , z_axis_range
                , indent
                , pos++
            );
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
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent data ...
        GPtrVectorT<GDecorator<dimensions::Dim3, coordinate_type>, GDecorator<dimensions::Dim3, coordinate_type>>::operator=(*p_load);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(
        GDecoratorContainer<dimensions::Dim3, coordinate_type> const &
        , GDecoratorContainer<dimensions::Dim3, coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GDecoratorContainer<dimensions::Dim3, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDecoratorContainer<dimensions::Dim3>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GCommonInterfaceT<GDecoratorContainer<dimensions::Dim3, coordinate_type>>>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data. This allows us to compare
        // the second parent class without directly calling it.
        compare_t(
            IDENTITY(this->m_data_cnt
                     , p_load->m_data_cnt)
            , token
        );

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
class GDecoratorContainer_3D
    : public GDecoratorContainer<dimensions::Dim3, coordinate_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDecoratorContainer_Dim3"
            , boost::serialization::base_object<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(*this));
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
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent data ...
        GDecoratorContainer<dimensions::Dim3, coordinate_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDecoratorContainer_3D<coordinate_type>>(
        GDecoratorContainer_3D<coordinate_type> const &
        , GDecoratorContainer_3D<coordinate_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GDecoratorContainer<dimensions::Dim3, coordinate_type> &cp // the other object
        , const expectation &e // the expectation for this object, e.g. equality
        , const double &limit // the limit for allowed deviations of floating point types
    ) const override {
        // Check that we are dealing with a GDecoratorContainer reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDecoratorContainer_3D<dimensions::Dim2>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GDecoratorContainer<dimensions::Dim3, coordinate_type>>(
            *this
            , *p_load
            , token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override { return false; }
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
/******************************************************************************/
/**
 * An abstract base class that defines functions for plots. Concrete plotters
 * derive from this class. They can be added to a master canvas, which takes care
 * to plot them into sub-pads.
 */
class GBasePlotter
    : public GCommonInterfaceT<GBasePlotter>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_NVP(m_drawingArguments)
        & BOOST_SERIALIZATION_NVP(m_x_axis_label)
        & BOOST_SERIALIZATION_NVP(m_y_axis_label)
        & BOOST_SERIALIZATION_NVP(m_z_axis_label)
        & BOOST_SERIALIZATION_NVP(m_plot_label)
        & BOOST_SERIALIZATION_NVP(m_dsMarker)
        & BOOST_SERIALIZATION_NVP(m_secondaryPlotter)
        & BOOST_SERIALIZATION_NVP(m_id);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Copy constructor */
    G_API_COMMON GBasePlotter(GBasePlotter const &);
    /** @brief Assignment operator */
    G_API_COMMON GBasePlotter& operator=(GBasePlotter const &);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_COMMON GBasePlotter() = default;
    G_API_COMMON GBasePlotter(GBasePlotter &&) = default;
    virtual G_API_COMMON ~GBasePlotter() = default;

    G_API_COMMON GBasePlotter& operator=(GBasePlotter &&) = default;

    /*********************************************************************/

    /** @brief Allows to set the drawing arguments for this plot */
    G_API_COMMON void setDrawingArguments(std::string);

    /** @brief Sets the label for the x-axis */
    G_API_COMMON void setXAxisLabel(std::string);
    /** @brief Retrieve the x-axis label */
    G_API_COMMON std::string xAxisLabel() const;
    /** @brief Sets the label for the y-axis */
    G_API_COMMON void setYAxisLabel(std::string);
    /** @brief Retrieve the y-axis label */
    G_API_COMMON std::string yAxisLabel() const;
    /** @brief Sets the label for the z-axis */
    G_API_COMMON void setZAxisLabel(std::string);
    /** @brief Retrieve the z-axis label */
    G_API_COMMON std::string zAxisLabel() const;

    /** @brief Allows to assign a label to the entire plot */
    G_API_COMMON void setPlotLabel(std::string);
    /** @brief Allows to retrieve the plot label */
    G_API_COMMON std::string plotLabel() const;

    /** @brief Allows to assign a marker to data structures */
    G_API_COMMON void setDataStructureMarker(std::string);
    /** @brief Allows to retrieve the data structure marker */
    G_API_COMMON std::string dsMarker() const;

    /** @brief Allows to add secondary plots to be added to the same sub-canvas */
    G_API_COMMON void registerSecondaryPlotter(std::shared_ptr<GBasePlotter>);

    /** @brief Allows to retrieve the id of this object */
    G_API_COMMON std::size_t id() const;
    /** @brief Sets the id of the object */
    G_API_COMMON void setId(const std::size_t &);

    /** @brief Retrieves a unique name for this plotter */
    virtual G_API_COMMON std::string getPlotterName() const BASE = 0;

    /***************************************************************************/

    /** @brief Retrieve header settings for this plot (and any sub-plots) */
    std::string headerData(const std::string &) const;

    /** @brief Retrieves body / data settings for this plot (and any sub-plots) */
    std::string bodyData(const std::string &) const;

    /** @brief Retrieves footer / drawing settings for this plot (and any sub-plots) */
    std::string footerData(const std::string &) const;

protected:
    /***************************************************************************/
    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GBasePlotter>(
        GBasePlotter const &
        , GBasePlotter const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /***************************************************************************/
    // Functions to be specified in derived classes

    /** @brief Retrieve specific header settings for this plot */
    virtual G_API_COMMON std::string headerData_(bool, std::size_t, const std::string &) const BASE = 0;

    /** @brief Retrieves the actual data sets */
    virtual G_API_COMMON std::string bodyData_(bool, std::size_t, const std::string &) const BASE = 0;

    /** @brief retrieves specific draw commands for this plot */
    virtual G_API_COMMON std::string footerData_(bool, std::size_t, const std::string &) const BASE = 0;

    /** @brief Retrieve the current drawing arguments */
    virtual G_API_COMMON std::string drawingArguments(bool) const BASE = 0;

    /** @brief Check that a given plotter is compatible with us */
    virtual G_API_COMMON bool isCompatible(std::shared_ptr<GBasePlotter>) const BASE;

    /** @brief calculate a suffix from id and parent ids */
    G_API_COMMON std::string suffix(bool, std::size_t) const;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_GENEVA bool modify_GUnitTests_() override { return false; }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

    /***************************************************************************/

    std::string m_drawingArguments = std::string(""); ///< Holds the drawing arguments for this plot

    std::string m_x_axis_label = std::string("x"); ///< A label for the x-axis
    std::string m_y_axis_label = std::string("y"); ///< A label for the y-axis
    std::string m_z_axis_label = std::string("z"); ///< A label for the z-axis (if available)

    std::string m_plot_label = std::string("");   ///< A label to be assigned to the entire plot
    std::string m_dsMarker = std::string("");     ///< A marker to make the origin of data structures clear in the output file

    std::vector<line> lines_; ///< Lines to be drawn into the drawing area

private:
    /***************************************************************************/
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override = 0;

    /***************************************************************************/
    /** @brief A list of plotters that should emit their data into the same canvas */
    std::vector<std::shared_ptr<GBasePlotter>> m_secondaryPlotter = std::vector<std::shared_ptr<GBasePlotter>>();

    std::size_t m_id = 0; ///< The id of this object
};

/******************************************************************************/
/**
 * A data collector for 1-d data of user-defined type. This will usually be
 * data of a histogram type. It is assumed to be movable, hence we use all
 * defaulted constructors and assignment operators.
 */
template<typename x_type>
class GDataCollector1T
    : public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(m_data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector1T() = default;
    GDataCollector1T(GDataCollector1T<x_type> const &) = default;
    GDataCollector1T(GDataCollector1T<x_type> &&) = default;

    ~GDataCollector1T() override = default;

    GDataCollector1T<x_type>& operator=(GDataCollector1T<x_type> const&) = default;
    GDataCollector1T<x_type>& operator=(GDataCollector1T<x_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to retrieve information about the amount of data sets stored in
	  * this object
	  */
    std::size_t currentSize() const {
        return m_data.size();
    }

    /***************************************************************************/
    /**
	  * This very simple functions allows derived classes
	  * to add data easily to their data sets, when called through a
	  * pointer. I.e., this makes "object_ptr->add(data)" instead of
	  * "*object_ptr & data" possible.
	  */
    template<typename data_type>
    void add(const data_type &item) {
        *this & item;
    }

    /***************************************************************************/
    /**
	  * Allows to add data of arbitrary type, provided it can be converted
	  * safely to the target type.
	  *
	  * @param x_undet The data item to be added to the collection
	  */
    template<typename x_type_undet>
    void operator&(const x_type_undet &x_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = boost::numeric_cast<x_type>(x_undet);
        }
        catch (bad_numeric_cast &e) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GDataCollector1T<x_type>::operator&(const T&): Error!" << std::endl
                    << "Encountered invalid cast with boost::numeric_cast," << std::endl
                    << "with the message " << std::endl
                    << e.what() << std::endl
            );
        }

        // Add the converted data to our collection
        m_data.push_back(x);
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type "x_type
	  *
	  * @param x The data item to be added to the collection
	  */
    void operator&(const x_type &x) {
        // Add the data item to our collection
        m_data.push_back(x);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type in one go,
	  * provided the type can be converted safely into the target type
	  *
	  * @param x_cnt_undet A collection of data items of undetermined type, to be added to the collection
	  */
    template<typename x_type_undet>
    void operator&(const std::vector<x_type_undet> &x_cnt_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);

        typename std::vector<x_type_undet>::const_iterator cit;
        for (cit = x_cnt_undet.begin(); cit != x_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = boost::numeric_cast<x_type>(*cit);
            }
            catch (bad_numeric_cast &e) {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GDataCollector1T::operator&(const std::vector<T>&): Error!" << std::endl
                        << "Encountered invalid cast with boost::numeric_cast," << std::endl
                        << "with the message " << std::endl
                        << e.what() << std::endl
                );
            }

            // Add the converted data to our collection
            m_data.push_back(x);
        }
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of type x_type to our m_data vector.
	  *
	  * @param x_cnt A vector of data items to be added to the m_data vector
	  */
    void operator&(const std::vector<x_type> &x_cnt) {
        typename std::vector<x_type>::const_iterator cit;
        for (cit = x_cnt.begin(); cit != x_cnt.end(); ++cit) {
            // Add the data item to our collection
            m_data.push_back(*cit);
        }
    }

    /***************************************************************************/
    /**
	  * Retrieves the minimum and maximum values in m_data
	  */
    std::tuple<x_type, x_type> getMinMaxElements() const {
        auto minmax = std::minmax_element(
            m_data.begin()
            , m_data.end());
        return std::make_tuple(
            *minmax.first
            , *minmax.second
        );
    };

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector1T<x_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own
        m_data = p_load->m_data; // This assumes that x_type is POD
    }

    /***************************************************************************/

    friend void compare_base_t<GDataCollector1T<x_type>>(
        GDataCollector1T<x_type> const &
        , GDataCollector1T<x_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GBasePlotter &cp
        , const expectation &e
        , const double &limit
    ) const override {
        // Check that we are dealing with a GDataCollector1T<x_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDataCollector1T<x_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(m_data
                     , p_load->m_data)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<x_type> m_data; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDataCollector1T<x_type>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override = 0;

    /***************************************************************************/
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH1D class (1-d double data). This will result in a 2D-plot.
 */
class GHistogram1D : public GDataCollector1T<double>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector1T_double"
            , boost::serialization::base_object<GDataCollector1T<double>>(*this))
        & BOOST_SERIALIZATION_NVP(nBinsX_)
        & BOOST_SERIALIZATION_NVP(minX_)
        & BOOST_SERIALIZATION_NVP(maxX_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with the number of bins and automatic range detection */
    explicit G_API_COMMON GHistogram1D(
        const std::size_t &
    );

    /** @brief Initialization with a range in the form of a tuple */
    G_API_COMMON GHistogram1D(
        const std::size_t &
        , const double &
        , const double &
    );
    /** @brief Initialization with a range in the form of a tuple */
    G_API_COMMON GHistogram1D(
        const std::size_t &
        , const std::tuple<double, double> &
    );

    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_COMMON GHistogram1D(GHistogram1D const &) = default;
    G_API_COMMON GHistogram1D(GHistogram1D &&) = default;
    G_API_COMMON ~GHistogram1D() override = default;

    G_API_COMMON GHistogram1D& operator=(GHistogram1D const&) = default;
    G_API_COMMON GHistogram1D& operator=(GHistogram1D &&) = default;

    // Defaulted default-constructor in private section

    /**********************************************************************/

    /** @brief Retrieve the number of bins in x-direction */
    G_API_COMMON std::size_t getNBinsX() const;

    /** @brief Retrieve the lower boundary of the plot */
    G_API_COMMON double getMinX() const;
    /** @brief Retrieve the upper boundary of the plot */
    G_API_COMMON double getMaxX() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /***************************************************************************/

    friend void compare_base_t<GHistogram1D>(
        GHistogram1D const &
        , GHistogram1D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    G_API_COMMON GHistogram1D() = default; ///< The default constructor -- intentionally private as it is only needed for (de-)serialization

    std::size_t nBinsX_ = 10; ///< The number of bins in the histogram

    double minX_ = 0; ///< The lower boundary of the histogram
    double maxX_ = minX_; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH1I class (1-d integer data)
 */
class GHistogram1I
    : public GDataCollector1T<std::int32_t>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector1T_int32_t"
            , boost::serialization::base_object<GDataCollector1T<std::int32_t>>(*this))
        & BOOST_SERIALIZATION_NVP(nBinsX_)
        & BOOST_SERIALIZATION_NVP(minX_)
        & BOOST_SERIALIZATION_NVP(maxX_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    G_API_COMMON GHistogram1I(
        const std::size_t &, const double &, const double &
    );
    /** @brief Initialization with a range in the form of a tuple */
    G_API_COMMON GHistogram1I(
        const std::size_t &, const std::tuple<double, double> &
    );

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operator

    G_API_COMMON GHistogram1I(GHistogram1I const&) = default;
    G_API_COMMON GHistogram1I(GHistogram1I &&) = default;

    // Defaulted default-constructor in private section

    G_API_COMMON ~GHistogram1I() override = default;

    G_API_COMMON GHistogram1I& operator=(GHistogram1I const&) = default;
    G_API_COMMON GHistogram1I& operator=(GHistogram1I &&) = default;

    /*********************************************************************/

    /** @brief Retrieve the number of bins in x-direction */
    G_API_COMMON std::size_t getNBinsX() const;

    /** @brief Retrieve the lower boundary of the plot */
    G_API_COMMON double getMinX() const;
    /** @brief Retrieve the upper boundary of the plot */
    G_API_COMMON double getMaxX() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GHistogram1I>(
        GHistogram1I const &
        , GHistogram1I const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    GHistogram1I() = default; ///< The default constructor -- intentionally private as it is only needed for (de-)serialization

    std::size_t nBinsX_; ///< The number of bins in the histogram

    double minX_; ///< The lower boundary of the histogram // TODO: Really "double" ?
    double maxX_; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * A data collector for 2-d data of user-defined type, such as a TGraph.
 * Note that the plot dimension may be different.
 */
template<typename x_type, typename y_type>
class GDataCollector2T
    :
        public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(m_data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector2T() = default;
    GDataCollector2T(GDataCollector2T<x_type, y_type> const &) = default;
    GDataCollector2T(GDataCollector2T<x_type, y_type> &&) = default;

    ~GDataCollector2T() override = default;

    GDataCollector2T<x_type, y_type> &operator=(GDataCollector2T<x_type, y_type> const&) = default;
    GDataCollector2T<x_type, y_type> &operator=(GDataCollector2T<x_type, y_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to retrieve information about the amount of data sets stored in
	  * this object
	  */
    std::size_t currentSize() const {
        return m_data.size();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (x-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<x_type>> projectX(
        std::size_t
        , std::tuple<x_type, x_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector2T<>::projectX(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<x_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (y-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<y_type>> projectY(
        std::size_t
        , std::tuple<y_type, y_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector2T<>::projectY(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<y_type>>();
    }

    /***************************************************************************/
    /**
	  * This very simple functions allows derived classes
	  * to add data easily to their data sets, when called through a
	  * pointer. I.e., this makes object_ptr->add(data) instead of
	  * *object_ptr & data possible.
	  */
    template<typename data_type1, typename data_type2>
    void add(const data_type1 &item1, const data_type2 &item2) {
        *this & std::make_tuple(
            item1
            , item2
        );
    }

    /***************************************************************************/
    /**
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @param point_undet The data item to be added to the collection
	  */
    template<typename x_type_undet, typename y_type_undet>
    void operator&(const std::tuple<x_type_undet, y_type_undet> &point_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        y_type y = y_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = boost::numeric_cast<x_type>(std::get<0>(point_undet));
            y = boost::numeric_cast<y_type>(std::get<1>(point_undet));
        }
        catch (bad_numeric_cast &e) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GDataCollector2T::operator&(const std::tuple<S,T>&): Error!" << std::endl
                    << "Encountered invalid cast with boost::numeric_cast," << std::endl
                    << "with the message " << std::endl
                    << e.what() << std::endl
            );
        }

        m_data.push_back(
            std::tuple<x_type, y_type>(
                x
                , y
            ));
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type std::tuple<x_type, y_type> to the collection in
	  * an intuitive way.
	  *
	  * @param point The data item to be added to the collection
	  */
    void operator&(const std::tuple<x_type, y_type> &point) {
        // Add the data item to the collection
        m_data.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template<typename x_type_undet, typename y_type_undet>
    void operator&(const std::vector<std::tuple<x_type_undet, y_type_undet>> &point_cnt_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        y_type y = y_type(0);

        typename std::vector<std::tuple<x_type_undet, y_type_undet>>::const_iterator cit;
        for (cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = boost::numeric_cast<x_type>(std::get<0>(*cit));
                y = boost::numeric_cast<y_type>(std::get<1>(*cit));
            }
            catch (bad_numeric_cast &e) {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GDataCollector2T::operator&(const std::vector<std::tuple<S,T>>&): Error!" << std::endl
                        << "Encountered invalid cast with boost::numeric_cast," << std::endl
                        << "with the message " << std::endl
                        << e.what() << std::endl
                );
            }

            m_data.push_back(
                std::tuple<x_type, y_type>(
                    x
                    , y
                ));
        }
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of type std::tuple<x_type, y_type>
	  * to the collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt The collection of data items to be added to the collection
	  */
    void operator&(const std::vector<std::tuple<x_type, y_type>> &point_cnt) {
        typename std::vector<std::tuple<x_type, y_type>>::const_iterator cit;
        for (cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            m_data.push_back(*cit);
        }
    }

    /***************************************************************************/
    /**
	  * Sorts the data according to its x-component
	  */
    void sortX() {
        std::sort(
            m_data.begin()
            , m_data.end()
            , [](const std::tuple<x_type, y_type> &x, const std::tuple<x_type, y_type> &y) -> bool {
                return std::get<0>(x) < std::get<0>(y);
            }
        );
    }

    /***************************************************************************/
    /**
		* Retrieves the minimum and maximum values in m_data in x- and y-direction
		*/
    std::tuple<x_type, x_type, y_type, y_type> getMinMaxElements() const {
        auto minmax_x = std::minmax_element(
            m_data.begin()
            , m_data.end()
            , [](const std::tuple<x_type, y_type> &x, const std::tuple<x_type, y_type> &y) -> bool {
                return (std::get<0>(x) < std::get<0>(y));
            }
        );

        auto minmax_y = std::minmax_element(
            m_data.begin()
            , m_data.end()
            , [](const std::tuple<x_type, y_type> &x, const std::tuple<x_type, y_type> &y) -> bool {
                return (std::get<1>(x) < std::get<1>(y));
            }
        );

        double minX = std::get<0>(*minmax_x.first);
        double maxX = std::get<0>(*minmax_x.second);
        double minY = std::get<1>(*minmax_y.first);
        double maxY = std::get<1>(*minmax_y.second);

        return std::make_tuple(
            minX
            , maxX
            , minY
            , maxY
        );
    };

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own
        m_data = p_load->m_data; // This assumes that x_type is POD
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector2T<x_type, y_type>>(
        GDataCollector2T<x_type, y_type> const &
        , GDataCollector2T<x_type, y_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GBasePlotter &cp
        , const expectation &e
        , const double &limit
    ) const override {
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDataCollector2T<x_type, y_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(m_data
                     , p_load->m_data)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, y_type>> m_data; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDataCollector2T<x_type, y_type>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override = 0;
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type> = <double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline std::shared_ptr<GDataCollector1T<double>> GDataCollector2T<double, double>::projectX(
    std::size_t nBinsX
    , std::tuple<double, double> rangeX
) const {
    std::tuple<double, double> myRangeX;
    std::tuple<double, double> default_range;
    if (rangeX == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double> extremes = getMinMax(this->m_data);
        myRangeX = std::tuple<double, double>(
            std::get<0>(extremes)
            , std::get<1>(extremes));
    } else {
        myRangeX = rangeX;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsX
            , myRangeX
        ));
    result->setXAxisLabel(this->xAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / x-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<0>(o);
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
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector2T<double, double>::projectY(std::size_t nBinsY, std::tuple<double, double> rangeY) const {
    std::tuple<double, double> myRangeY;
    std::tuple<double, double> default_range;
    if (rangeY == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double> extremes = getMinMax(m_data);
        myRangeY = std::tuple<double, double>(
            std::get<2>(extremes)
            , std::get<3>(extremes));
    } else {
        myRangeY = rangeY;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsY
            , myRangeY
        ));
    result->setXAxisLabel(this->yAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / y-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<1>(o);
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * A data collector for 2-d data of user-defined type, with the ability to
 * additionally specify an error component for both dimensions. Note that the
 * plot dimension may be different.
 */
template<typename x_type, typename y_type>
class GDataCollector2ET
    :
        public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(m_data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector2ET() = default;
    GDataCollector2ET(GDataCollector2ET<x_type, y_type> const &) = default;
    GDataCollector2ET(GDataCollector2ET<x_type, y_type> &&) = default;

    ~GDataCollector2ET() override = default;

    GDataCollector2ET<x_type, y_type> &operator=(GDataCollector2ET<x_type, y_type> const&) = default;
    GDataCollector2ET<x_type, y_type> &operator=(GDataCollector2ET<x_type, y_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @param point_undet The data item to be added to the collection
	  */
    template<typename x_type_undet, typename y_type_undet>
    void operator&(const std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet> &point_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        x_type ex = x_type(0);
        y_type y = y_type(0);
        y_type ey = y_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = boost::numeric_cast<x_type>(std::get<0>(point_undet));
            ex = boost::numeric_cast<x_type>(std::get<1>(point_undet));
            y = boost::numeric_cast<y_type>(std::get<2>(point_undet));
            ey = boost::numeric_cast<y_type>(std::get<3>(point_undet));
        }
        catch (bad_numeric_cast &e) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GDataCollector2ET::operator&(const std::tuple<S,S,T,T>&): Error!" << std::endl
                    << "Encountered invalid cast with boost::numeric_cast," << std::endl
                    << "with the message " << std::endl
                    << e.what() << std::endl
            );
        }

        m_data.push_back(
            std::tuple<x_type, x_type, y_type, y_type>(
                x
                , ex
                , y
                , ey
            ));
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type std::tuple<x_type, y_type> to the collection in
	  * an intuitive way.
	  *
	  * @param point The data item to be added to the collection
	  */
    void operator&(const std::tuple<x_type, x_type, y_type, y_type> &point) {
        // Add the data item to the collection
        m_data.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template<typename x_type_undet, typename y_type_undet>
    void operator&(
        const std::vector<std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>> &point_cnt_undet
    ) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        x_type ex = x_type(0);
        y_type y = y_type(0);
        y_type ey = y_type(0);

        typename std::vector<std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>>::const_iterator cit;
        for (cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = boost::numeric_cast<x_type>(std::get<0>(*cit));
                ex = boost::numeric_cast<x_type>(std::get<1>(*cit));
                y = boost::numeric_cast<y_type>(std::get<2>(*cit));
                ey = boost::numeric_cast<y_type>(std::get<3>(*cit));
            }
            catch (bad_numeric_cast &e) {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GDataCollector2ET::operator&(const std::vector<std::tuple<S,S,T,T>>&): Error!"
                        << std::endl
                        << "Encountered invalid cast with boost::numeric_cast," << std::endl
                        << "with the message " << std::endl
                        << e.what() << std::endl
                );
            }

            m_data.push_back(
                std::tuple<x_type, x_type, y_type, y_type>(
                    x
                    , ex
                    , y
                    , ey
                ));
        }
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of type std::tuple<x_type, x_type, y_type, y_type>
	  * to the collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt The collection of data items to be added to the collection
	  */
    void operator&(const std::vector<std::tuple<x_type, x_type, y_type, y_type>> &point_cnt) {
        typename std::vector<std::tuple<x_type, x_type, y_type, y_type>>::const_iterator cit;
        for (cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            m_data.push_back(*cit);
        }
    }

    /***************************************************************************/
    /**
	  * This very simple functions allows derived classes
	  * to add data easily to their data sets, when called through a
	  * pointer. I.e., this makes "object_ptr->add(data)" instead of
	  * "*object_ptr & data" possible.
	  */
    template<typename data_type>
    void add(const data_type &item) {
        *this & item;
    }

    /***************************************************************************/
    /**
	  * Sorts the data according to its x-component
	  */
    void sortX() {
        std::sort(
            m_data.begin()
            , m_data.end()
            , [](
                const std::tuple<x_type, x_type, y_type, y_type> &x, const std::tuple<x_type, x_type, y_type, y_type> &y
            ) -> bool {
                return std::get<0>(x) < std::get<0>(y);
            }
        );
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector2ET<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own
        m_data = p_load->m_data; // This assumes that x_type is POD
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector2ET<x_type, y_type>>(
        GDataCollector2ET<x_type, y_type> const &
        , GDataCollector2ET<x_type, y_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GBasePlotter &cp
        , const expectation &e
        , const double &limit
    ) const override {
        // Check that we are dealing with a GDataCollector2ET<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDataCollector2ET<x_type, y_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(m_data
                     , p_load->m_data)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, x_type, y_type, y_type>> m_data; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDataCollector2ET<x_type, y_type>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override = 0;
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH2D class (2-d double data). This will result in a
 * 3D plot.
 */
class GHistogram2D
    : public GDataCollector2T<double, double>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector2T_double_double"
            , boost::serialization::base_object<GDataCollector2T<double, double>>(*this))
        & BOOST_SERIALIZATION_NVP(nBinsX_)
        & BOOST_SERIALIZATION_NVP(nBinsY_)
        & BOOST_SERIALIZATION_NVP(minX_)
        & BOOST_SERIALIZATION_NVP(maxX_)
        & BOOST_SERIALIZATION_NVP(minY_)
        & BOOST_SERIALIZATION_NVP(maxY_)
        & BOOST_SERIALIZATION_NVP(dropt_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    G_API_COMMON GHistogram2D(
        const std::size_t &
        , const std::size_t &
        , const double &
        , const double &
        , const double &
        , const double &
    );
    /** @brief Initialization with ranges */
    G_API_COMMON GHistogram2D(
        const std::size_t &
        , const std::size_t &
        , const std::tuple<double, double> &
        , const std::tuple<double, double> &
    );
    /** @brief Initialization with automatic range detection */
    G_API_COMMON GHistogram2D(
        const std::size_t &
        , const std::size_t &
    );

    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    G_API_COMMON GHistogram2D(GHistogram2D const &) = default;
    G_API_COMMON GHistogram2D(GHistogram2D &&) = default;
    G_API_COMMON ~GHistogram2D() override = default;

    G_API_COMMON GHistogram2D& operator=(GHistogram2D const&) = default;
    G_API_COMMON GHistogram2D& operator=(GHistogram2D &&) = default;

    /**********************************************************************/

    /** @brief Retrieve the number of bins in x-direction */
    G_API_COMMON std::size_t getNBinsX() const;
    /** @brief Retrieve the number of bins in y-direction */
    G_API_COMMON std::size_t getNBinsY() const;

    /** @brief Retrieve the lower boundary of the plot in x-direction */
    G_API_COMMON double getMinX() const;
    /** @brief Retrieve the upper boundary of the plot in x-direction */
    G_API_COMMON double getMaxX() const;
    /** @brief Retrieve the lower boundary of the plot in y-direction */
    G_API_COMMON double getMinY() const;
    /** @brief Retrieve the upper boundary of the plot in y-direction */
    G_API_COMMON double getMaxY() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

    /** @brief Allows to specify 2d-drawing options */
    G_API_COMMON void set2DOpt(tddropt);
    /** @brief Allows to retrieve 2d-drawing options */
    G_API_COMMON tddropt get2DOpt() const;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GHistogram2D>(
        GHistogram2D const &
        , GHistogram2D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    G_API_COMMON GHistogram2D() = default; ///< The default constructor -- intentionally private, as it is only needed for (de-)serialization

    std::size_t nBinsX_ = 0; ///< The number of bins in the x-direction of the histogram
    std::size_t nBinsY_ = 0; ///< The number of bins in the y-direction of the histogram

    double minX_ = 0.; ///< The lower boundary of the histogram in x-direction
    double maxX_ = 0.; ///< The upper boundary of the histogram in x-direction
    double minY_ = 0.; ///< The lower boundary of the histogram in y-direction
    double maxY_ = 0.; ///< The upper boundary of the histogram in y-direction

    tddropt dropt_ = tddropt::BOX; ///< The drawing options for 2-d histograms
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TGraph class (2d data and curve-like structures). It
 * also adds the option to draw arrows between consecutive points. This results
 * in a 2D plot.
 */
class GGraph2D
    : public GDataCollector2T<double, double>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector2T_double_double"
            , boost::serialization::base_object<GDataCollector2T<double, double>>(*this))
        & BOOST_SERIALIZATION_NVP(pM_)
        & BOOST_SERIALIZATION_NVP(drawArrows_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_COMMON GGraph2D() = default;
    G_API_COMMON GGraph2D(GGraph2D const &) = default;
    G_API_COMMON GGraph2D(GGraph2D &&) = default;
    G_API_COMMON ~GGraph2D() override = default;

    G_API_COMMON GGraph2D& operator=(GGraph2D const&) = default;
    G_API_COMMON GGraph2D& operator=(GGraph2D &&) = default;

    /**********************************************************************/

    /** @brief Adds arrows to the plots between consecutive points */
    G_API_COMMON void setDrawArrows(bool= true);
    /** @brief Retrieves the value of the drawArrows_ variable */
    G_API_COMMON bool getDrawArrows() const;

    /** @brief Determines whether a scatter plot or a curve is created */
    G_API_COMMON void setPlotMode(graphPlotMode);
    /** @brief Allows to retrieve the current plotting mode */
    G_API_COMMON graphPlotMode getPlotMode() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph2D>(
        GGraph2D const &
        , GGraph2D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    graphPlotMode pM_ = DEFPLOTMODE; ///< Whether to create scatter plots or a curve, connected by lines
    bool drawArrows_ = false; ///< When set to true, arrows will be drawn between consecutive points
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TGraphErrors class (2d data and curve-like structures).
 * This results in a 2D plot.
 */
class GGraph2ED
    : public GDataCollector2ET<double, double>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector2ET_double_double"
            , boost::serialization::base_object<GDataCollector2ET<double, double>>(*this))
        & BOOST_SERIALIZATION_NVP(pM_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_COMMON GGraph2ED() = default;
    G_API_COMMON GGraph2ED(GGraph2ED const &) = default;
    G_API_COMMON GGraph2ED(GGraph2ED &&) = default;
    G_API_COMMON ~GGraph2ED() override = default;

    G_API_COMMON GGraph2ED& operator=(GGraph2ED const&) = default;
    G_API_COMMON GGraph2ED& operator=(GGraph2ED &&) = default;

    /**********************************************************************/

    /** @brief Determines whether a scatter plot or a curve is created */
    G_API_COMMON void setPlotMode(graphPlotMode);
    /** @brief Allows to retrieve the current plotting mode */
    G_API_COMMON graphPlotMode getPlotMode() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph2ED>(
        GGraph2ED const &
        , GGraph2ED const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    graphPlotMode pM_ = DEFPLOTMODE; ///< Whether to create scatter plots or a curve, connected by lines
};

/******************************************************************************/
/**
 * A data collector for 3-d data of user-defined type
 */
template<typename x_type, typename y_type, typename z_type>
class GDataCollector3T
    : public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(m_data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector3T() = default;
    GDataCollector3T(GDataCollector3T<x_type, y_type, z_type> const &) = default;
    GDataCollector3T(GDataCollector3T<x_type, y_type, z_type> &&) = default;

    ~GDataCollector3T() override = default;

    GDataCollector3T<x_type, y_type, z_type>& operator=(GDataCollector3T<x_type, y_type, z_type> const&) = default;
    GDataCollector3T<x_type, y_type, z_type>& operator=(GDataCollector3T<x_type, y_type, z_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (x-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<x_type>> projectX(
        std::size_t, std::tuple<x_type, x_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector3T<>::projectX(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<x_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (y-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<y_type>> projectY(
        std::size_t, std::tuple<y_type, y_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector3T<>::projectY(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<y_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (z-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<z_type>> projectZ(
        std::size_t, std::tuple<z_type, z_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector3T<>::projectZ(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<z_type>>();
    }

    /***************************************************************************/
    /**
	  * This very simple functions allows derived classes
	  * to add data easily to their data sets, when called through a
	  * pointer. I.e., this makes object_ptr->add(data) instead of
	  * *object_ptr & data possible.
	  */
    template<typename data_type>
    void add(const data_type &item) {
        *this & item;
    }

    /***************************************************************************/
    /**
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @param point_undet The data item to be added to the collection
	  */
    template<typename x_type_undet, typename y_type_undet, typename z_type_undet>
    void operator&(const std::tuple<x_type_undet, y_type_undet, z_type_undet> &point_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = boost::numeric_cast<x_type>(std::get<0>(point_undet));
            y = boost::numeric_cast<y_type>(std::get<1>(point_undet));
            z = boost::numeric_cast<z_type>(std::get<2>(point_undet));
        }
        catch (bad_numeric_cast &e) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GDataCollector3T::operator&(const std::tuple<S,T,U>&): Error!" << std::endl
                    << "Encountered invalid cast with boost::numeric_cast," << std::endl
                    << "with the message " << std::endl
                    << e.what() << std::endl
            );
        }

        m_data.push_back(
            std::tuple<x_type, y_type, z_type>(
                x
                , y
                , z
            ));
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type std::tuple<x_type, y_type, z_type> to the collection
	  * in an intuitive way.
	  *
	  * @param point The data item to be added to the collection
	  */
    void operator&(const std::tuple<x_type, y_type, z_type> &point) {
        // Add the data item to the collection
        m_data.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template<typename x_type_undet, typename y_type_undet, typename z_type_undet>
    void operator&(const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>> &point_cnt_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);

        typename std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>>::const_iterator cit;
        for (cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = boost::numeric_cast<x_type>(std::get<0>(*cit));
                y = boost::numeric_cast<y_type>(std::get<1>(*cit));
                z = boost::numeric_cast<z_type>(std::get<2>(*cit));
            }
            catch (bad_numeric_cast &e) {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GDataCollector3T::operator&(const std::vector<std::tuple<S,T,U>>&): Error!" << std::endl
                        << "Encountered invalid cast with boost::numeric_cast," << std::endl
                        << "with the message " << std::endl
                        << e.what() << std::endl
                );
            }

            m_data.push_back(
                std::tuple<x_type, y_type, z_type>(
                    x
                    , y
                    , z
                ));
        }
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of type std::tuple<x_type, y_type, z_type>
	  * to the collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt The collection of data items to be added to the collection
	  */
    void operator&(const std::vector<std::tuple<x_type, y_type, z_type>> &point_cnt) {
        typename std::vector<std::tuple<x_type, y_type, z_type>>::const_iterator cit;
        for (cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            m_data.push_back(*cit);
        }
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector3T<x_type, y_type, z_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own
        m_data = p_load->m_data; // This assumes that x_type is POD
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector3T<x_type, y_type, z_type>>(
        GDataCollector3T<x_type, y_type, z_type> const &
        , GDataCollector3T<x_type, y_type, z_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GBasePlotter &cp
        , const expectation &e
        , const double &limit
    ) const override {
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDataCollector3T<x_type, y_type, z_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(m_data
                     , p_load->m_data)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, y_type, z_type>> m_data; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDataCollector3T<x_type, y_type, z_type>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override = 0;
};


/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type> = <double, double, double>, that will return a
 * GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector3T<double, double, double>::projectX(std::size_t nBinsX, std::tuple<double, double> rangeX) const {
    std::tuple<double, double> myRangeX;
    std::tuple<double, double> default_range;
    if (rangeX == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(this->m_data);
        myRangeX = std::tuple<double, double>(
            std::get<0>(extremes)
            , std::get<1>(extremes));
    } else {
        myRangeX = rangeX;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsX
            , myRangeX
        ));
    result->setXAxisLabel(this->xAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / x-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<0>(o);
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
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector3T<double, double, double>::projectY(std::size_t nBinsY, std::tuple<double, double> rangeY) const {
    std::tuple<double, double> myRangeY;
    std::tuple<double, double> default_range;
    if (rangeY == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(m_data);
        myRangeY = std::tuple<double, double>(
            std::get<2>(extremes)
            , std::get<3>(extremes));
    } else {
        myRangeY = rangeY;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsY
            , myRangeY
        ));
    result->setXAxisLabel(this->yAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / y-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<1>(o);
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
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector3T<double, double, double>::projectZ(std::size_t nBinsZ, std::tuple<double, double> rangeZ) const {
    std::tuple<double, double> myRangeZ;
    std::tuple<double, double> default_range;
    if (rangeZ == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(m_data);
        myRangeZ = std::tuple<double, double>(
            std::get<4>(extremes)
            , std::get<5>(extremes));
    } else {
        myRangeZ = rangeZ;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsZ
            , myRangeZ
        ));
    result->setXAxisLabel(this->zAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / z-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<2>(o);
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
class GGraph3D
    : public GDataCollector3T<double, double, double>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector3T_3double"
            , boost::serialization::base_object<GDataCollector3T<double, double, double>>(*this))
        & BOOST_SERIALIZATION_NVP(drawLines_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_COMMON GGraph3D() = default;
    G_API_COMMON GGraph3D(GGraph3D const &) = default;
    G_API_COMMON GGraph3D(GGraph3D &&) = default;
    G_API_COMMON ~GGraph3D() override = default;

    G_API_COMMON GGraph3D& operator=(GGraph3D const&) = default;
    G_API_COMMON GGraph3D& operator=(GGraph3D &&) = default;

    /*********************************************************************/

    /** @brief Adds lines to the plots between consecutive points */
    G_API_COMMON void setDrawLines(bool= true);
    /** @brief Retrieves the value of the drawLines_ variable */
    G_API_COMMON bool getDrawLines() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph3D>(
        GGraph3D const &
        , GGraph3D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    bool drawLines_ = false; ///< When set to true, lines will be drawn between consecutive points
};

/******************************************************************************/
/**
 * A data collector for 4-d data of user-defined type
 */
template<
    typename x_type, typename y_type, typename z_type, typename w_type
        >
class GDataCollector4T
    : public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(m_data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector4T() = default;
    GDataCollector4T(GDataCollector4T<x_type, y_type, z_type, w_type> const&) = default;
    GDataCollector4T(GDataCollector4T<x_type, y_type, z_type, w_type> &&) = default;
    ~GDataCollector4T() override = default;

    GDataCollector4T<x_type, y_type, z_type, w_type>& operator=(GDataCollector4T<x_type, y_type, z_type, w_type> const&) = default;
    GDataCollector4T<x_type, y_type, z_type, w_type>& operator=(GDataCollector4T<x_type, y_type, z_type, w_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (x-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<x_type>> projectX(
        std::size_t, std::tuple<x_type, x_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector4T<>::projectX(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<x_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (y-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<y_type>> projectY(
        std::size_t, std::tuple<y_type, y_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector4T<>::projectY(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<y_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (z-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<z_type>> projectZ(
        std::size_t, std::tuple<z_type, z_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector4T<>::projectZ(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<z_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (w-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  */
    std::shared_ptr<GDataCollector1T<w_type>> projectW(
        std::size_t, std::tuple<w_type, w_type>
    ) const {
        throw gemfony_exception(
            g_error_streamer(
                DO_LOG
                , time_and_place
            )
                << "In GDataCollector4T<>::projectZ(range, nBins): Error!" << std::endl
                << "Function was called for class with un-implemented types" << std::endl
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<w_type>>();
    }

    /***************************************************************************/
    /**
	  * This very simple functions allows derived classes
	  * to add data easily to their data sets, when called through a
	  * pointer. I.e., this makes object_ptr->add(data) instead of
	  * *object_ptr & data possible.
	  */
    template<typename data_type>
    void add(const data_type &item) {
        *this & item;
    }

    /***************************************************************************/
    /**
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @param point_undet The data item to be added to the collection
	  */
    template<
        typename x_type_undet, typename y_type_undet, typename z_type_undet, typename w_type_undet
            >
    void operator&(const std::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet> &point_undet) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);
        w_type w = w_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = boost::numeric_cast<x_type>(std::get<0>(point_undet));
            y = boost::numeric_cast<y_type>(std::get<1>(point_undet));
            z = boost::numeric_cast<z_type>(std::get<2>(point_undet));
            w = boost::numeric_cast<w_type>(std::get<3>(point_undet));
        }
        catch (bad_numeric_cast &e) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GDataCollector4T::operator&(const std::tuple<S,T,U,W>&): Error!" << std::endl
                    << "Encountered invalid cast with boost::numeric_cast," << std::endl
                    << "with the message " << std::endl
                    << e.what() << std::endl
            );
        }

        m_data.push_back(
            std::tuple<x_type, y_type, z_type, w_type>(
                x
                , y
                , z
                , w
            ));
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type std::tuple<x_type, y_type, z_type> to the collection
	  * in an intuitive way.
	  *
	  * @param point The data item to be added to the collection
	  */
    void operator&(const std::tuple<x_type, y_type, z_type, w_type> &point) {
        // Add the data item to the collection
        m_data.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template<
        typename x_type_undet, typename y_type_undet, typename z_type_undet, typename w_type_undet
            >
    void operator&(
        const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet>> &point_cnt_undet
    ) {
        using boost::numeric::bad_numeric_cast;

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);
        w_type w = w_type(0);

        typename std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>>::const_iterator cit;
        for (cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = boost::numeric_cast<x_type>(std::get<0>(*cit));
                y = boost::numeric_cast<y_type>(std::get<1>(*cit));
                z = boost::numeric_cast<z_type>(std::get<2>(*cit));
                w = boost::numeric_cast<w_type>(std::get<3>(*cit));
            }
            catch (bad_numeric_cast &e) {
                throw gemfony_exception(
                    g_error_streamer(
                        DO_LOG
                        , time_and_place
                    )
                        << "In GDataCollector4T::operator&(const std::vector<std::tuple<S,T,U,W>>&): Error!"
                        << std::endl
                        << "Encountered invalid cast with boost::numeric_cast," << std::endl
                        << "with the message " << std::endl
                        << e.what() << std::endl
                );
            }

            m_data.push_back(
                std::tuple<x_type, y_type, z_type, w_type>(
                    x
                    , y
                    , z
                    , w
                ));
        }
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of type std::tuple<x_type, y_type, z_type, w_type>
	  * to the collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @param point_cnt The collection of data items to be added to the collection
	  */
    void operator&(const std::vector<std::tuple<x_type, y_type, z_type, w_type>> &point_cnt) {
        typename std::vector<std::tuple<x_type, y_type, z_type, w_type>>::const_iterator cit;
        for (cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            m_data.push_back(*cit);
        }
    }

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another object
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector4T<x_type, y_type, z_type, w_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
                cp
                , this
            );

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own
        m_data = p_load->m_data; // This assumes that x_type is POD
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector4T<x_type, y_type, z_type, w_type>>(
        GDataCollector4T<x_type, y_type, z_type, w_type> const &
        , GDataCollector4T<x_type, y_type, z_type, w_type> const &
        , GToken &
    );

    /***************************************************************************/
    /**
	  * Investigates compliance with expectations with respect to another object
	  * of the same type
	  */
    void compare_(
        const GBasePlotter &cp
        , const expectation &e
        , const double &limit
    ) const override {
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(
            cp
            , this
        );

        GToken token(
            "GDataCollector4T<x_type, y_type, z_type, w_type>"
            , e
        );

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(m_data
                     , p_load->m_data)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, y_type, z_type, w_type>> m_data; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  */
    std::string name_() const override {
        return std::string("GDataCollector4T<x_type, y_type, z_type, w_type>");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override = 0;
};

/******************************************************************************/
/**
 * Specialization of projectX for <x_type, y_type, z_type, w_type> = <double, double, double, double>,
 * that will return a GHistogram1D object, wrapped into a std::shared_ptr<GHistogram1D>. In case of a
 * default-constructed range, the function will attempt to determine suitable parameters
 * for the range settings.
 *
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectX(
    std::size_t nBinsX
    , std::tuple<double, double> rangeX
) const {
    std::tuple<double, double> myRangeX;
    std::tuple<double, double> default_range;
    if (rangeX == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double, double, double> extremes = getMinMax(
            this->m_data
        );
        myRangeX = std::tuple<double, double>(
            std::get<0>(extremes)
            , std::get<1>(extremes));
    } else {
        myRangeX = rangeX;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsX
            , myRangeX
        ));
    result->setXAxisLabel(this->xAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / x-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<0>(o);
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
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectY(
    std::size_t nBinsY
    , std::tuple<double, double> rangeY
) const {
    std::tuple<double, double> myRangeY;
    std::tuple<double, double> default_range;
    if (rangeY == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double, double, double> extremes = getMinMax(
            this->m_data
        );
        myRangeY = std::tuple<double, double>(
            std::get<2>(extremes)
            , std::get<3>(extremes));
    } else {
        myRangeY = rangeY;
    }

    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsY
            , myRangeY
        ));
    result->setXAxisLabel(this->yAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / y-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<1>(o);
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
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectZ(
    std::size_t nBinsZ, std::tuple<double, double> rangeZ
) const {
    std::tuple<double, double> myRangeZ;
    std::tuple<double, double> default_range;
    if (rangeZ == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double, double, double> extremes = getMinMax(
            this->m_data
        );
        myRangeZ = std::tuple<double, double>(
            std::get<4>(extremes)
            , std::get<5>(extremes));
    } else {
        myRangeZ = rangeZ;
    }


    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsZ
            , myRangeZ
        ));
    result->setXAxisLabel(this->zAxisLabel());
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / z-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<2>(o);
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
 * @param nBins The number of bins of the histogram
 * @param range The minimum and maximum boundaries of the histogram
 */
template<>
inline
std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectW(
    std::size_t nBinsW
    , std::tuple<double, double> rangeW
) const {
    std::tuple<double, double> myRangeW;
    std::tuple<double, double> default_range;
    if (rangeW == default_range) {
        // Find out about the minimum and maximum values in the m_data array
        std::tuple<double, double, double, double, double, double, double, double> extremes = getMinMax(
            this->m_data
        );
        myRangeW = std::tuple<double, double>(
            std::get<6>(extremes)
            , std::get<7>(extremes));
    } else {
        myRangeW = rangeW;
    }


    // Construct the result object
    std::shared_ptr<GHistogram1D> result(
        new GHistogram1D(
            nBinsW
            , myRangeW
        ));
    result->setXAxisLabel("w");
    result->setYAxisLabel("Number of entries");
    result->setPlotLabel(this->plotLabel() + " / w-projection");

    // Add data to the object
    for (auto const &o: m_data) {
        (*result) & std::get<3>(o);
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
class GGraph4D
    : public GDataCollector4T<double, double, double, double>
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GDataCollector4T_4double"
            , boost::serialization::base_object<GDataCollector4T<double, double, double, double>>(*this))
        & BOOST_SERIALIZATION_NVP(minMarkerSize_)
        & BOOST_SERIALIZATION_NVP(maxMarkerSize_)
        & BOOST_SERIALIZATION_NVP(smallWLargeMarker_)
        & BOOST_SERIALIZATION_NVP(nBest_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_COMMON GGraph4D() = default;
    G_API_COMMON GGraph4D(const GGraph4D &) = default;
    G_API_COMMON GGraph4D(GGraph4D &&) = default;
    G_API_COMMON ~GGraph4D() override = default;

    G_API_COMMON GGraph4D& operator=(GGraph4D const&) = default;
    G_API_COMMON GGraph4D& operator=(GGraph4D &&) = default;

    /*********************************************************************/

    /** @brief Allows to set the minimum marker size */
    G_API_COMMON void setMinMarkerSize(const double &);
    /** @brief Allows to set the maximum marker size */
    G_API_COMMON void setMaxMarkerSize(const double &);

    /** @brief Allows to retrieve the minimum marker size */
    G_API_COMMON double getMinMarkerSize() const;
    /** @brief Allows to retrieve the maximum marker size */
    G_API_COMMON double getMaxMarkerSize() const;

    /** @brief Allows to specify whether small w yield large markers */
    G_API_COMMON void setSmallWLargeMarker(const bool &);
    /** @brief Allows to check whether small w yield large markers */
    G_API_COMMON bool getSmallWLargeMarker() const;

    /** @brief Allows to set the number of solutions the class should show */
    G_API_COMMON void setNBest(const std::size_t &);
    /** @brief Allows to retrieve the number of solutions the class should show */
    G_API_COMMON std::size_t getNBest() const;

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GGraph4D>(
        GGraph4D const &
        , GGraph4D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    double minMarkerSize_ = DEFMINMARKERSIZE; ///< The minimum allowed size of the marker
    double maxMarkerSize_ = DEFMAXMARKERSIZE; ///< The maximum allowed size of the marker

    bool smallWLargeMarker_ = true; ///< Indicates whether a small w value yields a large marker

    std::size_t nBest_ = 0; ///< Determines the number of items the class should show
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A wrapper for the ROOT TF1 1d-function plotter.
 * TODO: Add ability to add markers!
 */
class GFunctionPlotter1D
    : public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(functionDescription_)
        & BOOST_SERIALIZATION_NVP(xExtremes_)
        & BOOST_SERIALIZATION_NVP(nSamplesX_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    G_API_COMMON GFunctionPlotter1D(
        const std::string &, const std::tuple<double, double> &
    );

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    G_API_COMMON GFunctionPlotter1D(GFunctionPlotter1D const&) = default;
    G_API_COMMON GFunctionPlotter1D(GFunctionPlotter1D &&) = default;
    G_API_COMMON ~GFunctionPlotter1D() override = default;

    G_API_COMMON GFunctionPlotter1D& operator=(GFunctionPlotter1D const&) = default;
    G_API_COMMON GFunctionPlotter1D& operator=(GFunctionPlotter1D &&) = default;

    /*********************************************************************/

    /** @brief Allows to set the number of sampling points in x-direction */
    G_API_COMMON void setNSamplesX(std::size_t);

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GFunctionPlotter1D>(
        GFunctionPlotter1D const &
        , GFunctionPlotter1D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    G_API_COMMON GFunctionPlotter1D() = default; ///< The default constructor. Intentionally private, as it is only needed for (de-)serialization

    std::string functionDescription_;

    std::tuple<double, double> xExtremes_; ///< Minimum and maximum values for the x-axis
    std::size_t nSamplesX_ = DEFNSAMPLES; ///< The number of sampling points of the function
};

/******************************************************************************/
/**
 * A wrapper for the ROOT TF2 2d-function plotter
 */
class GFunctionPlotter2D
    : public GBasePlotter
{

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter)
        & BOOST_SERIALIZATION_NVP(functionDescription_)
        & BOOST_SERIALIZATION_NVP(xExtremes_)
        & BOOST_SERIALIZATION_NVP(yExtremes_)
        & BOOST_SERIALIZATION_NVP(nSamplesX_)
        & BOOST_SERIALIZATION_NVP(nSamplesY_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    G_API_COMMON GFunctionPlotter2D(
        const std::string &, const std::tuple<double, double> &, const std::tuple<double, double> &
    );

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    G_API_COMMON GFunctionPlotter2D(GFunctionPlotter2D const&) = default;
    G_API_COMMON GFunctionPlotter2D(GFunctionPlotter2D &&) = default;
    G_API_COMMON ~GFunctionPlotter2D() override = default;

    G_API_COMMON GFunctionPlotter2D& operator=(GFunctionPlotter2D const&) = default;
    G_API_COMMON GFunctionPlotter2D& operator=(GFunctionPlotter2D &&) = default;

    /*********************************************************************/

    /** @brief Allows to set the number of sampling points in x-direction */
    G_API_COMMON void setNSamplesX(std::size_t);
    /** @brief Allows to set the number of sampling points in y-direction */
    G_API_COMMON void setNSamplesY(std::size_t);

    /** @brief Retrieves a unique name for this plotter */
    G_API_COMMON std::string getPlotterName() const override;

protected:
    /** @brief Retrieve specific header settings for this plot */
    std::string headerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves the actual data sets */
    std::string bodyData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieves specific draw commands for this plot */
    std::string footerData_(bool, std::size_t, const std::string &) const override;

    /** @brief Retrieve the current drawing arguments */
    std::string drawingArguments(bool) const override;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GFunctionPlotter2D>(
        GFunctionPlotter2D const &
        , GFunctionPlotter2D const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GBasePlotter & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

private:
    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GBasePlotter *clone_() const override;

    G_API_COMMON GFunctionPlotter2D() = default; ///< The default constructor -- intentionally private, as it is only needed for (de-)serialization

    std::string functionDescription_;

    std::tuple<double, double> xExtremes_; ///< Minimum and maximum values for the x-axis
    std::tuple<double, double> yExtremes_; ///< Minimum and maximum values for the y-axis

    std::size_t nSamplesX_ = DEFNSAMPLES; ///< The number of sampling points of the function
    std::size_t nSamplesY_ = DEFNSAMPLES; ///< The number of sampling points of the function
};

/******************************************************************************/
/**
 * A class that outputs a ROOT input file (compare http://root.cern.ch), based
 * on the data providers stored in it.
 */
class GPlotDesigner
    : public GCommonInterfaceT<GPlotDesigner>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_NVP(m_c_x_div)
        & BOOST_SERIALIZATION_NVP(m_c_y_div)
        & BOOST_SERIALIZATION_NVP(m_c_x_dim)
        & BOOST_SERIALIZATION_NVP(m_c_y_dim)
        & BOOST_SERIALIZATION_NVP(m_canvas_label)
        & BOOST_SERIALIZATION_NVP(m_add_print_command)
        & BOOST_SERIALIZATION_NVP(m_n_indention_spaces);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    G_API_COMMON GPlotDesigner(
        const std::string &
        , const std::size_t &
        , const std::size_t &
    );

    /** @brief Copy constructor */
    G_API_COMMON GPlotDesigner(GPlotDesigner const &);
    /** @brief Assignment operator */
    G_API_COMMON GPlotDesigner& operator=(GPlotDesigner const &);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    G_API_COMMON GPlotDesigner(GPlotDesigner &&) = default;
    G_API_COMMON virtual ~GPlotDesigner() = default;

    G_API_COMMON GPlotDesigner& operator=(GPlotDesigner &&) = default;

    /*********************************************************************/

    /* @brief Emits the overall plot */
    G_API_COMMON std::string plot(const boost::filesystem::path & = boost::filesystem::path("empty")) const;
    /** @brief Writes the plot to a file */
    G_API_COMMON void writeToFile(const boost::filesystem::path &);

    /** @brief Allows to add a new plotter object */
    G_API_COMMON void registerPlotter(std::shared_ptr<GBasePlotter>);

    /** @brief Set the dimensions of the output canvas */
    G_API_COMMON void setCanvasDimensions(const std::uint32_t &, const std::uint32_t &);
    /** @brief Set the dimensions of the output canvas */
    G_API_COMMON void setCanvasDimensions(const std::tuple<std::uint32_t, std::uint32_t> &);
    /** @brief Allows to retrieve the canvas dimensions */
    G_API_COMMON std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const;

    /** @brief Allows to set the canvas label */
    G_API_COMMON void setCanvasLabel(const std::string &);
    /** @brief Allows to retrieve the canvas label */
    G_API_COMMON std::string getCanvasLabel() const;

    /** @brief Allows to add a "Print" command to the end of the script so that picture files are created */
    G_API_COMMON void setAddPrintCommand(bool);
    /** @brief Allows to retrieve the current value of the addPrintCommand_ variable */
    G_API_COMMON bool getAddPrintCommand() const;

    /** @brief Resets the plotters */
    G_API_COMMON void resetPlotters();

    /** @brief Allows to set the number of spaces used for indention */
    void setNIndentionSpaces(const std::size_t &);

    /** @brief Allows to retrieve the number spaces used for indention */
    std::size_t getNIndentionSpaces() const;

    /** @brief Returns the current number of indention spaces as a string */
    std::string indent() const;

protected:
    /** @brief A header for static data in a ROOT file */
    std::string staticHeader(const std::string &) const;

    /** @brief Loads the data of another object */
    G_API_COMMON void load_(const GPlotDesigner *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GPlotDesigner>(
        GPlotDesigner const &
        , GPlotDesigner const &
        , GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_COMMON void compare_(
        const GPlotDesigner & // the other object
        , const expectation & // the expectation for this object, e.g. equality
        , const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    G_API_COMMON bool modify_GUnitTests_() override { return false; }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_COMMON void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_COMMON void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /** @brief The default constructor -- only needed for (de-)serialization */
    GPlotDesigner() = default;

    /** @brief Returns the name of this class */
    G_API_COMMON std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    G_API_COMMON GPlotDesigner *clone_() const override;

    std::vector<std::shared_ptr<GBasePlotter>>
        m_plotters_cnt = std::vector<std::shared_ptr<GBasePlotter>>(); ///< A list of plots to be added to the diagram

    std::size_t m_c_x_div = 1, m_c_y_div = 1; ///< The number of divisions in x- and y-direction
    std::uint32_t m_c_x_dim = DEFCXDIM, m_c_y_dim = DEFCYDIM; ///< Holds the number of pixels of the canvas

    std::string m_canvas_label = std::string("empty"); ///< A label to be assigned to the entire canvas

    bool m_add_print_command = false; ///< Indicates whether a print command for the creation of a png file should be added

    std::size_t m_n_indention_spaces = std::size_t(DEFNINDENTIONSPACES);
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
// Declare abstract or export class names for Boost.Serialization
namespace boost {
namespace serialization {

template<typename coordinate_type>
struct is_abstract<Gem::Common::GDecorator<Gem::Common::dimensions::Dim2, coordinate_type>> : public boost::true_type
{ /* nothing */ };
template<typename coordinate_type>
struct is_abstract<const Gem::Common::GDecorator<Gem::Common::dimensions::Dim2, coordinate_type>> : public boost::true_type
{ /* nothing */ };

template<typename coordinate_type>
struct is_abstract<Gem::Common::GDecorator<Gem::Common::dimensions::Dim3, coordinate_type>> : public boost::true_type
{ /* nothing */ };
template<typename coordinate_type>
struct is_abstract<const Gem::Common::GDecorator<Gem::Common::dimensions::Dim3, coordinate_type>> : public boost::true_type
{ /* nothing */ };

template<typename coordinate_type>
struct is_abstract<Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim2, coordinate_type>> : public boost::true_type
{ /* nothing */ };
template<typename coordinate_type>
struct is_abstract<const Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim2, coordinate_type>> : public boost::true_type
{ /* nothing */ };

template<typename coordinate_type>
struct is_abstract<Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim3, coordinate_type>> : public boost::true_type
{ /* nothing */ };
template<typename coordinate_type>
struct is_abstract<const Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim3, coordinate_type>> : public boost::true_type
{ /* nothing */ };

template<typename x_type>
struct is_abstract<Gem::Common::GDataCollector1T<x_type>> : public boost::true_type
{ /* nothing */ };
template<typename x_type>
struct is_abstract<const Gem::Common::GDataCollector1T<x_type>> : public boost::true_type
{ /* nothing */ };

template<typename x_type, typename y_type>
struct is_abstract<Gem::Common::GDataCollector2T<x_type,y_type>> : public boost::true_type
{ /* nothing */ };
template<typename x_type, typename y_type>
struct is_abstract<const Gem::Common::GDataCollector2T<x_type, y_type>> : public boost::true_type
{ /* nothing */ };

template<typename x_type, typename y_type>
struct is_abstract<Gem::Common::GDataCollector2ET<x_type,y_type>> : public boost::true_type
{ /* nothing */ };
template<typename x_type, typename y_type>
struct is_abstract<const Gem::Common::GDataCollector2ET<x_type, y_type>> : public boost::true_type
{ /* nothing */ };

template<typename x_type, typename y_type, typename z_type>
struct is_abstract<Gem::Common::GDataCollector3T<x_type, y_type, z_type>> : public boost::true_type
{ /* nothing */ };
template<typename x_type, typename y_type, typename z_type>
struct is_abstract<const Gem::Common::GDataCollector3T<x_type, y_type, z_type>> : public boost::true_type
{ /* nothing */ };

template<typename x_type, typename y_type, typename z_type, typename w_type>
struct is_abstract<Gem::Common::GDataCollector4T<x_type, y_type, z_type, w_type>> : public boost::true_type
{ /* nothing */ };
template<typename x_type, typename y_type, typename z_type, typename w_type>
struct is_abstract<const Gem::Common::GDataCollector4T<x_type, y_type, z_type, w_type>> : public boost::true_type
{ /* nothing */ };

} /* namespace serialization */
} /* namespace boost */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(GBasePlotter)

BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<short>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<std::int32_t>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<std::uint32_t>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<float>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<double>);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<short>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<std::int32_t>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<std::uint32_t>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<float>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<double>);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<short>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<std::int32_t>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<std::uint32_t>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<float>);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<double>);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GHistogram1D);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GHistogram1I);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GHistogram2D);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph2D);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph2ED);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph3D);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph4D);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GFunctionPlotter1D);
BOOST_CLASS_EXPORT_KEY(Gem::Common::GFunctionPlotter2D);

BOOST_CLASS_EXPORT_KEY(Gem::Common::GPlotDesigner);
