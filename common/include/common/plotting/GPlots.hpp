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
 * A data collector for 1-d data of user-defined type. This will usually be
 * data of a histogram type. It is assumed to be movable, hence we use all
 * defaulted constructors and assignment operators.
 *
 * @tparam x_type The numeric type of the 1-d data items stored in this collector
 */
template <typename x_type>
class GDataCollector1T : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) & BOOST_SERIALIZATION_NVP(data_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector1T() = default;
    GDataCollector1T(GDataCollector1T<x_type> const &) = default;
    GDataCollector1T(GDataCollector1T<x_type> &&) = default;

    ~GDataCollector1T() override = default;

    GDataCollector1T<x_type> &operator=(GDataCollector1T<x_type> const &) = default;
    GDataCollector1T<x_type> &operator=(GDataCollector1T<x_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to retrieve information about the amount of data sets stored in
	  * this object
	  *
	  * @return The number of data items currently stored in this collector
	  */
    std::size_t currentSize() const {
        return data_.size();
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
	  * Allows to add data of arbitrary type, provided it can be converted
	  * safely to the target type.
	  *
	  * @tparam x_type_undet The source type of the data item, narrowed to x_type
	  * @param x_undet The data item to be added to the collection
	  */
    template <typename x_type_undet>
    void operator&(const x_type_undet &x_undet) {

        x_type x = x_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = Gem::Common::narrow<x_type>(x_undet);
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollector1T<x_type>::operator&(const T&): Error!" << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }

        // Add the converted data to our collection
        data_.push_back(x);
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type "x_type
	  *
	  * @param x The data item to be added to the collection
	  */
    void operator&(const x_type &x) {
        // Add the data item to our collection
        data_.push_back(x);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type in one go,
	  * provided the type can be converted safely into the target type
	  *
	  * @tparam x_type_undet The source element type of the vector, narrowed to x_type
	  * @param x_cnt_undet A collection of data items of undetermined type, to be added to the collection
	  */
    template <typename x_type_undet>
    void operator&(const std::vector<x_type_undet> &x_cnt_undet) {

        x_type x = x_type(0);

        typename std::vector<x_type_undet>::const_iterator cit;
        for(cit = x_cnt_undet.begin(); cit != x_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<x_type>(*cit);
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollector1T::operator&(const std::vector<T>&): Error!" << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            // Add the converted data to our collection
            data_.push_back(x);
        }
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of type x_type to our data_ vector.
	  *
	  * @param x_cnt A vector of data items to be added to the data_ vector
	  */
    void operator&(const std::vector<x_type> &x_cnt) {
        typename std::vector<x_type>::const_iterator cit;
        for(cit = x_cnt.begin(); cit != x_cnt.end(); ++cit) {
            // Add the data item to our collection
            data_.push_back(*cit);
        }
    }

    /***************************************************************************/
    /**
	  * Retrieves the minimum and maximum values in data_
	  *
	  * @return A tuple holding the minimum and the maximum element found in the data
	  */
    std::tuple<x_type, x_type> getMinMaxElements() const {
        auto minmax = std::minmax_element(data_.begin(), data_.end());
        return std::make_tuple(*minmax.first, *minmax.second);
    };

protected:
    /***************************************************************************/
    /**
	  * Single declaration of this class'es local data members, used by load_() and compare_()
	  *
	  * @return A tuple of named members (the data vector) of this object
	  */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(make_member("data_", self.data_));
    }

    /**
	  * Loads the data of another object
	  *
	  * @param cp A pointer to another GDataCollector1T<x_type> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector1T<x_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own, derived from the single localMembers() declaration
        g_load_members(localMembers_(*this), localMembers_(*p_load));
    }

    /***************************************************************************/

    friend void compare_base_t<GDataCollector1T<x_type>>(
        GDataCollector1T<x_type> const &,
        GDataCollector1T<x_type> const &,
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
        // Check that we are dealing with a GDataCollector1T<x_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollector1T<x_type>", e);

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<x_type> data_; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  *
	  * @return The name of this class as a string
	  */
    std::string name_() const override {
        return std::string("GDataCollector1T<x_type>");
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
 * A wrapper for ROOT's TH1D class (1-d double data). This will result in a 2D-plot.
 */
class GHistogram1D : public GDataCollector1T<double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
 * A data collector for 2-d data of user-defined type, such as a TGraph.
 * Note that the plot dimension may be different.
 *
 * @tparam x_type The numeric type of the x-component of each data item
 * @tparam y_type The numeric type of the y-component of each data item
 */
template <typename x_type, typename y_type>
class GDataCollector2T : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) & BOOST_SERIALIZATION_NVP(data_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector2T() = default;
    GDataCollector2T(GDataCollector2T<x_type, y_type> const &) = default;
    GDataCollector2T(GDataCollector2T<x_type, y_type> &&) = default;

    ~GDataCollector2T() override = default;

    GDataCollector2T<x_type, y_type> &operator=(GDataCollector2T<x_type, y_type> const &) = default;
    GDataCollector2T<x_type, y_type> &operator=(GDataCollector2T<x_type, y_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to retrieve information about the amount of data sets stored in
	  * this object
	  *
	  * @return The number of data items currently stored in this collector
	  */
    std::size_t currentSize() const {
        return data_.size();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (x-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the x-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<x_type>>
    projectX(std::size_t nBins, std::tuple<x_type, x_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector2T<>::projectX(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<x_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (y-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the y-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<y_type>>
    projectY(std::size_t nBins, std::tuple<y_type, y_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector2T<>::projectY(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
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
	  *
	  * @tparam data_type1 The type of the x-component being added
	  * @tparam data_type2 The type of the y-component being added
	  * @param item1 The x-component of the data point to be added
	  * @param item2 The y-component of the data point to be added
	  */
    template <typename data_type1, typename data_type2>
    void add(const data_type1 &item1, const data_type2 &item2) {
        *this &std::make_tuple(item1, item2);
    }

    /***************************************************************************/
    /**
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component, narrowed to y_type
	  * @param point_undet The data item to be added to the collection
	  */
    template <typename x_type_undet, typename y_type_undet>
    void operator&(const std::tuple<x_type_undet, y_type_undet> &point_undet) {

        x_type x = x_type(0);
        y_type y = y_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = Gem::Common::narrow<x_type>(std::get<0>(point_undet));
            y = Gem::Common::narrow<y_type>(std::get<1>(point_undet));
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollector2T::operator&(const std::tuple<S,T>&): Error!" << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }

        data_.push_back(std::tuple<x_type, y_type>(x, y));
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
        data_.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component, narrowed to y_type
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template <typename x_type_undet, typename y_type_undet>
    void operator&(const std::vector<std::tuple<x_type_undet, y_type_undet>> &point_cnt_undet) {

        x_type x = x_type(0);
        y_type y = y_type(0);

        typename std::vector<std::tuple<x_type_undet, y_type_undet>>::const_iterator cit;
        for(cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<x_type>(std::get<0>(*cit));
                y = Gem::Common::narrow<y_type>(std::get<1>(*cit));
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollector2T::operator&(const std::vector<std::tuple<S,T>>&): Error!"
                    << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            data_.push_back(std::tuple<x_type, y_type>(x, y));
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
        for(cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            data_.push_back(*cit);
        }
    }

    /***************************************************************************/
    /**
	  * Sorts the data according to its x-component
	  */
    void sortX() {
        std::sort(
            data_.begin(),
            data_.end(),
            [](const std::tuple<x_type, y_type> &x, const std::tuple<x_type, y_type> &y) -> bool {
                return std::get<0>(x) < std::get<0>(y);
            }
        );
    }

    /***************************************************************************/
    /**
		* Retrieves the minimum and maximum values in data_ in x- and y-direction
		*
		* @return A tuple holding (min_x, max_x, min_y, max_y) of the stored data
		*/
    std::tuple<x_type, x_type, y_type, y_type> getMinMaxElements() const {
        auto minmax_x = std::minmax_element(
            data_.begin(),
            data_.end(),
            [](const std::tuple<x_type, y_type> &x, const std::tuple<x_type, y_type> &y) -> bool {
                return (std::get<0>(x) < std::get<0>(y));
            }
        );

        auto minmax_y = std::minmax_element(
            data_.begin(),
            data_.end(),
            [](const std::tuple<x_type, y_type> &x, const std::tuple<x_type, y_type> &y) -> bool {
                return (std::get<1>(x) < std::get<1>(y));
            }
        );

        double min_x = std::get<0>(*minmax_x.first);
        double max_x = std::get<0>(*minmax_x.second);
        double min_y = std::get<1>(*minmax_y.first);
        double max_y = std::get<1>(*minmax_y.second);

        return std::make_tuple(min_x, max_x, min_y, max_y);
    };

protected:
    /***************************************************************************/
    /**
	  * Single declaration of this class'es local data members, used by load_() and compare_()
	  *
	  * @return A tuple of named members (the data vector) of this object
	  */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(make_member("data_", self.data_));
    }

    /**
	  * Loads the data of another object
	  *
	  * @param cp A pointer to another GDataCollector2T<x_type, y_type> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own, derived from the single localMembers() declaration
        g_load_members(localMembers_(*this), localMembers_(*p_load));
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector2T<x_type, y_type>>(
        GDataCollector2T<x_type, y_type> const &,
        GDataCollector2T<x_type, y_type> const &,
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
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollector2T<x_type, y_type>", e);

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, y_type>> data_; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  *
	  * @return The name of this class as a string
	  */
    std::string name_() const override {
        return std::string("GDataCollector2T<x_type, y_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	  */
    GBasePlotter *clone_() const override = 0;
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
inline std::shared_ptr<GDataCollector1T<double>> GDataCollector2T<double, double>::projectX(
    std::size_t n_bins_x,
    std::tuple<double, double> range_x
) const {
    std::tuple<double, double> my_range_x;
    std::tuple<double, double> default_range;
    if(range_x == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double> extremes = getMinMax(this->data_);
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
    for(auto const &o : data_) {
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
 * @param n_bins_y The number of bins of the histogram
 * @param range_y The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the y-projection of the data
 */
template <>
inline std::shared_ptr<GDataCollector1T<double>> GDataCollector2T<double, double>::projectY(
    std::size_t n_bins_y,
    std::tuple<double, double> range_y
) const {
    std::tuple<double, double> my_range_y;
    std::tuple<double, double> default_range;
    if(range_y == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double> extremes = getMinMax(data_);
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
    for(auto const &o : data_) {
        (*result) & std::get<1>(o);
    }

    // Return the data
    return result;
}

/******************************************************************************/
/**
 * A data collector for 2-d data of user-defined type, with the ability to
 * additionally specify an error component for both dimensions. Note that the
 * plot dimension may be different. Each data item is a tuple (x, error_x, y, error_y).
 *
 * @tparam x_type The numeric type of the x-component and its error
 * @tparam y_type The numeric type of the y-component and its error
 */
template <typename x_type, typename y_type>
class GDataCollector2ET : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) & BOOST_SERIALIZATION_NVP(data_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector2ET() = default;
    GDataCollector2ET(GDataCollector2ET<x_type, y_type> const &) = default;
    GDataCollector2ET(GDataCollector2ET<x_type, y_type> &&) = default;

    ~GDataCollector2ET() override = default;

    GDataCollector2ET<x_type, y_type> &
    operator=(GDataCollector2ET<x_type, y_type> const &) = default;
    GDataCollector2ET<x_type, y_type> &operator=(GDataCollector2ET<x_type, y_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component and its error, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component and its error, narrowed to y_type
	  * @param point_undet The data item (x, error_x, y, error_y) to be added to the collection
	  */
    template <typename x_type_undet, typename y_type_undet>
    void operator&(
        const std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet> &point_undet
    ) {

        x_type x = x_type(0);
        x_type ex = x_type(0);
        y_type y = y_type(0);
        y_type ey = y_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = Gem::Common::narrow<x_type>(std::get<0>(point_undet));
            ex = Gem::Common::narrow<x_type>(std::get<1>(point_undet));
            y = Gem::Common::narrow<y_type>(std::get<2>(point_undet));
            ey = Gem::Common::narrow<y_type>(std::get<3>(point_undet));
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollector2ET::operator&(const std::tuple<S,S,T,T>&): Error!"
                << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }

        data_.push_back(std::tuple<x_type, x_type, y_type, y_type>(x, ex, y, ey));
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type std::tuple<x_type, x_type, y_type, y_type>
	  * (x, error_x, y, error_y) to the collection in an intuitive way.
	  *
	  * @param point The data item (x, error_x, y, error_y) to be added to the collection
	  */
    void operator&(const std::tuple<x_type, x_type, y_type, y_type> &point) {
        // Add the data item to the collection
        data_.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component and its error, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component and its error, narrowed to y_type
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template <typename x_type_undet, typename y_type_undet>
    void
    operator&(const std::vector<std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>>
                  &point_cnt_undet) {

        x_type x = x_type(0);
        x_type ex = x_type(0);
        y_type y = y_type(0);
        y_type ey = y_type(0);

        typename std::vector<
            std::tuple<x_type_undet, x_type_undet, y_type_undet, y_type_undet>>::const_iterator cit;
        for(cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<x_type>(std::get<0>(*cit));
                ex = Gem::Common::narrow<x_type>(std::get<1>(*cit));
                y = Gem::Common::narrow<y_type>(std::get<2>(*cit));
                ey = Gem::Common::narrow<y_type>(std::get<3>(*cit));
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollector2ET::operator&(const std::vector<std::tuple<S,S,T,T>>&): "
                       "Error!"
                    << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            data_.push_back(std::tuple<x_type, x_type, y_type, y_type>(x, ex, y, ey));
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
        for(cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            data_.push_back(*cit);
        }
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
	  * Sorts the data according to its x-component
	  */
    void sortX() {
        std::sort(
            data_.begin(),
            data_.end(),
            [](const std::tuple<x_type, x_type, y_type, y_type> &x,
               const std::tuple<x_type, x_type, y_type, y_type> &y) -> bool {
                return std::get<0>(x) < std::get<0>(y);
            }
        );
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
        return std::make_tuple(make_member("data_", self.data_));
    }

    /**
	  * Loads the data of another object
	  *
	  * @param cp A pointer to another GDataCollector2ET<x_type, y_type> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector2ET<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own, derived from the single localMembers() declaration
        g_load_members(localMembers_(*this), localMembers_(*p_load));
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
        // Check that we are dealing with a GDataCollector2ET<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollector2ET<x_type, y_type>", e);

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, x_type, y_type, y_type>> data_; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  *
	  * @return The name of this class as a string
	  */
    std::string name_() const override {
        return std::string("GDataCollector2ET<x_type, y_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	  */
    GBasePlotter *clone_() const override = 0;
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH2D class (2-d double data). This will result in a
 * 3D plot.
 */
class GHistogram2D : public GDataCollector2T<double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
 * A data collector for 3-d data of user-defined type
 *
 * @tparam x_type The numeric type of the x-component of each data item
 * @tparam y_type The numeric type of the y-component of each data item
 * @tparam z_type The numeric type of the z-component of each data item
 */
template <typename x_type, typename y_type, typename z_type>
class GDataCollector3T : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) & BOOST_SERIALIZATION_NVP(data_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector3T() = default;
    GDataCollector3T(GDataCollector3T<x_type, y_type, z_type> const &) = default;
    GDataCollector3T(GDataCollector3T<x_type, y_type, z_type> &&) = default;

    ~GDataCollector3T() override = default;

    GDataCollector3T<x_type, y_type, z_type> &
    operator=(GDataCollector3T<x_type, y_type, z_type> const &) = default;
    GDataCollector3T<x_type, y_type, z_type> &
    operator=(GDataCollector3T<x_type, y_type, z_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (x-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the x-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<x_type>>
    projectX(std::size_t nBins, std::tuple<x_type, x_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector3T<>::projectX(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<x_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (y-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the y-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<y_type>>
    projectY(std::size_t nBins, std::tuple<y_type, y_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector3T<>::projectY(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<y_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (z-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the z-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<z_type>>
    projectZ(std::size_t nBins, std::tuple<z_type, z_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector3T<>::projectZ(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
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
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component, narrowed to y_type
	  * @tparam z_type_undet The source type of the z-component, narrowed to z_type
	  * @param point_undet The data item to be added to the collection
	  */
    template <typename x_type_undet, typename y_type_undet, typename z_type_undet>
    void operator&(const std::tuple<x_type_undet, y_type_undet, z_type_undet> &point_undet) {

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = Gem::Common::narrow<x_type>(std::get<0>(point_undet));
            y = Gem::Common::narrow<y_type>(std::get<1>(point_undet));
            z = Gem::Common::narrow<z_type>(std::get<2>(point_undet));
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollector3T::operator&(const std::tuple<S,T,U>&): Error!" << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }

        data_.push_back(std::tuple<x_type, y_type, z_type>(x, y, z));
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
        data_.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component, narrowed to y_type
	  * @tparam z_type_undet The source type of the z-component, narrowed to z_type
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template <typename x_type_undet, typename y_type_undet, typename z_type_undet>
    void operator&(
        const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>> &point_cnt_undet
    ) {

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);

        typename std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>>::const_iterator
            cit;
        for(cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<x_type>(std::get<0>(*cit));
                y = Gem::Common::narrow<y_type>(std::get<1>(*cit));
                z = Gem::Common::narrow<z_type>(std::get<2>(*cit));
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollector3T::operator&(const std::vector<std::tuple<S,T,U>>&): "
                       "Error!"
                    << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            data_.push_back(std::tuple<x_type, y_type, z_type>(x, y, z));
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
        for(cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            data_.push_back(*cit);
        }
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
        return std::make_tuple(make_member("data_", self.data_));
    }

    /**
	  * Loads the data of another object
	  *
	  * @param cp A pointer to another GDataCollector3T<x_type, y_type, z_type> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector3T<x_type, y_type, z_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own, derived from the single localMembers() declaration
        g_load_members(localMembers_(*this), localMembers_(*p_load));
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector3T<x_type, y_type, z_type>>(
        GDataCollector3T<x_type, y_type, z_type> const &,
        GDataCollector3T<x_type, y_type, z_type> const &,
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
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollector3T<x_type, y_type, z_type>", e);

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, y_type, z_type>> data_; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  *
	  * @return The name of this class as a string
	  */
    std::string name_() const override {
        return std::string("GDataCollector3T<x_type, y_type, z_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	  */
    GBasePlotter *clone_() const override = 0;
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
inline std::shared_ptr<GDataCollector1T<double>> GDataCollector3T<double, double, double>::projectX(
    std::size_t n_bins_x,
    std::tuple<double, double> range_x
) const {
    std::tuple<double, double> my_range_x;
    std::tuple<double, double> default_range;
    if(range_x == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double> extremes =
            getMinMax(this->data_);
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
    for(auto const &o : data_) {
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
 * @param n_bins_y The number of bins of the histogram
 * @param range_y The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the y-projection of the data
 */
template <>
inline std::shared_ptr<GDataCollector1T<double>> GDataCollector3T<double, double, double>::projectY(
    std::size_t n_bins_y,
    std::tuple<double, double> range_y
) const {
    std::tuple<double, double> my_range_y;
    std::tuple<double, double> default_range;
    if(range_y == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(data_);
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
    for(auto const &o : data_) {
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
 * @param n_bins_z The number of bins of the histogram
 * @param range_z The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the z-projection of the data
 */
template <>
inline std::shared_ptr<GDataCollector1T<double>> GDataCollector3T<double, double, double>::projectZ(
    std::size_t n_bins_z,
    std::tuple<double, double> range_z
) const {
    std::tuple<double, double> my_range_z;
    std::tuple<double, double> default_range;
    if(range_z == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double> extremes = getMinMax(data_);
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
    for(auto const &o : data_) {
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
class GGraph3D : public GDataCollector3T<double, double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
 * A data collector for 4-d data of user-defined type
 *
 * @tparam x_type The numeric type of the x-component of each data item
 * @tparam y_type The numeric type of the y-component of each data item
 * @tparam z_type The numeric type of the z-component of each data item
 * @tparam w_type The numeric type of the w-component of each data item
 */
template <typename x_type, typename y_type, typename z_type, typename w_type>
class GDataCollector4T : public GBasePlotter {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBasePlotter) & BOOST_SERIALIZATION_NVP(data_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors and destructors

    GDataCollector4T() = default;
    GDataCollector4T(GDataCollector4T<x_type, y_type, z_type, w_type> const &) = default;
    GDataCollector4T(GDataCollector4T<x_type, y_type, z_type, w_type> &&) = default;
    ~GDataCollector4T() override = default;

    GDataCollector4T<x_type, y_type, z_type, w_type> &
    operator=(GDataCollector4T<x_type, y_type, z_type, w_type> const &) = default;
    GDataCollector4T<x_type, y_type, z_type, w_type> &
    operator=(GDataCollector4T<x_type, y_type, z_type, w_type> &&) = default;

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (x-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the x-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<x_type>>
    projectX(std::size_t nBins, std::tuple<x_type, x_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector4T<>::projectX(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<x_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (y-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the y-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<y_type>>
    projectY(std::size_t nBins, std::tuple<y_type, y_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector4T<>::projectY(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<y_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (z-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the z-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<z_type>>
    projectZ(std::size_t nBins, std::tuple<z_type, z_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector4T<>::projectZ(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
        );

        // Make the compiler happy
        return std::shared_ptr<GDataCollector1T<z_type>>();
    }

    /***************************************************************************/
    /**
	  * Allows to project the graph into a histogram (w-direction). This function is a
	  * trap to catch calls with un-implemented types. Use the corresponding specializations,
	  * if available.
	  *
	  * @param nBins The desired number of bins of the resulting histogram
	  * @param range The lower and upper boundary of the resulting histogram
	  * @return A 1-d histogram of the data projected onto the w-axis (only in specializations)
	  */
    std::shared_ptr<GDataCollector1T<w_type>>
    projectW(std::size_t nBins, std::tuple<w_type, w_type> range) const {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GDataCollector4T<>::projectZ(range, nBins): Error!" << '\n'
            << "Function was called for class with un-implemented types" << '\n'
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
	  * Allows to add data of undetermined type to the collection in an intuitive way,
	  * provided that it can be converted safely to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component, narrowed to y_type
	  * @tparam z_type_undet The source type of the z-component, narrowed to z_type
	  * @tparam w_type_undet The source type of the w-component, narrowed to w_type
	  * @param point_undet The data item to be added to the collection
	  */
    template <
        typename x_type_undet,
        typename y_type_undet,
        typename z_type_undet,
        typename w_type_undet>
    void operator&(
        const std::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet> &point_undet
    ) {

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);
        w_type w = w_type(0);

        // Make sure the data can be converted to doubles
        try {
            x = Gem::Common::narrow<x_type>(std::get<0>(point_undet));
            y = Gem::Common::narrow<y_type>(std::get<1>(point_undet));
            z = Gem::Common::narrow<z_type>(std::get<2>(point_undet));
            w = Gem::Common::narrow<w_type>(std::get<3>(point_undet));
        }
        catch(std::overflow_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GDataCollector4T::operator&(const std::tuple<S,T,U,W>&): Error!" << '\n'
                << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                << "with the message " << '\n'
                << e.what() << '\n'
            );
        }

        data_.push_back(std::tuple<x_type, y_type, z_type, w_type>(x, y, z, w));
    }

    /***************************************************************************/
    /**
	  * Allows to add data of type std::tuple<x_type, y_type, z_type, w_type> to the
	  * collection in an intuitive way.
	  *
	  * @param point The data item to be added to the collection
	  */
    void operator&(const std::tuple<x_type, y_type, z_type, w_type> &point) {
        // Add the data item to the collection
        data_.push_back(point);
    }

    /***************************************************************************/
    /**
	  * Allows to add a collection of data items of undetermined type to the
	  * collection in an intuitive way, provided they can be converted safely
	  * to the target type.
	  *
	  * @tparam x_type_undet The source type of the x-component, narrowed to x_type
	  * @tparam y_type_undet The source type of the y-component, narrowed to y_type
	  * @tparam z_type_undet The source type of the z-component, narrowed to z_type
	  * @tparam w_type_undet The source type of the w-component, narrowed to w_type
	  * @param point_cnt_undet The collection of data items to be added to the collection
	  */
    template <
        typename x_type_undet,
        typename y_type_undet,
        typename z_type_undet,
        typename w_type_undet>
    void
    operator&(const std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet, w_type_undet>>
                  &point_cnt_undet) {

        x_type x = x_type(0);
        y_type y = y_type(0);
        z_type z = z_type(0);
        w_type w = w_type(0);

        typename std::vector<std::tuple<x_type_undet, y_type_undet, z_type_undet>>::const_iterator
            cit;
        for(cit = point_cnt_undet.begin(); cit != point_cnt_undet.end(); ++cit) {
            // Make sure the data can be converted to doubles
            try {
                x = Gem::Common::narrow<x_type>(std::get<0>(*cit));
                y = Gem::Common::narrow<y_type>(std::get<1>(*cit));
                z = Gem::Common::narrow<z_type>(std::get<2>(*cit));
                w = Gem::Common::narrow<w_type>(std::get<3>(*cit));
            }
            catch(std::overflow_error &e) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GDataCollector4T::operator&(const std::vector<std::tuple<S,T,U,W>>&): "
                       "Error!"
                    << '\n'
                    << "Encountered invalid cast with Gem::Common::narrow," << '\n'
                    << "with the message " << '\n'
                    << e.what() << '\n'
                );
            }

            data_.push_back(std::tuple<x_type, y_type, z_type, w_type>(x, y, z, w));
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
        for(cit = point_cnt.begin(); cit != point_cnt.end(); ++cit) {
            // Add the data item to the collection
            data_.push_back(*cit);
        }
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
        return std::make_tuple(make_member("data_", self.data_));
    }

    /**
	  * Loads the data of another object
	  *
	  * @param cp A pointer to another GDataCollector4T<x_type, y_type, z_type, w_type> object, camouflaged as a GBasePlotter
	  */
    void load_(const GBasePlotter *cp) override {
        // Check that we are dealing with a GDataCollector4T<x_type, y_type, z_type, w_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        // Load our parent class'es data ...
        GBasePlotter::load_(cp);

        // ... and then our own, derived from the single localMembers() declaration
        g_load_members(localMembers_(*this), localMembers_(*p_load));
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GDataCollector4T<x_type, y_type, z_type, w_type>>(
        GDataCollector4T<x_type, y_type, z_type, w_type> const &,
        GDataCollector4T<x_type, y_type, z_type, w_type> const &,
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
        // Check that we are dealing with a GDataCollector2T<x_type, y_type> reference independent of this object and convert the pointer
        const auto *p_load = g_convert_and_compare(cp, this);

        GToken token("GDataCollector4T<x_type, y_type, z_type, w_type>", e);

        // Compare our parent data ...
        compare_base_t<GBasePlotter>(*this, *p_load, token);

        // ... and then the local data, derived from the single localMembers() declaration
        g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/

    std::vector<std::tuple<x_type, y_type, z_type, w_type>> data_; ///< Holds the actual data

private:
    /***************************************************************************/
    /**
	  * Returns the name of this class
	  *
	  * @return The name of this class as a string
	  */
    std::string name_() const override {
        return std::string("GDataCollector4T<x_type, y_type, z_type, w_type>");
    }

    /***************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  * @return A deep clone of this object, wrapped into a GBasePlotter pointer
	  */
    GBasePlotter *clone_() const override = 0;
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
inline std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectX(
    std::size_t n_bins_x,
    std::tuple<double, double> range_x
) const {
    std::tuple<double, double> my_range_x;
    std::tuple<double, double> default_range;
    if(range_x == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->data_);
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
    for(auto const &o : data_) {
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
 * @param n_bins_y The number of bins of the histogram
 * @param range_y The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the y-projection of the data
 */
template <>
inline std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectY(
    std::size_t n_bins_y,
    std::tuple<double, double> range_y
) const {
    std::tuple<double, double> my_range_y;
    std::tuple<double, double> default_range;
    if(range_y == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->data_);
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
    for(auto const &o : data_) {
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
 * @param n_bins_z The number of bins of the histogram
 * @param range_z The minimum and maximum boundaries of the histogram
 * @return A shared pointer to a GHistogram1D holding the z-projection of the data
 */
template <>
inline std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectZ(
    std::size_t n_bins_z,
    std::tuple<double, double> range_z
) const {
    std::tuple<double, double> my_range_z;
    std::tuple<double, double> default_range;
    if(range_z == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->data_);
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
    for(auto const &o : data_) {
        (*result) & std::get<2>(o);
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
inline std::shared_ptr<GDataCollector1T<double>>
GDataCollector4T<double, double, double, double>::projectW(
    std::size_t n_bins_w,
    std::tuple<double, double> range_w
) const {
    std::tuple<double, double> my_range_w;
    std::tuple<double, double> default_range;
    if(range_w == default_range) {
        // Find out about the minimum and maximum values in the data_ array
        std::tuple<double, double, double, double, double, double, double, double> extremes =
            getMinMax(this->data_);
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
    for(auto const &o : data_) {
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
class GGraph4D : public GDataCollector4T<double, double, double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool isSecondary, std::size_t pId, const std::string &indention) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool isSecondary, std::size_t pId, const std::string &indention) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool isSecondary, std::size_t pId, const std::string &indention) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
    void serialize(Archive &ar, const unsigned int) {
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
    std::string headerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

    /**
	 * @brief Retrieves the actual data sets
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The body section of the ROOT code, holding the actual data
	 */
    std::string bodyData_(bool isSecondary, std::size_t pId, const std::string &indention) const override;

    /**
	 * @brief Retrieves specific draw commands for this plot
	 * @param isSecondary Whether this is a secondary plot drawn into an existing pad
	 * @param pId The id of this plotter, used to build unique variable names
	 * @param indention The indention string prepended to each emitted line
	 * @return The footer section of the ROOT code, holding the draw commands
	 */
    std::string footerData_(bool is_secondary, std::size_t p_id, const std::string &indent) const override;

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
