################################################################################
#
# Driver for the GPlotDesignerGnuplotValidity ctest. It confirms that the gnuplot
# script GPlotDesigner generates (via the GNUPLOT backend) is VALID gnuplot input
# by feeding it to the real gnuplot interpreter and checking the rendered artifact.
#
# The generated script is terminal-agnostic; this driver prepends a pngcairo
# terminal + output file before handing it to gnuplot. gnuplot renders reliably
# headless (no xvfb needed), so success is asserted via both a produced PNG and a
# zero exit code.
#
# Inputs (passed via -D): GEN_EXE, GNUPLOT_EXE, WORKDIR.
#
################################################################################

# 1. Run the generator -> WORKDIR/result.gp (it also writes result.C).
execute_process(
	COMMAND "${GEN_EXE}"
	WORKING_DIRECTORY "${WORKDIR}"
	RESULT_VARIABLE gen_rc
)
if(NOT gen_rc EQUAL 0)
	message(FATAL_ERROR "GPlotDesignerTest generator failed (rc=${gen_rc})")
endif()
if(NOT EXISTS "${WORKDIR}/result.gp")
	message(FATAL_ERROR "generator did not produce result.gp in ${WORKDIR}")
endif()

# 2. Build a probe script: a pngcairo terminal + output prepended to the generated,
#    terminal-agnostic body (the body already ends with `unset multiplot`).
file(READ "${WORKDIR}/result.gp" _script)
file(WRITE "${WORKDIR}/gpd_gnuplot_validate.gp"
	"set terminal pngcairo size 800,600\nset output 'gpd_gnuplot.png'\n${_script}")
file(REMOVE "${WORKDIR}/gpd_gnuplot.png")

# 3. Run it through gnuplot headless.
execute_process(
	COMMAND "${GNUPLOT_EXE}" gpd_gnuplot_validate.gp
	WORKING_DIRECTORY "${WORKDIR}"
	RESULT_VARIABLE _gp_rc
	OUTPUT_VARIABLE _out
	ERROR_VARIABLE _err
	TIMEOUT 300
)

# 4. Validate via both gnuplot's exit code and the rendered artifact.
if(NOT _gp_rc EQUAL 0)
	message(FATAL_ERROR
		"gnuplot exited non-zero (rc=${_gp_rc}) -- the generated script is not valid gnuplot input.\n"
		"stdout:\n${_out}\nstderr:\n${_err}")
endif()
if(NOT EXISTS "${WORKDIR}/gpd_gnuplot.png")
	message(FATAL_ERROR
		"gnuplot did not render the generated script -- no PNG was produced.\n"
		"stdout:\n${_out}\nstderr:\n${_err}")
endif()
file(SIZE "${WORKDIR}/gpd_gnuplot.png" _sz)
if(_sz LESS 1000)
	message(FATAL_ERROR "gnuplot produced a suspiciously small PNG (${_sz} bytes); the script likely did not render.")
endif()
message(STATUS "GPlotDesignerGnuplotValidity: gnuplot rendered a ${_sz}-byte PNG from the generated script.")
