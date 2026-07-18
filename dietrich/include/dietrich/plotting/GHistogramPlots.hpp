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

#include "dietrich/plotting/GDataCollectors.hpp"


namespace Gem::Dietrich {


/******************************************************************************/
/**
 * A wrapper for ROOT's TH1D class (1-d double data). This will result in a 2D-plot.
 */
class GHistogram1D : public GDataCollector1T<double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    // Defined ahead of serialize(): a deduced-return-type member must be
    // defined before its first use in this class (serialize_members below).
    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(
            make_member("n_bins_x_", self.n_bins_x_),
            make_member("min_x_", self.min_x_),
            make_member("max_x_", self.max_x_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector1T_double",
            boost::serialization::base_object<GDataCollector1T<double>>(*this)
        );
        // ... and then our own data, derived from the single localMembers_() declaration
        Gem::Common::serialize_members(ar, this->localMembers_());
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

    /**
	 * @brief Reports this plotter's choice (kind, columns, bins) as a GPlotSpec value
	 * @return A GPlotSpec describing this plotter
	 */
    [[nodiscard]] GPlotSpec plotSpec() const override;

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
	 * @brief Loads the data of another object
	 * @param cp A pointer to another GHistogram1D object, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /***************************************************************************/

    friend void Gem::Common::compare_base_t<GHistogram1D>(GHistogram1D const &, GHistogram1D const &, GToken &);

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

    /** @brief Applies test-only modifications: the base members plus this histogram's bin count,
     *  value range and a sample datum. Never invoked on rendered objects. */
    bool modify_GUnitTests_() override {
        GBasePlotter::modify_GUnitTests_();
        n_bins_x_ += 5;
        min_x_ = -2.;
        max_x_ = 2.;
        this->add(0.5);
        return true;
    }

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

    // Defined ahead of serialize(): a deduced-return-type member must be
    // defined before its first use in this class (serialize_members below).
    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    auto localMembers_(this Self &self) {
        return std::make_tuple(
            make_member("n_bins_x_", self.n_bins_x_),
            make_member("min_x_", self.min_x_),
            make_member("max_x_", self.max_x_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector1T_int32_t",
            boost::serialization::base_object<GDataCollector1T<std::int32_t>>(*this)
        );
        // ... and then our own data, derived from the single localMembers_() declaration
        Gem::Common::serialize_members(ar, this->localMembers_());
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

    /**
	 * @brief Reports this plotter's choice (kind, columns, bins) as a GPlotSpec value
	 * @return A GPlotSpec describing this plotter
	 */
    [[nodiscard]] GPlotSpec plotSpec() const override;

protected:

    /**
	 * @brief Loads the data of another object
	 * @param cp A pointer to another GHistogram1I object, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GHistogram1I>(GHistogram1I const &, GHistogram1I const &, GToken &);

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

    /** @brief Applies test-only modifications: the base members plus this histogram's bin count,
     *  value range and an integer sample datum. Never invoked on rendered objects. */
    bool modify_GUnitTests_() override {
        GBasePlotter::modify_GUnitTests_();
        n_bins_x_ += 5;
        min_x_ = -2.;
        max_x_ = 2.;
        this->add(std::int32_t(1));
        return true;
    }

    GHistogram1I() =
        default; ///< The default constructor -- intentionally private as it is only needed for (de-)serialization

    std::size_t n_bins_x_ = 0; ///< The number of bins in the histogram

    double min_x_ = 0.; ///< The lower boundary of the histogram
    double max_x_ = 0.; ///< The upper boundary of the histogram
};

/******************************************************************************/
/**
 * A wrapper for ROOT's TH2D class (2-d double data). This will result in a
 * 3D plot.
 */
class GHistogram2D : public GDataCollector2T<double, double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    // Defined ahead of serialize(): a deduced-return-type member must be
    // defined before its first use in this class (serialize_members below).
    /**
	 * @brief Single declaration of this class'es local data members, used by load_() and compare_()
	 * @return A tuple of named local members of this object
	 */
    template <typename Self>
    auto localMembers_(this Self &self) {
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

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GDataCollector2T_double_double",
            boost::serialization::base_object<GDataCollector2T<double, double>>(*this)
        );
        // ... and then our own data, derived from the single localMembers_() declaration
        Gem::Common::serialize_members(ar, this->localMembers_());
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
	 * @brief Reports this plotter's choice (kind, columns, bins) as a GPlotSpec value
	 * @return A GPlotSpec describing this plotter
	 */
    [[nodiscard]] GPlotSpec plotSpec() const override;

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
	 * @brief Loads the data of another object
	 * @param cp A pointer to another object of the same type, camouflaged as a GBasePlotter
	 */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GHistogram2D>(GHistogram2D const &, GHistogram2D const &, GToken &);

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

    /** @brief Applies test-only modifications: the base members plus this histogram's per-axis bin
     *  counts, value ranges and a sample datum. Never invoked on rendered objects. */
    bool modify_GUnitTests_() override {
        GBasePlotter::modify_GUnitTests_();
        n_bins_x_ += 5;
        n_bins_y_ += 3;
        min_x_ = -2.;
        max_x_ = 2.;
        min_y_ = -1.;
        max_y_ = 1.;
        this->add(0.5, 0.25);
        return true;
    }

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


} /* namespace Gem::Dietrich */
