# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# This file sets up six lists:
# MAIN_SOURCE_FILES     List of compilation units which will be included in
#                       the library. If it isn't on this list, it won't be
#                       part of the library. Please try to keep it sorted to
#                       maintain sanity.
#
# TEST_SOURCE_FILES     List of programs that will be run as unit tests.
#
# TEST_DATA_FILES       Files from the source three that should be made
#                       available in the corresponding location in the build
#                       tree in order to run tests there.
#
# EXAMPLE_SOURCE_FILES  Other programs that will be compiled as part of the
#                       build, but which is not part of the library nor is
#                       run as tests.
#
# PUBLIC_HEADER_FILES   List of public header files that should be
#                       distributed together with the library. The source
#                       files can of course include other files than these;
#                       you should only add to this list if the *user* of
#                       the library needs it.
#
# ATTIC_FILES           Unmaintained files. This for the projects developers
#                       only. Don't expect these files to build.

# originally generated with the command:
# find dune -name '*.c*' -printf '\t%p\n' | sort
list (APPEND MAIN_SOURCE_FILES
  opm/grid/cpgrid/Intersection.cpp
  opm/grid/cpgrid/CpGridData.cpp
  opm/grid/cpgrid/CpGrid.cpp
  opm/grid/cpgrid/GridHelpers.cpp
  opm/grid/cpgrid/PartitionTypeIndicator.cpp
  opm/grid/cpgrid/processEclipseFormat.cpp
  opm/grid/cpgrid/readSintefLegacyFormat.cpp
  opm/grid/cpgrid/writeSintefLegacyFormat.cpp
  opm/grid/common/GeometryHelpers.cpp
  opm/grid/common/GridPartitioning.cpp
  opm/grid/common/WellConnections.cpp
  opm/grid/common/ZoltanGraphFunctions.cpp
  opm/grid/common/ZoltanPartition.cpp
  opm/grid/GridHelpers.cpp
  opm/grid/GridManager.cpp
  opm/grid/GridUtilities.cpp
  opm/grid/cart_grid.c
  opm/grid/cornerpoint_grid.c
  opm/grid/cpgpreprocess/facetopology.c
  opm/grid/cpgpreprocess/geometry.c
  opm/grid/cpgpreprocess/preprocess.c
  opm/grid/cpgpreprocess/uniquepoints.c
  opm/grid/UnstructuredGrid.c
  opm/grid/grid_equal.cpp
  opm/grid/utility/compressedToCartesian.cpp
  opm/grid/utility/cartesianToCompressed.cpp
  opm/grid/utility/StopWatch.cpp
  opm/grid/utility/WachspressCoord.cpp
  opm/grid/verteq/topsurf.cpp
  opm/grid/verteq/nav.cpp
  opm/grid/verteq/utility/exc.cpp
  opm/grid/verteq/utility/runlen.cpp
  )

if (opm-common_FOUND)
  list(APPEND MAIN_SOURCE_FILES
		opm/grid/utility/VelocityInterpolation.cpp
		opm/grid/transmissibility/trans_tpfa.c)
endif()

if(HAVE_ECL_INPUT)
  list(APPEND MAIN_SOURCE_FILES opm/grid/utility/extractPvtTableIndex.cpp)
endif()

# originally generated with the command:
# find tests/not-unit/ -name \*.cpp -o \*.cc
list (APPEND ATTIC_FILES
  attic/partition_test.cpp
# attic/dumux_test.cpp
  attic/mapper_test.cpp
  attic/buildcpgrid_test.cpp
# attic/cpgrid_test.cpp
  attic/check_grid_normals.cpp
  )

# originally generated with the command:
# find tests -name '*.cpp' -a ! -wholename '*/not-unit/*' -printf '\t%p\n' | sort
list (APPEND TEST_SOURCE_FILES
  tests/test_cartgrid.cpp
  tests/test_column_extract.cpp
  tests/cpgrid/distribution_test.cpp
  tests/cpgrid/entityrep_test.cpp
  tests/cpgrid/entity_test.cpp
  tests/cpgrid/facetag_test.cpp
  tests/cpgrid/geometry_test.cpp
  tests/cpgrid/orientedentitytable_test.cpp
  tests/cpgrid/partition_iterator_test.cpp
  tests/cpgrid/zoltan_test.cpp
  tests/test_geom2d.cpp
  tests/test_gridutilities.cpp
  tests/test_minpvprocessor.cpp
  tests/grid_test.cc
  tests/p2pcommunicator_test.cc
  tests/test_repairzcorn.cpp
  tests/test_sparsetable.cpp
  tests/test_quadratures.cpp
  tests/test_compressed_cartesian_mapping.cpp
	)

if(HAVE_ECL_INPUT)
  list(APPEND TEST_SOURCE_FILES
		tests/test_regionmapping.cpp
		tests/test_ug.cpp
		tests/test_compressedpropertyaccess.cpp
                tests/cpgrid/grid_nnc.cpp
	)
endif()

# originally generated with the command:
# find tests -name '*.xml' -a ! -wholename '*/not-unit/*' -printf '\t%p\n' | sort
list (APPEND TEST_DATA_FILES
     tests/CORNERPOINT_ACTNUM.DATA
     tests/compressed_gridproperty.data
     tests/FIVE.DATA
     tests/FIVE_ACTNUM.DATA
     tests/FIVE_PINCH.DATA
  )

