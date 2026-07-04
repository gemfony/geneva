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

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "common/GExceptions.hpp"
#include "common/GParserBuilder.hpp"

using namespace Gem::Common;

namespace {

std::filesystem::path scratch(std::string const &tag) {
    auto base = std::filesystem::temp_directory_path() / "geneva_parserbuilder_tests";
    std::filesystem::create_directories(base);
    return base / (tag + ".json");
}

} // namespace

// ---------------------------------------------------------------------------
// Empty state

TEST_CASE("GParserBuilder: fresh instance reports zero options",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    CHECK(gpb.numberOfFileOptions() == 0);
    CHECK(gpb.numberOfCLOptions()   == 0);
}

// ---------------------------------------------------------------------------
// registerFileParameter (reference form): default value applied when missing.

TEST_CASE("GParserBuilder: writeConfigFile + parseConfigFile round-trip "
          "(missing file → default values written)",
          "[common][parser-builder]") {
    auto cfg = scratch("rt_default");
    std::filesystem::remove(cfg);

    {
        GParserBuilder gpb;
        int    answer = 0;
        double scale  = 0.0;
        gpb.registerFileParameter<int>   ("answer", answer, 42,   VAR_IS_ESSENTIAL, "the int");
        gpb.registerFileParameter<double>("scale",  scale,  3.14, VAR_IS_ESSENTIAL, "the double");
        REQUIRE(gpb.numberOfFileOptions() == 2);

        // First parse: file is absent, so GParserBuilder writes one with
        // defaults; parsing then returns the defaults.
        REQUIRE(gpb.parseConfigFile(cfg));
        CHECK(answer == 42);
        CHECK(scale  == 3.14);
        REQUIRE(std::filesystem::exists(cfg));
    }

    // Second parse re-reads the generated file. Values should still be the
    // defaults because nothing has changed.
    {
        GParserBuilder gpb;
        int    answer = -1;
        double scale  = -1.0;
        gpb.registerFileParameter<int>   ("answer", answer, 0,   VAR_IS_ESSENTIAL);
        gpb.registerFileParameter<double>("scale",  scale,  0.0, VAR_IS_ESSENTIAL);
        REQUIRE(gpb.parseConfigFile(cfg));
        CHECK(answer == 42);
        CHECK(scale  == 3.14);
    }

    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// Call-back form: callback receives the parsed value.

TEST_CASE("GParserBuilder::registerFileParameter (callback form) fires after parse",
          "[common][parser-builder]") {
    auto cfg = scratch("callback");
    std::filesystem::remove(cfg);

    int sink = 0;
    GParserBuilder gpb;
    gpb.registerFileParameter<int>(
        "value",
        99,
        [&sink](int v) { sink = v; },
        VAR_IS_ESSENTIAL,
        "via callback"
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(sink == 99);

    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// file_at<>: locate a registered parameter by name.

TEST_CASE("GParserBuilder::file_at<T> returns the matching shared_ptr",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int sink = 0;
    gpb.registerFileParameter<int>("hit", sink, 5);

    auto p = gpb.file_at<GFileParsableI>("hit");
    REQUIRE(p);
    CHECK(p->GParsableI::optionName(0) == "hit");

    auto miss = gpb.file_at<GFileParsableI>("nope");
    CHECK_FALSE(miss);
}

// ---------------------------------------------------------------------------
// numberOfFileOptions grows with each new registration.

TEST_CASE("GParserBuilder: numberOfFileOptions counts registrations",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int a = 0;
    int b = 0;
    int c = 0;
    gpb.registerFileParameter<int>("a", a, 1);
    gpb.registerFileParameter<int>("b", b, 2);
    gpb.registerFileParameter<int>("c", c, 3);
    CHECK(gpb.numberOfFileOptions() == 3);
}

// ---------------------------------------------------------------------------
// Comment-arg branch of registerFileParameter (single / reference).

TEST_CASE("GParserBuilder::registerFileParameter (reference + non-empty comment)",
          "[common][parser-builder]") {
    auto cfg = scratch("ref_with_comment");
    std::filesystem::remove(cfg);

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 11, VAR_IS_ESSENTIAL, "a numeric value");
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(v == 11);
    std::filesystem::remove(cfg);
}

TEST_CASE("GParserBuilder::registerFileParameter (callback + non-empty comment)",
          "[common][parser-builder]") {
    auto cfg = scratch("cb_with_comment");
    std::filesystem::remove(cfg);

    int sink = 0;
    GParserBuilder gpb;
    gpb.registerFileParameter<int>(
        "v",
        13,
        [&sink](int v) { sink = v; },
        VAR_IS_SECONDARY,
        "comment-text"
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(sink == 13);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// Two-parameter (combined) form.

TEST_CASE("GParserBuilder::registerFileParameter (two-parameter combined form)",
          "[common][parser-builder]") {
    auto cfg = scratch("combined");
    std::filesystem::remove(cfg);

    int    captured_int = 0;
    double captured_dbl = 0.0;
    GParserBuilder gpb;
    gpb.registerFileParameter<int, double>(
        "alpha", "beta",
        17, 2.5,
        [&](int i, double d) { captured_int = i; captured_dbl = d; },
        "combined_label",
        VAR_IS_ESSENTIAL
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(captured_int == 17);
    CHECK(captured_dbl == 2.5);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// Vector forms: callback and reference.

TEST_CASE("GParserBuilder::registerFileParameter (vector callback form)",
          "[common][parser-builder]") {
    auto cfg = scratch("vec_cb");
    std::filesystem::remove(cfg);

    std::vector<int> received;
    std::vector<int> def_val{1, 2, 3};
    GParserBuilder gpb;
    gpb.registerFileParameter<int>(
        "values",
        def_val,
        [&received](std::vector<int> v) { received = std::move(v); },
        VAR_IS_ESSENTIAL,
        "a vector"
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(received == def_val);
    std::filesystem::remove(cfg);
}

TEST_CASE("GParserBuilder::registerFileParameter (vector reference form)",
          "[common][parser-builder]") {
    auto cfg = scratch("vec_ref");
    std::filesystem::remove(cfg);

    std::vector<double> sink;
    std::vector<double> def_val{1.5, 2.5};
    GParserBuilder gpb;
    gpb.registerFileParameter<double>(
        "vals", sink, def_val, VAR_IS_ESSENTIAL, "doubles"
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(sink == def_val);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// Array forms (callback + reference, with std::array<T,N>).

TEST_CASE("GParserBuilder::registerFileParameter (std::array callback form)",
          "[common][parser-builder]") {
    auto cfg = scratch("arr_cb");
    std::filesystem::remove(cfg);

    constexpr std::size_t N = 4;
    std::array<int, N> received{};
    std::array<int, N> def_val{10, 20, 30, 40};

    GParserBuilder gpb;
    gpb.registerFileParameter<int, N>(
        "arr",
        def_val,
        [&received](std::array<int, N> v) { received = v; },
        VAR_IS_ESSENTIAL,
        "array via callback"
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(received == def_val);
    std::filesystem::remove(cfg);
}

TEST_CASE("GParserBuilder::registerFileParameter (std::array reference form)",
          "[common][parser-builder]") {
    auto cfg = scratch("arr_ref");
    std::filesystem::remove(cfg);

    constexpr std::size_t N = 3;
    std::array<double, N> sink{};
    std::array<double, N> def_val{1.0, 2.0, 3.0};

    GParserBuilder gpb;
    gpb.registerFileParameter<double, N>(
        "arr", sink, def_val, VAR_IS_ESSENTIAL, "array ref"
    );
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(sink == def_val);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// Command-line parameter registration and parsing.

TEST_CASE("GParserBuilder::registerCLParameter + parseCommandLine",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int    intval = 0;
    double dblval = 0.0;
    gpb.registerCLParameter<int>   ("intopt", intval, 100, "an integer");
    gpb.registerCLParameter<double>("dblopt", dblval, 0.5, "a double");
    CHECK(gpb.numberOfCLOptions() == 2);

    // Build an argv that supplies both options.
    char prog[]   = "prog";
    char opt1[]   = "--intopt=42";
    char opt2[]   = "--dblopt=7.25";
    char *argv[]  = {prog, opt1, opt2, nullptr};
    // parseCommandLine returns GCL_HELP_REQUESTED (true) only when --help is
    // present; success returns GCL_NO_HELP_REQUESTED (false).
    CHECK_FALSE(gpb.parseCommandLine(3, argv, /*verbose*/ false));

    CHECK(intval == 42);
    CHECK(dblval == 7.25);
}

TEST_CASE("GParserBuilder::registerCLParameter applies defaults when option absent",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int v = -1;
    gpb.registerCLParameter<int>("k", v, 999);
    char prog[] = "prog";
    char *argv[] = {prog, nullptr};
    CHECK_FALSE(gpb.parseCommandLine(1, argv, false));
    CHECK(v == 999);
}

TEST_CASE("GParserBuilder::parseCommandLine: --help returns GCL_HELP_REQUESTED",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int v = 0;
    gpb.registerCLParameter<int>("k", v, 1);

    char prog[] = "prog";
    char help[] = "--help";
    char *argv[] = {prog, help, nullptr};
    CHECK(gpb.parseCommandLine(2, argv, false));
}

TEST_CASE("GParserBuilder::cl_at finds and casts a registered CL parameter",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int v = 0;
    gpb.registerCLParameter<int>("hit", v, 5);

    auto p = gpb.cl_at<GCLParsableI>("hit");
    REQUIRE(p);
    CHECK(p->GParsableI::optionName(0) == "hit");

    CHECK_FALSE(gpb.cl_at<GCLParsableI>("missing"));
}

// ---------------------------------------------------------------------------
// resetFileParameterDefaults (single + vector + array shapes).

TEST_CASE("GParserBuilder::resetFileParameterDefaults (single value)",
          "[common][parser-builder]") {
    auto cfg = scratch("reset_single");
    std::filesystem::remove(cfg);

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("k", v, 1);
    gpb.resetFileParameterDefaults<int>("k", 77);

    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(v == 77);
    std::filesystem::remove(cfg);
}

TEST_CASE("GParserBuilder::resetFileParameterDefaults (vector value)",
          "[common][parser-builder]") {
    auto cfg = scratch("reset_vec");
    std::filesystem::remove(cfg);

    GParserBuilder gpb;
    std::vector<int> sink;
    gpb.registerFileParameter<int>("vs", sink, std::vector<int>{1, 2});
    gpb.resetFileParameterDefaults<int>("vs", std::vector<int>{10, 20, 30});

    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(sink == std::vector<int>{10, 20, 30});
    std::filesystem::remove(cfg);
}

TEST_CASE("GParserBuilder::resetFileParameterDefaults (array value)",
          "[common][parser-builder]") {
    auto cfg = scratch("reset_arr");
    std::filesystem::remove(cfg);

    constexpr std::size_t N = 2;
    GParserBuilder gpb;
    std::array<int, N> sink{};
    gpb.registerFileParameter<int, N>("ar", sink, std::array<int, N>{1, 1});
    gpb.resetFileParameterDefaults<int, N>("ar", std::array<int, N>{42, 43});

    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(sink == std::array<int, N>{42, 43});
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// resetFileParameterDefaults: missing key throws.

TEST_CASE("GParserBuilder::resetFileParameterDefaults: missing option throws",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    CHECK_THROWS_AS(gpb.resetFileParameterDefaults<int>("nope", 1),
                    geneva_exception);
    CHECK_THROWS_AS(gpb.resetFileParameterDefaults<int>("nope", std::vector<int>{1}),
                    geneva_exception);
}

// ---------------------------------------------------------------------------
// writeConfigFile

TEST_CASE("GParserBuilder::writeConfigFile produces a parseable JSON",
          "[common][parser-builder]") {
    auto cfg = scratch("write_only");
    std::filesystem::remove(cfg);

    {
        GParserBuilder gpb;
        int    a = 0;
        double b = 0.0;
        gpb.registerFileParameter<int>   ("a", a, 1, VAR_IS_ESSENTIAL, "alpha");
        gpb.registerFileParameter<double>("b", b, 2.5, VAR_IS_ESSENTIAL, "beta");
        gpb.writeConfigFile(cfg, "test-header", /*pretty*/ true);
    }
    REQUIRE(std::filesystem::exists(cfg));

    // Reading the file back into a fresh builder should yield the same values.
    GParserBuilder gpb2;
    int    a = 0;
    double b = 0.0;
    gpb2.registerFileParameter<int>   ("a", a, 99);
    gpb2.registerFileParameter<double>("b", b, 0.0);
    REQUIRE(gpb2.parseConfigFile(cfg));
    CHECK(a == 1);
    CHECK(b == 2.5);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// configureFromFile<T> helper.

namespace {

struct ConfigurableObject {
    int    answer = 0;
    double scale  = 0.0;

    void addConfigurationOptions(GParserBuilder &gpb) {
        gpb.registerFileParameter<int>   ("answer", answer, 42);
        gpb.registerFileParameter<double>("scale",  scale,  3.14);
    }
};

} // namespace

TEST_CASE("GParserBuilder::configureFromFile populates a target object",
          "[common][parser-builder]") {
    auto cfg = scratch("configure_from_file");
    std::filesystem::remove(cfg);

    ConfigurableObject obj;
    configureFromFile(obj, cfg);
    CHECK(obj.answer == 42);
    CHECK(obj.scale  == 3.14);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// writeConfigFile: error branches

TEST_CASE("GParserBuilder::writeConfigFile throws when target is an existing directory",
          "[common][parser-builder]") {
    // The scratch helper guarantees this directory exists.
    auto dir = std::filesystem::temp_directory_path() / "geneva_parserbuilder_tests";
    std::filesystem::create_directories(dir);

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 1);
    CHECK_THROWS_AS(gpb.writeConfigFile(dir), geneva_exception);
}

TEST_CASE("GParserBuilder::writeConfigFile throws when target file already exists",
          "[common][parser-builder]") {
    auto cfg = scratch("already_exists");
    { std::ofstream ofs(cfg); ofs << "{}"; }
    REQUIRE(std::filesystem::exists(cfg));

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 1);
    CHECK_THROWS_AS(gpb.writeConfigFile(cfg), geneva_exception);
    std::filesystem::remove(cfg);
}

TEST_CASE("GParserBuilder::writeConfigFile creates a missing parent directory",
          "[common][parser-builder]") {
    // A missing parent directory is created for the user (with a logged note) rather than treated
    // as an error -- this is what lets a fresh ./config directory come into being on first run.
    auto missing_dir = std::filesystem::temp_directory_path() / "gpb_no_such_parent_dir_xyz";
    std::filesystem::remove_all(missing_dir);
    auto cfg = missing_dir / "file.json";

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 1);
    CHECK_NOTHROW(gpb.writeConfigFile(cfg));
    CHECK(std::filesystem::is_directory(missing_dir));
    CHECK(std::filesystem::exists(cfg));

    std::filesystem::remove_all(missing_dir);
}

TEST_CASE("GParserBuilder::writeConfigFile throws when the parent path is not a directory",
          "[common][parser-builder]") {
    // If the intended parent path already exists but is a regular file, it cannot be turned into a
    // configuration directory -- that is a genuine error.
    auto not_a_dir = std::filesystem::temp_directory_path() / "gpb_parent_is_a_file_xyz";
    std::filesystem::remove_all(not_a_dir);
    { std::ofstream ofs(not_a_dir); ofs << "x"; }
    REQUIRE(std::filesystem::is_regular_file(not_a_dir));
    auto cfg = not_a_dir / "file.json";

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 1);
    CHECK_THROWS_AS(gpb.writeConfigFile(cfg), geneva_exception);

    std::filesystem::remove(not_a_dir);
}

TEST_CASE("GParserBuilder::writeConfigFile throws for a non-.json extension",
          "[common][parser-builder]") {
    auto base = std::filesystem::temp_directory_path() / "geneva_parserbuilder_tests";
    std::filesystem::create_directories(base);
    auto cfg = base / "config_bad_ext.txt";
    std::filesystem::remove(cfg);

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 1);
    CHECK_THROWS_AS(gpb.writeConfigFile(cfg), geneva_exception);
}

TEST_CASE("GParserBuilder::writeConfigFile with writeAll=false omits secondary parameters",
          "[common][parser-builder]") {
    auto cfg = scratch("write_not_all");
    std::filesystem::remove(cfg);

    {
        GParserBuilder gpb;
        int e = 0;
        int s = 0;
        gpb.registerFileParameter<int>("essential_p", e, 42, VAR_IS_ESSENTIAL, "essential");
        gpb.registerFileParameter<int>("secondary_p", s, 99, VAR_IS_SECONDARY, "secondary");
        gpb.writeConfigFile(cfg, "", false); // writeAll=false skips secondary
    }

    // Re-parse: essential should be 42; secondary key absent → default stays 0
    GParserBuilder gpb2;
    int e2 = 0;
    int s2 = 0;
    gpb2.registerFileParameter<int>("essential_p", e2, 0);
    gpb2.registerFileParameter<int>("secondary_p", s2, 0);
    REQUIRE(gpb2.parseConfigFile(cfg));
    CHECK(e2 == 42);
    CHECK(s2 == 0);
    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// parseConfigFile: error branches

TEST_CASE("GParserBuilder::parseConfigFile returns false when path is an existing directory",
          "[common][parser-builder]") {
    // Any existing directory makes is_regular_file() return false → internal
    // throw is caught → parseConfigFile returns false.
    auto dir = std::filesystem::temp_directory_path() / "geneva_parserbuilder_tests";
    std::filesystem::create_directories(dir);

    GParserBuilder gpb;
    int v = 0;
    gpb.registerFileParameter<int>("v", v, 1);
    CHECK_FALSE(gpb.parseConfigFile(dir));
}

// ---------------------------------------------------------------------------
// parseCommandLine: verbose=true branch

TEST_CASE("GParserBuilder::parseCommandLine with verbose=true produces no exception",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    int v = 0;
    gpb.registerCLParameter<int>("k", v, 7);
    char prog[] = "prog";
    char *argv[] = {prog, nullptr};
    CHECK_FALSE(gpb.parseCommandLine(1, argv, /*verbose*/ true));
    CHECK(v == 7);
}

// ---------------------------------------------------------------------------
// resetFileParameterDefaults: missing-key error for combined and array forms

TEST_CASE("GParserBuilder::resetFileParameterDefaults (combined form): missing option throws",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    CHECK_THROWS_AS(
        (gpb.resetFileParameterDefaults<int, double>("nope", 1, 2.0)),
        geneva_exception
    );
}

TEST_CASE("GParserBuilder::resetFileParameterDefaults (array form): missing option throws",
          "[common][parser-builder]") {
    GParserBuilder gpb;
    CHECK_THROWS_AS(
        (gpb.resetFileParameterDefaults<int, 2>("nope", std::array<int, 2>{1, 2})),
        geneva_exception
    );
}

// ---------------------------------------------------------------------------
// updateConfigFile: the update-in-place contract (drop stale keys, preserve
// existing values, add newly-registered keys with defaults).

TEST_CASE("GParserBuilder::updateConfigFile drops stale keys, preserves values, defaults new keys",
          "[common][parser-builder]") {
    namespace pt = boost::property_tree;
    auto cfg = scratch("update_inplace");
    std::filesystem::remove(cfg);

    // Write a v1 config by hand: a CUSTOMIZED "answer" value (13, not the default 42), a normal
    // "scale", and a "stale_key" that the v2 schema below no longer registers.
    {
        pt::ptree t;
        t.put("answer.default", "42");
        t.put("answer.value",   "13"); // customized on disk
        t.put("scale.default",  "3.14");
        t.put("scale.value",    "3.14");
        t.put("stale_key.default", "999");
        t.put("stale_key.value",   "999");
        pt::write_json(cfg.string(), t);
    }

    // v2 schema: {answer, scale, fresh} -- "stale_key" gone, "fresh" new.
    GParserBuilder gpb;
    int    answer = 0;
    double scale  = 0.0;
    int    fresh  = 0;
    gpb.registerFileParameter<int>   ("answer", answer, 42,   VAR_IS_ESSENTIAL, "the int");
    gpb.registerFileParameter<double>("scale",  scale,  3.14, VAR_IS_ESSENTIAL, "the double");
    gpb.registerFileParameter<int>   ("fresh",  fresh,  7,    VAR_IS_ESSENTIAL, "new in v2");

    REQUIRE(gpb.updateConfigFile(cfg));

    // Applied (in-memory) values: existing preserved, new defaulted.
    CHECK(answer == 13);   // preserved from disk (not reset to the 42 default)
    CHECK(scale  == 3.14);
    CHECK(fresh  == 7);    // absent from v1 -> registered default

    // On-disk shape after the rewrite.
    pt::ptree after;
    pt::read_json(cfg.string(), after);
    CHECK(after.get<int>("answer.value")   == 13);   // value preserved on disk
    CHECK(after.get<double>("scale.value") == 3.14);
    CHECK(after.get<int>("fresh.value")    == 7);    // new key present, defaulted
    CHECK_FALSE(after.get_child_optional("stale_key").has_value()); // stale key dropped
    CHECK(after.get_child_optional("header").has_value());          // canonical header written

    std::filesystem::remove(cfg);
}

// ---------------------------------------------------------------------------
// Regression (vector load_from): a config that lacks a registered vector
// parameter must keep that parameter's defaults instead of throwing. Guards the
// get_child_optional() fix that makes update-in-place work when a NEW vector
// parameter is added to the schema.

TEST_CASE("GParserBuilder: a config missing a registered vector parameter keeps its defaults",
          "[common][parser-builder]") {
    namespace pt = boost::property_tree;
    auto cfg = scratch("vec_newkey");
    std::filesystem::remove(cfg);

    // A config with a scalar but WITHOUT the vector key "vints".
    {
        pt::ptree t;
        t.put("other.default", "5");
        t.put("other.value",   "5");
        pt::write_json(cfg.string(), t);
    }

    GParserBuilder gpb;
    int              other = 0;
    std::vector<int> vints;
    gpb.registerFileParameter<int>("other", other, 5, VAR_IS_ESSENTIAL, "scalar");
    gpb.registerFileParameter<int>("vints", vints, std::vector<int>{10, 20, 30}, VAR_IS_ESSENTIAL, "a vector");

    // Before the fix this threw boost::property_tree::ptree_bad_path and aborted the parse.
    REQUIRE(gpb.parseConfigFile(cfg));
    CHECK(other == 5);
    CHECK(vints == std::vector<int>{10, 20, 30}); // the registered defaults survive

    std::filesystem::remove(cfg);
}
