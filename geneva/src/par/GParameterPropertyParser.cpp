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

#include "geneva/par/GParameterPropertyParser.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Gem::Geneva::Genome {

constexpr std::size_t GPP_DEF_NSTEPS = 100; // The default number of steps for a given parameter

/******************************************************************************/
// Small hand-written parsing helpers. They replace the former Boost.Spirit grammar:
// the parameter-property syntax is now strictly positional, so a tiny tokenizer is
// both sufficient and far simpler.
//
// Grammar (whitespace is ignored):
//   spec-list := spec ( ',' spec )*
//   spec      := type '(' args ')'
//   type      := 'd' | 'f' | 'i' | 'b' | 's'
//   args (d/f/i) := index ',' lower ',' upper [ ',' nSteps ] [ ',' label ]
//   args (b)     := index [ ',' lower ',' upper [ ',' nSteps ] ] [ ',' label ]
//   args (s)     := nItems
// Parameters are addressed by 'index' (their position in the flat parameter vector of
// the corresponding type). The optional 'label' is a free-form display name used by
// monitors (e.g. GProgressPlotter axis labels); it does NOT identify a parameter.
namespace {

std::string trim(const std::string &s) {
    std::size_t b = s.find_first_not_of(" \t\n\r");
    if(b == std::string::npos) {
        return std::string{};
    }
    std::size_t e = s.find_last_not_of(" \t\n\r");
    return s.substr(b, e - b + 1);
}

std::vector<std::string> splitOnComma(const std::string &s) {
    std::vector<std::string> out;
    std::string cur;
    for(char c : s) {
        if(c == ',') {
            out.push_back(trim(cur));
            cur.clear();
        }
        else {
            cur.push_back(c);
        }
    }
    out.push_back(trim(cur));
    return out;
}

bool isUnsigned(const std::string &s) {
    if(s.empty()) {
        return false;
    }
    for(char c : s) {
        if(c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

[[noreturn]] void fail(const std::string &raw) {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In GParameterPropertyParser::parse(): Error!" << '\n'
        << "Could not parse parameter-property fragment: " << raw << '\n'
    );
}

std::size_t toUnsigned(const std::string &s, const std::string &raw) {
    if(not isUnsigned(s)) {
        fail(raw);
    }
    return static_cast<std::size_t>(std::stoul(s));
}

bool toBool(const std::string &s, const std::string &raw) {
    if(s == "true" || s == "1") {
        return true;
    }
    if(s == "false" || s == "0") {
        return false;
    }
    fail(raw);
}

// Fills the var/bounds/nSteps/label of a numeric (d/f/i) spec from its argument tokens.
template <typename par_type, typename ConvFun>
parPropSpec<par_type> makeNumericSpec(
    const std::vector<std::string> &tok,
    ConvFun convertBound,
    const std::string &raw
) {
    if(tok.size() < 3) {
        fail(raw);
    }
    parPropSpec<par_type> spec;
    const std::size_t index = toUnsigned(tok[0], raw);
    spec.lowerBoundary = convertBound(tok[1], raw);
    spec.upperBoundary = convertBound(tok[2], raw);
    spec.nSteps = GPP_DEF_NSTEPS;
    std::string label;
    for(std::size_t k = 3; k < tok.size(); ++k) {
        if(isUnsigned(tok[k])) {
            spec.nSteps = static_cast<std::size_t>(std::stoul(tok[k]));
        }
        else {
            label = tok[k];
        }
    }
    spec.var = NAMEANDIDTYPE(label.empty() ? 0 : 2, label, index);
    return spec;
}

} /* anonymous namespace */

/******************************************************************************/
/**
 * The standard constructor -- assignment of the "raw" parameter property string
 */
GParameterPropertyParser::GParameterPropertyParser(const std::string &rw)
  : raw_(rw)
  , parsed_(false) {
    this->parse();
}

/******************************************************************************/
/**
 * Retrieves the raw parameter description
 */
std::string GParameterPropertyParser::getRawParameterDescription() const {
    return raw_;
}

/******************************************************************************/
/**
 * Allows to check whether parsing has already taken place
 */
bool GParameterPropertyParser::isParsed() const {
    return parsed_;
}

/******************************************************************************/
/**
 * Allows to reset the internal structures and to parse a new parameter string
 */
void GParameterPropertyParser::setNewParameterDescription(std::string raw) {
    raw_ = raw;

    s_spec_vec_.clear();
    d_spec_vec_.clear();
    f_spec_vec_.clear();
    i_spec_vec_.clear();
    b_spec_vec_.clear();

    parsed_ = false;

    // Update the information
    this->parse();
}

/******************************************************************************/
/**
 * Initiates parsing of the raw_ string. The grammar is documented at the top of this file.
 */
void GParameterPropertyParser::parse() {
    // Do nothing if the string has already been parsed
    if(parsed_) {
        return;
    }

    // Tokenize the raw string into (type, content) fragments of the form type'('content')'.
    std::vector<std::pair<char, std::string>> fragments;
    {
        const std::string &s = raw_;
        std::size_t i = 0;
        const std::size_t n = s.size();
        auto skipSep = [&]() {
            while(i < n && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r' || s[i] == ',')) {
                ++i;
            }
        };
        skipSep();
        while(i < n) {
            const char type = s[i];
            if(type != 'd' && type != 'f' && type != 'i' && type != 'b' && type != 's') {
                fail(s.substr(i));
            }
            ++i;
            if(i >= n || s[i] != '(') {
                fail(s.substr(i));
            }
            ++i; // consume '('
            std::string content;
            while(i < n && s[i] != ')') {
                content.push_back(s[i]);
                ++i;
            }
            if(i >= n) { // no closing ')'
                fail(s);
            }
            ++i; // consume ')'
            fragments.emplace_back(type, content);
            skipSep();
        }
    }

    // Process each fragment.
    for(const auto &fragment : fragments) {
        const char type = fragment.first;
        const std::vector<std::string> tok = splitOnComma(fragment.second);

        if(type == 'd') {
            d_spec_vec_.push_back(makeNumericSpec<double>(
                tok,
                [](const std::string &t, const std::string &raw) { return t.empty() ? (fail(raw), 0.0) : std::stod(t); },
                fragment.second
            ));
        }
        else if(type == 'f') {
            f_spec_vec_.push_back(makeNumericSpec<float>(
                tok,
                [](const std::string &t, const std::string &raw) { return t.empty() ? (fail(raw), 0.0f) : std::stof(t); },
                fragment.second
            ));
        }
        else if(type == 'i') {
            i_spec_vec_.push_back(makeNumericSpec<std::int32_t>(
                tok,
                [](const std::string &t, const std::string &raw) { return t.empty() ? (fail(raw), std::int32_t(0)) : static_cast<std::int32_t>(std::stoi(t)); },
                fragment.second
            ));
        }
        else if(type == 'b') {
            if(tok.empty() || tok[0].empty()) {
                fail(fragment.second);
            }
            parPropSpec<bool> spec;
            const std::size_t index = toUnsigned(tok[0], fragment.second);
            spec.lowerBoundary = false;
            spec.upperBoundary = true;
            spec.nSteps = GPP_DEF_NSTEPS;
            std::string label;
            if(tok.size() >= 3) {
                spec.lowerBoundary = toBool(tok[1], fragment.second);
                spec.upperBoundary = toBool(tok[2], fragment.second);
                for(std::size_t k = 3; k < tok.size(); ++k) {
                    if(isUnsigned(tok[k])) {
                        spec.nSteps = static_cast<std::size_t>(std::stoul(tok[k]));
                    }
                    else {
                        label = tok[k];
                    }
                }
            }
            else if(tok.size() == 2) {
                if(isUnsigned(tok[1])) {
                    spec.nSteps = static_cast<std::size_t>(std::stoul(tok[1]));
                }
                else {
                    label = tok[1];
                }
            }
            spec.var = NAMEANDIDTYPE(label.empty() ? 0 : 2, label, index);
            b_spec_vec_.push_back(spec);
        }
        else if(type == 's') {
            if(tok.empty()) {
                fail(fragment.second);
            }
            simpleScanSpec spec{};
            spec.nItems = toUnsigned(tok[0], fragment.second);
            s_spec_vec_.push_back(spec);
        }
        else {
            fail(fragment.second);
        }
    }

    // We only accept a single "simple-scan" entry. Complain, if more than one was found.
    if(s_spec_vec_.size() > 1) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterPropertyParser::parse(): Error!" << '\n'
            << "Found " << s_spec_vec_.size() << " simple scan entries where a" << '\n'
            << "maximum of 1 is allowed" << '\n'
        );
    }
    if(s_spec_vec_.size() == 1) { // If we did find a "simple scan" entry, we will discard the other entries.
        if(not d_spec_vec_.empty() || not f_spec_vec_.empty() || not i_spec_vec_.empty() ||
                not b_spec_vec_.empty()) {
            glogger << "In GParameterPropertyParser::parse(): Warning!" << '\n'
                    << "You have specified both a simple-scan component and explicit" << '\n'
                    << "scan-components. The explicit components will be discarded." << '\n'
                    << GWARNING;

            d_spec_vec_.clear();
            f_spec_vec_.clear();
            i_spec_vec_.clear();
            b_spec_vec_.clear();
        }
    }

    // Prevent further use of this function
    parsed_ = true;
}

/******************************************************************************/
/**
 * Retrieve the number of "simple scan" items
 */
std::size_t GParameterPropertyParser::getNSimpleScanItems() const {
    if(s_spec_vec_.empty()) {
        return static_cast<std::size_t>(0);
    }
    // Return the data of the first item
#ifdef DEBUG
    if(s_spec_vec_.size() > 1) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterPropertyParser::getNSimpleScanItems() const: Error!" << '\n'
            << "Found " << s_spec_vec_.size() << " simple scan entries where a" << '\n'
            << "maximum of 1 is allowed" << '\n'
        );
    }
#endif

    return (s_spec_vec_.front()).nItems;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