# originally generated with the command:
# find tutorials examples -name '*.c*' -printf '\t%p\n' | sort
list (APPEND EXAMPLE_SOURCE_FILES
  examples/finitevolume/finitevolume.cc
  examples/mirror_grid.cpp
  )

# programs listed here will not only be compiled, but also marked for
# installation
list (APPEND PROGRAM_SOURCE_FILES
  examples/mirror_grid.cpp
  )
if(HAVE_ECL_INPUT)
  list(APPEND EXAMPLE_SOURCE_FILES examples/grdecl2vtu.cpp)
  list(APPEND PROGRAM_SOURCE_FILES examples/grdecl2vtu.cpp)
endif()

# originally generated with the command:
# find dune -name '*.h*' -a ! -name '*-pch.hpp' -printf '\t%p\n' | sort
list (APPEND PUBLIC_HEADER_FILES
  opm/grid/common/GeometryHelpers.hpp
  opm/grid/common/GridAdapter.hpp
  opm/grid/common/GridPartitioning.hpp
  opm/grid/common/Volumes.hpp
  opm/grid/common/p2pcommunicator.hh
  opm/grid/common/p2pcommunicator_impl.hh
  opm/grid/cpgrid/CartesianIndexMapper.hpp
  opm/grid/cpgrid/CpGridData.hpp
  opm/grid/cpgrid/DefaultGeometryPolicy.hpp
  opm/grid/cpgrid/dgfparser.hh
  opm/grid/cpgrid/Entity2IndexDataHandle.hpp
  opm/grid/cpgrid/Entity.hpp
  opm/grid/cpgrid/EntityRep.hpp
  opm/grid/cpgrid/Geometry.hpp
  opm/grid/cpgrid/GlobalIdMapping.hpp
  opm/grid/cpgrid/GridHelpers.hpp
  opm/grid/CpGrid.hpp
  opm/grid/cpgrid/Indexsets.hpp
  opm/grid/cpgrid/Intersection.hpp
  opm/grid/cpgrid/Iterators.hpp
  opm/grid/cpgrid/OrientedEntityTable.hpp
  opm/grid/cpgrid/PartitionIteratorRule.hpp
  opm/grid/cpgrid/PartitionTypeIndicator.hpp
  opm/grid/cpgrid/PersistentContainer.hpp
  opm/grid/common/CartesianIndexMapper.hpp
  opm/grid/common/WellConnections.hpp
  opm/grid/common/ZoltanGraphFunctions.hpp
  opm/grid/common/ZoltanPartition.hpp
  opm/grid/polyhedralgrid/capabilities.hh
  opm/grid/polyhedralgrid/cartesianindexmapper.hh
  opm/grid/polyhedralgrid/declaration.hh
  opm/grid/polyhedralgrid/dgfparser.hh
  opm/grid/polyhedralgrid/entity.hh
  opm/grid/polyhedralgrid/entitypointer.hh
  opm/grid/polyhedralgrid/entityseed.hh
  opm/grid/polyhedralgrid/geometry.hh
  opm/grid/polyhedralgrid/gridhelpers.hh
  opm/grid/polyhedralgrid/grid.hh
  opm/grid/polyhedralgrid/gridview.hh
  opm/grid/polyhedralgrid.hh
  opm/grid/polyhedralgrid/idset.hh
  opm/grid/polyhedralgrid/indexset.hh
  opm/grid/polyhedralgrid/intersection.hh
  opm/grid/polyhedralgrid/intersectioniterator.hh
  opm/grid/polyhedralgrid/iterator.hh
  opm/grid/polyhedralgrid/persistentcontainer.hh
  opm/grid/UnstructuredGrid.h
  opm/grid/CellQuadrature.hpp
  opm/grid/ColumnExtract.hpp
  opm/grid/FaceQuadrature.hpp
  opm/grid/GridHelpers.hpp
  opm/grid/GridManager.hpp
  opm/grid/GridUtilities.hpp
  opm/grid/MinpvProcessor.hpp
  opm/grid/PinchProcessor.hpp
  opm/grid/RepairZCORN.hpp
  opm/grid/cart_grid.h
  opm/grid/cornerpoint_grid.h
  opm/grid/cpgpreprocess/facetopology.h
  opm/grid/cpgpreprocess/geometry.h
  opm/grid/cpgpreprocess/preprocess.h
  opm/grid/cpgpreprocess/uniquepoints.h
  opm/grid/transmissibility/trans_tpfa.h
  opm/grid/transmissibility/TransTpfa.hpp
  opm/grid/transmissibility/TransTpfa_impl.hpp
  opm/grid/utility/CompressedPropertyAccess.hpp
  opm/grid/utility/compressedToCartesian.hpp
  opm/grid/utility/cartesianToCompressed.hpp
  opm/grid/utility/extractPvtTableIndex.hpp
  opm/grid/utility/RegionMapping.hpp
  opm/grid/utility/SparseTable.hpp
  opm/grid/utility/StopWatch.hpp
  opm/grid/utility/VelocityInterpolation.hpp
  opm/grid/utility/WachspressCoord.hpp
  opm/grid/utility/ErrorMacros.hpp
  opm/grid/utility/OpmParserIncludes.hpp
  opm/grid/utility/platform_dependent/disable_warnings.h
  opm/grid/utility/platform_dependent/reenable_warnings.h
  opm/grid/verteq/topsurf.hpp
  opm/grid/verteq/nav.hpp
  opm/grid/verteq/utility/visibility.h
  opm/grid/verteq/utility/exc.hpp
  opm/grid/verteq/utility/runlen.hpp
  )
