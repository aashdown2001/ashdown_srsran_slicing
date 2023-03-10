#
# Copyright 2021-2023 Software Radio Systems Limited
#
# This file is part of srsRAN
#
# srsRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# srsRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#


find_package(Doxygen OPTIONAL_COMPONENTS dot)

if (DOXYGEN_FOUND)
  # Configure doxygen
  set(DOXYGEN_PROJECT_NUMBER ${VERSION})
  set(DOXYGEN_PROJECT_LOGO ${CMAKE_CURRENT_SOURCE_DIR}/srsRan_logo.png)
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set(DOXYGEN_WARN_LOGFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen.log)
  set(DOXYGEN_USE_MATHJAX "YES")
  set(DOXYGEN_MATHJAX_EXTENSIONS "TeX/AMSmath")
  set(DOXYGEN_FORMULA_MACROFILE ${CMAKE_CURRENT_SOURCE_DIR}/math_macros.tex)
  set(DOXYGEN_STRIP_FROM_PATH ${CMAKE_HOME_DIRECTORY})
  set(DOXYGEN_EXTRACT_STATIC "YES")
  set(DOXYGEN_EXTRACT_PRIVATE "YES")
  set(DOXYGEN_GENERATE_TREEVIEW "YES")
  set(DOXYGEN_GENERATE_XML "YES")
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_GENERATE_TREEVIEW "YES")
    set(DOXYGEN_HTML_EXTRA_STYLESHEET "doxygen-awesome.css")
  endif()
  set(DOXYGEN_UML_LOOK "YES")
  set(DOXYGEN_CALL_GRAPH "YES")
  set(DOXYGEN_CALLER_GRAPH "YES")
  set(DOXYGEN_DISTRIBUTE_GROUP_DOC "YES")
  set(DOXYGEN_PREDEFINED "HAVE_AVX2:=1")

  # find plantuml
  find_program(PLANTUML_PATH plantuml.jar PATH_SUFFIXES PlantUML plantuml Plantuml
          PATHS /usr/share /usr/local/share /usr/local/bin /opt/local/share/java)
  if (PLANTUML_PATH)
    message(STATUS "PLANTUML found in ${PLANTUML_PATH}.")
  endif ()
  set(DOXYGEN_PLANTUML_JAR_PATH "${PLANTUML_PATH}")

  # directories and files to process for all the docs
  set(docs_inputs ${CMAKE_CURRENT_SOURCE_DIR}
                  ${CMAKE_HOME_DIRECTORY}/include/srsran
                  ${CMAKE_HOME_DIRECTORY}/lib
  )
  set(DOXYGEN_EXCLUDE ${CMAKE_HOME_DIRECTORY}/include/srsran/asn1)

  doxygen_add_docs(doxygen
    ${docs_inputs}
    COMMENT "Generating all documentation."
  )

  # directories and files to process for FAPI related docs
  set(docs_inputs_fapi ${CMAKE_CURRENT_SOURCE_DIR}
                       ${CMAKE_HOME_DIRECTORY}/include/srsran/fapi
                       ${CMAKE_HOME_DIRECTORY}/include/srsran/fapi_adaptor
                       ${CMAKE_HOME_DIRECTORY}/lib/fapi
                       ${CMAKE_HOME_DIRECTORY}/lib/fapi_adaptor
  )

  doxygen_add_docs(doxygen-fapi
    ${docs_inputs_fapi}
    COMMENT "Generating FAPI-related documentation."
  )

  # directories and files to process for MAC+RLC docs
  set(docs_inputs_mac ${CMAKE_CURRENT_SOURCE_DIR}
                      ${CMAKE_HOME_DIRECTORY}/include/srsran/mac
                      ${CMAKE_HOME_DIRECTORY}/include/srsran/rlc
                      ${CMAKE_HOME_DIRECTORY}/lib/mac
                      ${CMAKE_HOME_DIRECTORY}/lib/rlc
  )

  doxygen_add_docs(doxygen-du-high
    ${docs_inputs_mac}
    COMMENT "Generating documentation for layers MAC and RLC."
  )

  # directories and files to process for PHY+RAN docs
  set(docs_inputs_phy ${CMAKE_CURRENT_SOURCE_DIR}
                      ${CMAKE_HOME_DIRECTORY}/include/srsran/phy
                      ${CMAKE_HOME_DIRECTORY}/include/srsran/ran
                      ${CMAKE_HOME_DIRECTORY}/include/srsran/radio
                      ${CMAKE_HOME_DIRECTORY}/lib/phy
                      ${CMAKE_HOME_DIRECTORY}/lib/ran
                      ${CMAKE_HOME_DIRECTORY}/lib/radio
  )

  doxygen_add_docs(doxygen-phy
    ${docs_inputs_phy}
    COMMENT "Generating documentation for layers PHY and radio."
  )

  # directories and files to process for support-related docs
  set(docs_inputs_support ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_HOME_DIRECTORY}/include/srsran/adt
                          ${CMAKE_HOME_DIRECTORY}/include/srsran/srslog
                          ${CMAKE_HOME_DIRECTORY}/include/srsran/srsvec
                          ${CMAKE_HOME_DIRECTORY}/include/srsran/support
                          ${CMAKE_HOME_DIRECTORY}/lib/srslog
                          ${CMAKE_HOME_DIRECTORY}/lib/srsvec
                          ${CMAKE_HOME_DIRECTORY}/lib/support
  )

  doxygen_add_docs(doxygen-support
    ${docs_inputs_support}
    COMMENT "Generating documentation for support libraries."
  )

  add_custom_target(doxygen-clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/html
                                       ${CMAKE_CURRENT_BINARY_DIR}/doxygen.log
    COMMENT "Cleaning documentation"
    VERBATIM
  )
endif(DOXYGEN_FOUND)
