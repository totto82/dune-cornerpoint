/*
  Copyright 2015 Dr. Blatt - HPC-Simulation-Software & Services.
  Copyright 2015 NTNU
  Copyright 2015 Statoil AS

  This file is part of The Open Porous Media project  (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <limits>

#include <opm/grid/utility/OpmParserIncludes.hpp>

#include <dune/grid/common/ZoltanGraphFunctions.hpp>
#include <dune/common/parallel/indexset.hh>

#if defined(HAVE_ZOLTAN) && defined(HAVE_MPI)

namespace Dune
{
namespace cpgrid
{

void getNullVertexList(void* cpGridPointer, int numGlobalIdEntries,
                       int numLocalIdEntries, ZOLTAN_ID_PTR gids,
                       ZOLTAN_ID_PTR lids, int wgtDim,
                       float *objWgts, int *err)
{
    (void) cpGridPointer; (void) numGlobalIdEntries;
    (void) numLocalIdEntries; (void) gids; (void) lids; (void) objWgts;
    (void) wgtDim;
    // We do nothing as we pretend to not have any grid cells.
    *err = ZOLTAN_OK;
}

void getCpGridVertexList(void* cpGridPointer, int numGlobalIdEntries,
                         int numLocalIdEntries, ZOLTAN_ID_PTR gids,
                         ZOLTAN_ID_PTR lids, int wgtDim,
                         float *objWgts, int *err)
{
    (void) wgtDim; (void) objWgts;
    const Dune::CpGrid&  grid = *static_cast<const Dune::CpGrid*>(cpGridPointer);
    auto& globalIdSet         =  grid.globalIdSet();
    auto& localIdSet          =  grid.localIdSet();

    if ( numGlobalIdEntries != numLocalIdEntries || numGlobalIdEntries != 1 )
    {
        std::cerr<<"numGlobalIdEntries="<<numGlobalIdEntries<<" numLocalIdEntries="<<numLocalIdEntries<<" grid cells="
                 <<grid.numCells()<<std::endl;
        *err = ZOLTAN_FATAL;
        return;
    }
    int idx = 0;
    for (auto cell = grid.leafbegin<0>(), cellEnd = grid.leafend<0>();
         cell != cellEnd; ++cell)
    {
        gids[idx]   = globalIdSet.id(*cell);
        lids[idx++] = localIdSet.id(*cell);
    }
    *err = ZOLTAN_OK;
}

void getNullNumEdgesList(void *cpGridPointer, int sizeGID, int sizeLID,
                           int numCells,
                           ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                           int *numEdges, int *err)
{
    (void) sizeGID; (void) sizeLID; (void) numCells; (void) globalID;
    (void) localID; (void) numEdges; (void) cpGridPointer;
    // Pretend that there are no edges
    numEdges = 0;
    *err = ZOLTAN_OK;
}

void getCpGridNumEdgesList(void *cpGridPointer, int sizeGID, int sizeLID,
                           int numCells,
                           ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                           int *numEdges, int *err)
{
    (void) globalID;
    const Dune::CpGrid&  grid = *static_cast<const Dune::CpGrid*>(cpGridPointer);
    if ( sizeGID != 1 || sizeLID != 1 || numCells != grid.numCells() )
    {
        *err = ZOLTAN_FATAL;
        return;
    }
    for( int i = 0; i < numCells;  i++ )
    {
        // For the graph there is an edge only if the face has two neighbors.
        // Therefore we need to check each face
        int edges = 0;
        int lid   = localID[i];
        for ( int local_face = 0; local_face < grid.numCellFaces(static_cast<int>(localID[i])); ++local_face )
        {
            const int face = grid.cellFace(lid, local_face);
            if ( grid.faceCell(face, 0) != -1 && grid.faceCell(face, 1) != -1 )
            {
                ++edges;
            }
        }
        numEdges[i] = edges;
    }
    *err = ZOLTAN_OK;
}

void getCpGridWellsNumEdgesList(void *graphPointer, int sizeGID, int sizeLID,
                           int numCells,
                           ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                           int *numEdges, int *err)
{
    (void) globalID;
    const CombinedGridWellGraph& graph =
        *static_cast<CombinedGridWellGraph*>(graphPointer);
    const Dune::CpGrid&  grid = graph.getGrid();
    if ( sizeGID != 1 || sizeLID != 1 || numCells != grid.numCells() )
    {
        *err = ZOLTAN_FATAL;
        return;
    }
    for( int i = 0; i < numCells;  i++ )
    {
        // Initial set of faces is the ones of the well completions
        auto edges = graph.getWellsGraph()[i];
        // For the graph there is an edge only if the face has two neighbors.
        // Therefore we need to check each face
        int lid   = localID[i];
        for ( int local_face = 0; local_face < grid.numCellFaces(static_cast<int>(localID[i])); ++local_face )
        {
            const int face  = grid.cellFace(lid, local_face);
            const int face0 = grid.faceCell(face, 0);
            const int face1 = grid.faceCell(face, 1);

            if ( face0 != -1 && face1 != -1 )
            {
                if ( face0 != i )
                    edges.insert(face0);
                else
                    edges.insert(face1);
            }
        }
        numEdges[i] = edges.size();
    }
    *err = ZOLTAN_OK;
}

void getNullEdgeList(void *cpGridPointer, int sizeGID, int sizeLID,
                       int numCells, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                       int *numEdges,
                       ZOLTAN_ID_PTR nborGID, int *nborProc,
                       int wgtDim, float *ewgts, int *err)
{
    (void) cpGridPointer; (void) sizeGID; (void) sizeLID; (void) numCells;
    (void) globalID; (void) localID; (void) numEdges; (void) nborGID;
    (void) nborProc; (void) wgtDim; (void) ewgts;
    *err = ZOLTAN_OK;
}

void getCpGridEdgeList(void *cpGridPointer, int sizeGID, int sizeLID,
                       int numCells, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                       int *numEdges,
                       ZOLTAN_ID_PTR nborGID, int *nborProc,
                       int wgtDim, float *ewgts, int *err)
{
    (void) wgtDim; (void) globalID; (void) numEdges; (void) ewgts;
    const Dune::CpGrid&  grid = *static_cast<const Dune::CpGrid*>(cpGridPointer);
    if ( sizeGID != 1 || sizeLID != 1 || numCells != grid.numCells() )
    {
        *err = ZOLTAN_FATAL;
        return;
    }
#ifndef NDEBUG
    int oldidx = 0;
#endif
    int idx = 0;

    for( int cell = 0; cell < numCells;  cell++ )
    {
        const int currentCell = localID[cell];
        for ( int local_face = 0 ; local_face < grid.numCellFaces(static_cast<int>(localID[cell])); ++local_face )
        {
            const int face  = grid.cellFace(currentCell, local_face);
            int otherCell   = grid.faceCell(face, 0);
            if ( otherCell == currentCell || otherCell == -1 )
            {
                otherCell = grid.faceCell(face, 1);
                if ( otherCell == currentCell || otherCell == -1 )
                {
                    continue;
                }
                else
                {
                    nborGID[idx++] = globalID[otherCell];
                    continue;
                }
            }
            nborGID[idx++] = globalID[otherCell];
        }
#ifndef NDEBUG
        assert(numEdges[cell] == idx - oldidx);
        oldidx = idx;
#endif
    }

    const int myrank = grid.comm().rank();

    for ( int i = 0; i < idx; ++i )
    {
        nborProc[i] = myrank;
    }
#if defined(DEBUG) && false // The index set will not be initialized here!
    // The above relies heavily on the grid not being distributed already.
    // Therefore we check here that all cells are owned by us.
    GlobalLookupIndexSet<Dune::CpGrid::ParallelIndexSet>
        globalIdxSet(grid.getCellIndexSet(),
                     grid.numCells());
    for ( int cell = 0; cell < numCells;  cell++ )
    {
        if ( globalIdxSet.pair(cell)->local().attribute() !=
             Dune::CpGrid::ParallelIndexSet::LocalIndex::Attribute::owner )
        {
            *err = ZOLTAN_FATAL;
        }
    }
#endif
}

void getCpGridWellsEdgeList(void *graphPointer, int sizeGID, int sizeLID,
                       int numCells, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                       int *numEdges,
                       ZOLTAN_ID_PTR nborGID, int *nborProc,
                       int wgtDim, float *ewgts, int *err)
{
    (void) wgtDim; (void) globalID; (void) numEdges; (void) ewgts;
    assert(wgtDim==1);
    const CombinedGridWellGraph& graph =
        *static_cast<const CombinedGridWellGraph*>(graphPointer);
    const Dune::CpGrid&  grid = graph.getGrid();

    if ( sizeGID != 1 || sizeLID != 1 || numCells != grid.numCells() )
    {
        *err = ZOLTAN_FATAL;
        return;
    }
#ifndef NDEBUG
    int oldidx = 0;
#endif
    int idx = 0;

    for( int cell = 0; cell < numCells;  cell++ )
    {
        const int currentCell = localID[cell];

        // First the strong edges of the well completions.
        auto wellEdges = graph.getWellsGraph()[currentCell];
        for( auto edge : wellEdges)
        {
            nborGID[idx] = edge;
            ewgts[idx++] = std::numeric_limits<float>::max();
        }

        // Now the ones of the grid that are not handled by the well completions
        for ( int local_face = 0 ; local_face < grid.numCellFaces(static_cast<int>(localID[cell])); ++local_face )
        {
            const int face  = grid.cellFace(currentCell, local_face);
            int otherCell   = grid.faceCell(face, 0);
            if ( otherCell == currentCell || otherCell == -1 )
            {
                otherCell = grid.faceCell(face, 1);
                if ( otherCell == currentCell || otherCell == -1 )
                {
                    // no real face or already handled by well
                    continue;
                }
                else
                {
                    if ( wellEdges.find(otherCell) == wellEdges.end() )
                    {
                        nborGID[idx] = globalID[otherCell];
                        if(~wellEdges.empty())
                            ewgts[idx++] = std::numeric_limits<float>::max();
                        else
                            ewgts[idx++] = graph.transmissibility(face);



                    }
                    continue;
                }
            }
            if ( wellEdges.find(otherCell) == wellEdges.end() )
            {
                nborGID[idx] = globalID[otherCell];
                if(~wellEdges.empty())
                    ewgts[idx++] = std::numeric_limits<float>::max();
                else
                    ewgts[idx++] = graph.transmissibility(face);
            }
        }
#ifndef NDEBUG
        assert(idx-oldidx==numEdges[cell]);
        oldidx = idx;
#endif
    }

    const int myrank = grid.comm().rank();

    for ( int i = 0; i < idx; ++i )
    {
        nborProc[i] = myrank;
    }
#if defined(DEBUG) && false // The index set will not be initialized here!
    // The above relies heavily on the grid not being distributed already.
    // Therefore we check here that all cells are owned by us.
    GlobalLookupIndexSet<Dune::CpGrid::ParallelIndexSet>
        globalIdxSet(grid.getCellIndexSet(),
                     grid.numCells());
    for ( int cell = 0; cell < numCells;  cell++ )
    {
        if ( globalIdxSet.pair(cell)->local().attribute() !=
             Dune::CpGrid::ParallelIndexSet::LocalIndex::Attribute::owner )
        {
            *err = ZOLTAN_FATAL;
        }
    }
#endif
}

CombinedGridWellGraph::CombinedGridWellGraph(const CpGrid& grid,
                                             const std::vector<const OpmWellType*> * wells,
                                             const double* transmissibilities,
                                             bool pretendEmptyGrid)
    : grid_(grid), transmissibilities_(transmissibilities)
{
    if ( pretendEmptyGrid )
    {
        // wellsGraph not needed
        return;
    }
    wellsGraph_.resize(grid.numCells());
    const auto& cpgdim = grid.logicalCartesianSize();
    // create compressed lookup from cartesian.
    std::vector<int> cartesian_to_compressed(cpgdim[0]*cpgdim[1]*cpgdim[2], -1);

    for( int i=0; i < grid.numCells(); ++i )
    {
        cartesian_to_compressed[grid.globalCell()[i]] = i;
    }
    well_indices_.init(*wells, cpgdim, cartesian_to_compressed);
    std::vector<int>().swap(cartesian_to_compressed); // free memory.
    addCompletionSetToGraph();
}

void setCpGridZoltanGraphFunctions(Zoltan_Struct *zz, const Dune::CpGrid& grid,
                                   bool pretendNull)
{
    Dune::CpGrid *gridPointer = const_cast<Dune::CpGrid*>(&grid);
    if ( pretendNull )
    {
        Zoltan_Set_Num_Obj_Fn(zz, getNullNumCells, gridPointer);
        Zoltan_Set_Obj_List_Fn(zz, getNullVertexList, gridPointer);
        Zoltan_Set_Num_Edges_Multi_Fn(zz, getNullNumEdgesList, gridPointer);
        Zoltan_Set_Edge_List_Multi_Fn(zz, getNullEdgeList, gridPointer);
    }
    else
    {
        Zoltan_Set_Num_Obj_Fn(zz, getCpGridNumCells, gridPointer);
        Zoltan_Set_Obj_List_Fn(zz, getCpGridVertexList, gridPointer);
        Zoltan_Set_Num_Edges_Multi_Fn(zz, getCpGridNumEdgesList, gridPointer);
        Zoltan_Set_Edge_List_Multi_Fn(zz, getCpGridEdgeList, gridPointer);
    }
}

void setCpGridZoltanGraphFunctions(Zoltan_Struct *zz,
                                   const CombinedGridWellGraph& graph,
                                   bool pretendNull)
{
    Dune::CpGrid *gridPointer = const_cast<Dune::CpGrid*>(&graph.getGrid());
    if ( pretendNull )
    {
        Zoltan_Set_Num_Obj_Fn(zz, getNullNumCells, gridPointer);
        Zoltan_Set_Obj_List_Fn(zz, getNullVertexList, gridPointer);
        Zoltan_Set_Num_Edges_Multi_Fn(zz, getNullNumEdgesList, gridPointer);
        Zoltan_Set_Edge_List_Multi_Fn(zz, getNullEdgeList, gridPointer);
    }
    else
    {
        CombinedGridWellGraph* graphPointer = const_cast<CombinedGridWellGraph*>(&graph);
        Zoltan_Set_Num_Obj_Fn(zz, getCpGridNumCells, gridPointer);
        Zoltan_Set_Obj_List_Fn(zz, getCpGridVertexList, gridPointer);
        Zoltan_Set_Num_Edges_Multi_Fn(zz, getCpGridWellsNumEdgesList, graphPointer);
        Zoltan_Set_Edge_List_Multi_Fn(zz, getCpGridWellsEdgeList, graphPointer);
    }
}
} // end namespace cpgrid
} // end namespace Dune
#endif // HAVE_ZOLTAN
