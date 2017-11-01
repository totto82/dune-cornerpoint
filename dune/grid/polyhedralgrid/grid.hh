// -*- mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=2 sw=2 sts=2:
#ifndef DUNE_POLYHEDRALGRID_GRID_HH
#define DUNE_POLYHEDRALGRID_GRID_HH

#include <set>
#include <vector>

// Warning suppression for Dune includes.
#include <opm/grid/utility/platform_dependent/disable_warnings.h>

//- dune-common includes
#include <dune/common/version.hh>
#include <dune/common/array.hh>

//- dune-grid includes
#include <dune/grid/common/grid.hh>
#if DUNE_VERSION_NEWER(DUNE_COMMON,2,3)
#include <dune/common/parallel/collectivecommunication.hh>
#else
#include <dune/common/collectivecommunication.hh>
#endif

//- polyhedralgrid includes
#include <dune/grid/polyhedralgrid/capabilities.hh>
#include <dune/grid/polyhedralgrid/declaration.hh>
#include <dune/grid/polyhedralgrid/entity.hh>
#include <dune/grid/polyhedralgrid/entityseed.hh>
#include <dune/grid/polyhedralgrid/geometry.hh>
#include <dune/grid/polyhedralgrid/gridview.hh>
#include <dune/grid/polyhedralgrid/idset.hh>

// Re-enable warnings.
#include <opm/grid/utility/platform_dependent/reenable_warnings.h>

#include <opm/grid/utility/ErrorMacros.hpp>
#include <opm/core/grid.h>
#include <opm/core/grid/cpgpreprocess/preprocess.h>
#include <opm/core/grid/GridManager.hpp>
#include <opm/core/grid/cornerpoint_grid.h>
#include <opm/core/grid/MinpvProcessor.hpp>

namespace Dune
{


  // PolyhedralGridFamily
  // ------------

  template< int dim, int dimworld >
  struct PolyhedralGridFamily
  {
    struct Traits
    {
      typedef PolyhedralGrid< dim, dimworld > Grid;

      typedef double ctype;

      // type of data passed to entities, intersections, and iterators
      // for PolyhedralGrid this is just an empty place holder
      typedef const Grid* ExtraData;

      typedef int Index ;

      static const int dimension      = dim;
      static const int dimensionworld = dimworld;

      typedef Dune::FieldVector< ctype, dimensionworld > GlobalCoordinate ;

      typedef PolyhedralGridIntersection< const Grid > LeafIntersectionImpl;
      typedef PolyhedralGridIntersection< const Grid > LevelIntersectionImpl;
      typedef PolyhedralGridIntersectionIterator< const Grid > LeafIntersectionIteratorImpl;
      typedef PolyhedralGridIntersectionIterator< const Grid > LevelIntersectionIteratorImpl;

#if DUNE_VERSION_NEWER(DUNE_GRID,2,3)
      typedef Dune::Intersection< const Grid, LeafIntersectionImpl > LeafIntersection;
      typedef Dune::Intersection< const Grid, LevelIntersectionImpl > LevelIntersection;

      typedef Dune::IntersectionIterator< const Grid, LeafIntersectionIteratorImpl, LeafIntersectionImpl > LeafIntersectionIterator;
      typedef Dune::IntersectionIterator< const Grid, LevelIntersectionIteratorImpl, LevelIntersectionImpl > LevelIntersectionIterator;
#else
      typedef Dune::Intersection< const Grid, PolyhedralGridIntersection > LeafIntersection;
      typedef Dune::Intersection< const Grid, PolyhedralGridIntersection > LevelIntersection;

      typedef Dune::IntersectionIterator< const Grid, PolyhedralGridIntersectionIterator, PolyhedralGridIntersection > LeafIntersectionIterator;
      typedef Dune::IntersectionIterator< const Grid, PolyhedralGridIntersectionIterator, PolyhedralGridIntersection > LevelIntersectionIterator;
#endif

      typedef PolyhedralGridIterator< 0, const Grid, All_Partition > HierarchicIteratorImpl;
      typedef Dune::EntityIterator< 0, const Grid, HierarchicIteratorImpl > HierarchicIterator;

      template< int codim >
      struct Codim
      {
        typedef PolyhedralGridGeometry<dimension-codim, dimensionworld, const Grid> GeometryImpl;
        typedef Dune::Geometry< dimension-codim, dimensionworld, const Grid, PolyhedralGridGeometry > Geometry;

        typedef PolyhedralGridLocalGeometry< dimension-codim, dimensionworld, const Grid> LocalGeometryImpl;
        typedef Dune::Geometry< dimension-codim, dimension, const Grid, PolyhedralGridLocalGeometry > LocalGeometry;

        typedef PolyhedralGridEntity< codim, dimension, const Grid > EntityImpl;
        typedef Dune::Entity< codim, dimension, const Grid, PolyhedralGridEntity > Entity;

#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
        typedef EntityImpl EntityPointerImpl;
        typedef Entity     EntityPointer;
#else
        typedef PolyhedralGridEntityPointer< codim, const Grid > EntityPointerImpl;
        typedef Dune::EntityPointer< const Grid, EntityPointerImpl > EntityPointer;
#endif

        //typedef Dune::EntitySeed< const Grid, PolyhedralGridEntitySeed< codim, const Grid > > EntitySeed;
        typedef PolyhedralGridEntitySeed< codim, const Grid > EntitySeed;

        template< PartitionIteratorType pitype >
        struct Partition
        {
          typedef PolyhedralGridIterator< codim, const Grid, pitype > LeafIteratorImpl;
          typedef Dune::EntityIterator< codim, const Grid, LeafIteratorImpl > LeafIterator;

          typedef LeafIterator LevelIterator;
        };

        typedef typename Partition< All_Partition >::LeafIterator LeafIterator;
        typedef typename Partition< All_Partition >::LevelIterator LevelIterator;
      };

      typedef PolyhedralGridIndexSet< dim, dimworld > LeafIndexSet;
      typedef PolyhedralGridIndexSet< dim, dimworld > LevelIndexSet;

      typedef PolyhedralGridIdSet< dim, dimworld > GlobalIdSet;
      typedef GlobalIdSet  LocalIdSet;

      typedef Dune::CollectiveCommunication< Grid > CollectiveCommunication;

      template< PartitionIteratorType pitype >
      struct Partition
      {
        typedef Dune::GridView< PolyhedralGridViewTraits< dim, dimworld, pitype > > LeafGridView;
        typedef Dune::GridView< PolyhedralGridViewTraits< dim, dimworld, pitype > > LevelGridView;
      };

      typedef typename Partition<All_Partition>::LevelGridView LevelGridView;
      typedef typename Partition<All_Partition>::LeafGridView LeafGridView;

    };
  };



  // PolyhedralGrid
  // ------

  /** \class PolyhedralGrid
   *  \brief identical grid wrapper
   *  \ingroup PolyhedralGrid
   *
   *  \tparam  HostGrid   DUNE grid to be wrapped (called host grid)
   *
   *  \nosubgrouping
   */
  template < int dim, int dimworld >
  class PolyhedralGrid
  /** \cond */
  : public GridDefaultImplementation
      < dim, dimworld, double, PolyhedralGridFamily< dim, dimworld > >
  /** \endcond */
  {
    typedef PolyhedralGrid< dim, dimworld > Grid;

    typedef GridDefaultImplementation
      < dim, dimworld, double, PolyhedralGridFamily< dim, dimworld > > Base;

    typedef UnstructuredGrid  UnstructuredGridType;

    struct UnstructuredGridDeleter
    {
      inline void operator () ( UnstructuredGridType* grdPtr )
      {
        destroy_grid( grdPtr );
      }
    };
  public:
    /** \cond */
    typedef PolyhedralGridFamily< dim, dimworld > GridFamily;
    /** \endcond */

    /** \name Traits
     *  \{ */

    //! type of the grid traits
    typedef typename GridFamily::Traits Traits;

    /** \brief traits structure containing types for a codimension
     *
     *  \tparam codim  codimension
     *
     *  \nosubgrouping
     */
    template< int codim >
    struct Codim;

    /** \} */

    /** \name Iterator Types
     *  \{ */

    //! iterator over the grid hierarchy
    typedef typename Traits::HierarchicIterator HierarchicIterator;
    //! iterator over intersections with other entities on the leaf level
    typedef typename Traits::LeafIntersectionIterator LeafIntersectionIterator;
    //! iterator over intersections with other entities on the same level
    typedef typename Traits::LevelIntersectionIterator LevelIntersectionIterator;

    /** \} */

    /** \name Grid View Types
     *  \{ */

    /** \brief Types for GridView */
    template< PartitionIteratorType pitype >
    struct Partition
    {
      typedef typename GridFamily::Traits::template Partition< pitype >::LevelGridView LevelGridView;
      typedef typename GridFamily::Traits::template Partition< pitype >::LeafGridView LeafGridView;
    };

    /** \brief View types for All_Partition */
    typedef typename Partition< All_Partition >::LevelGridView LevelGridView;
    typedef typename Partition< All_Partition >::LeafGridView LeafGridView;

    /** \} */

    /** \name Index and Id Set Types
     *  \{ */

    /** \brief type of leaf index set
     *
     *  The index set assigns consecutive indices to the entities of the
     *  leaf grid. The indices are of integral type and can be used to access
     *  arrays.
     *
     *  The leaf index set is a model of Dune::IndexSet.
     */
    typedef typename Traits::LeafIndexSet LeafIndexSet;

    /** \brief type of level index set
     *
     *  The index set assigns consecutive indices to the entities of a grid
     *  level. The indices are of integral type and can be used to access
     *  arrays.
     *
     *  The level index set is a model of Dune::IndexSet.
     */
    typedef typename Traits::LevelIndexSet LevelIndexSet;

    /** \brief type of global id set
     *
     *  The id set assigns a unique identifier to each entity within the
     *  grid. This identifier is unique over all processes sharing this grid.
     *
     *  \note Id's are neither consecutive nor necessarily of an integral
     *        type.
     *
     *  The global id set is a model of Dune::IdSet.
     */
    typedef typename Traits::GlobalIdSet GlobalIdSet;

    /** \brief type of local id set
     *
     *  The id set assigns a unique identifier to each entity within the
     *  grid. This identifier needs only to be unique over this process.
     *
     *  Though the local id set may be identical to the global id set, it is
     *  often implemented more efficiently.
     *
     *  \note Ids are neither consecutive nor necessarily of an integral
     *        type.
     *  \note Local ids need not be compatible with global ids. Also, no
     *        mapping from local ids to global ones needs to exist.
     *
     *  The global id set is a model of Dune::IdSet.
     */
    typedef typename Traits::LocalIdSet LocalIdSet;

    /** \} */

    /** \name Miscellaneous Types
     * \{ */

    //! type of vector coordinates (e.g., double)
    typedef typename Traits::ctype ctype;

    //! communicator with all other processes having some part of the grid
    typedef typename Traits::CollectiveCommunication CollectiveCommunication;

    typedef typename Traits :: GlobalCoordinate GlobalCoordinate;

    /** \} */

    /** \name Construction and Destruction
     *  \{ */

#if HAVE_OPM_PARSER
    /** \brief constructor
     *
     *  \param[in]  deck         Opm Eclipse deck
     *  \param[in]  poreVolumes  vector with pore volumes (default = empty)
     */
    explicit PolyhedralGrid ( const Opm::Deck& deck,
                              const  std::vector<double>& poreVolumes = std::vector<double> ())
    : gridPtr_( createGrid( deck, poreVolumes ) ),
      grid_( *gridPtr_ ),
      comm_( *this ),
      leafIndexSet_( *this ),
      globalIdSet_( *this ),
      localIdSet_( *this )
    {
      init();
    }
#endif

    /** \brief constructor
     *
     *  The references to ug are stored in the grid.
     *  Therefore, they must remain valid until the grid is destroyed.
     *
     *  \param[in]  ug    UnstructuredGrid reference
     */
    explicit PolyhedralGrid ( const UnstructuredGridType& grid )
    : gridPtr_(),
      grid_( grid ),
      comm_( *this ),
      leafIndexSet_( *this ),
      globalIdSet_( *this ),
      localIdSet_( *this )
    {
      init();
    }

    /** \} */

    /** \name Casting operators
     *  \{ */
    operator const UnstructuredGridType& () const { return grid_; }

    /** \} */

    /** \name Size Methods
     *  \{ */

    /** \brief obtain maximal grid level
     *
     *  Grid levels are numbered 0, ..., L, where L is the value returned by
     *  this method.
     *
     *  \returns maximal grid level
     */
    int maxLevel () const
    {
      return 1;
    }

    /** \brief obtain number of entites on a level
     *
     *  \param[in]  level  level to consider
     *  \param[in]  codim  codimension to consider
     *
     *  \returns number of entities of codimension \em codim on grid level
     *           \em level.
     */
    int size ( int /* level */, int codim ) const
    {
      return size( codim );
    }

    /** \brief obtain number of leaf entities
     *
     *  \param[in]  codim  codimension to consider
     *
     *  \returns number of leaf entities of codimension \em codim
     */
    int size ( int codim ) const
    {
      if( codim == 0 )
      {
        return grid_.number_of_cells;
      }
      else if ( codim == 1 )
      {
        return grid_.number_of_faces;
      }
      else if ( codim == dim )
      {
        return grid_.number_of_nodes;
      }
      else
      {
        std::cerr << "Warning: codimension " << codim << " not available in PolyhedralGrid" << std::endl;
        return 0;
      }
    }

    /** \brief obtain number of entites on a level
     *
     *  \param[in]  level  level to consider
     *  \param[in]  type   geometry type to consider
     *
     *  \returns number of entities with a geometry of type \em type on grid
     *           level \em level.
     */
    int size ( int /* level */, GeometryType type ) const
    {
      return size( dim - type.dim() );
    }

    /** \brief returns the number of boundary segments within the macro grid
     *
     *  \returns number of boundary segments within the macro grid
     */
    int size ( GeometryType type ) const
    {
      return size( dim - type.dim() );
    }

    /** \brief obtain number of leaf entities
     *
     *  \param[in]  type   geometry type to consider
     *
     *  \returns number of leaf entities with a geometry of type \em type
     */
    size_t numBoundarySegments () const
    {
      return 0;
    }
    /** \} */

    template< int codim >
    typename Codim< codim >::LeafIterator leafbegin () const
    {
      return leafbegin< codim, All_Partition >();
    }

    template< int codim >
    typename Codim< codim >::LeafIterator leafend () const
    {
      return leafend< codim, All_Partition >();
    }

    template< int codim, PartitionIteratorType pitype >
    typename Codim< codim >::template Partition< pitype >::LeafIterator
    leafbegin () const
    {
      typedef typename Traits::template Codim< codim >::template Partition< pitype >::LeafIteratorImpl Impl;
      return Impl( extraData(), true );
    }

    template< int codim, PartitionIteratorType pitype >
    typename Codim< codim >::template Partition< pitype >::LeafIterator
    leafend () const
    {
      typedef typename Traits::template Codim< codim >::template Partition< pitype >::LeafIteratorImpl Impl;
      return Impl( extraData(), false );
    }

    template< int codim >
    typename Codim< codim >::LevelIterator lbegin ( const int /* level */ ) const
    {
      return leafbegin< codim, All_Partition >();
    }

    template< int codim >
    typename Codim< codim >::LevelIterator lend ( const int /* level */ ) const
    {
      return leafend< codim, All_Partition >();
    }

    template< int codim, PartitionIteratorType pitype >
    typename Codim< codim >::template Partition< pitype >::LevelIterator
    lbegin ( const int /* level */ ) const
    {
      return leafbegin< codim, pitype > ();
    }

    template< int codim, PartitionIteratorType pitype >
    typename Codim< codim >::template Partition< pitype >::LevelIterator
    lend ( const int /* level */ ) const
    {
      return leafend< codim, pitype > ();
    }

    const GlobalIdSet &globalIdSet () const
    {
      return globalIdSet_;
    }

    const LocalIdSet &localIdSet () const
    {
      return localIdSet_;
    }

    const LevelIndexSet &levelIndexSet ( int /* level */ ) const
    {
      return leafIndexSet();
    }

    const LeafIndexSet &leafIndexSet () const
    {
      return leafIndexSet_;
    }

    void globalRefine ( int /* refCount */ )
    {
    }

    bool mark ( int /* refCount */, const typename Codim< 0 >::Entity& /* entity */ )
    {
      return false;
    }

    int getMark ( const typename Codim< 0 >::Entity& /* entity */) const
    {
      return false;
    }

    /** \brief  @copydoc Dune::Grid::preAdapt() */
    bool preAdapt ()
    {
      return false;
    }

    /** \brief  @copydoc Dune::Grid::adapt() */
    bool adapt ()
    {
      return false ;
    }

    /** \brief  @copydoc Dune::Grid::adapt()
        \param handle handler for restriction and prolongation operations
        which is a Model of the AdaptDataHandleInterface class.
    */
    template< class DataHandle >
    bool adapt ( DataHandle & )
    {
      return false;
    }

    /** \brief  @copydoc Dune::Grid::postAdapt() */
    void postAdapt ()
    {
    }

    /** \name Parallel Data Distribution and Communication Methods
     *  \{ */

    /** \brief obtain size of overlap region for the leaf grid
     *
     *  \param[in]  codim  codimension for with the information is desired
     */
    int overlapSize ( int /* codim */) const
    {
      return 0;
    }

    /** \brief obtain size of ghost region for the leaf grid
     *
     *  \param[in]  codim  codimension for with the information is desired
     */
    int ghostSize( int codim ) const
    {
      return (codim == 0 ) ? 1 : 0;
    }

    /** \brief obtain size of overlap region for a grid level
     *
     *  \param[in]  level  grid level (0, ..., maxLevel())
     *  \param[in]  codim  codimension (0, ..., dimension)
     */
    int overlapSize ( int /* level */, int /* codim */ ) const
    {
      return 0;
    }

    /** \brief obtain size of ghost region for a grid level
     *
     *  \param[in]  level  grid level (0, ..., maxLevel())
     *  \param[in]  codim  codimension (0, ..., dimension)
     */
    int ghostSize ( int /* level */, int codim ) const
    {
      return ghostSize( codim );
    }

    /** \brief communicate information on a grid level
     *
     *  \param      dataHandle  communication data handle (user defined)
     *  \param[in]  interface   communication interface (one of
     *                          InteriorBorder_InteriorBorder_Interface,
     *                          InteriorBorder_All_Interface,
     *                          Overlap_OverlapFront_Interface,
     *                          Overlap_All_Interface,
     *                          All_All_Interface)
     *  \param[in]  direction   communication direction (one of
     *                          ForwardCommunication or BackwardCommunication)
     *  \param[in]  level       grid level to communicate
     */
    template< class DataHandle, class Data >
    void communicate ( CommDataHandleIF< DataHandle, Data >& /* dataHandle */,
                       InterfaceType /* interface */,
                       CommunicationDirection /* direction */,
                       int /* level */ ) const
    {
       //levelGridView( level ).communicate( dataHandle, interface, direction );
    }

    /** \brief communicate information on leaf entities
     *
     *  \param      dataHandle  communication data handle (user defined)
     *  \param[in]  interface   communication interface (one of
     *                          InteriorBorder_InteriorBorder_Interface,
     *                          InteriorBorder_All_Interface,
     *                          Overlap_OverlapFront_Interface,
     *                          Overlap_All_Interface,
     *                          All_All_Interface)
     *  \param[in]  direction   communication direction (one of
     *                          ForwardCommunication, BackwardCommunication)
     */
    template< class DataHandle, class Data >
    void communicate ( CommDataHandleIF< DataHandle, Data >& /* dataHandle */,
                       InterfaceType /* interface */,
                       CommunicationDirection /* direction */ ) const
    {
      //leafGridView().communicate( dataHandle, interface, direction );
    }

    /** \brief obtain CollectiveCommunication object
     *
     *  The CollectiveCommunication object should be used to globally
     *  communicate information between all processes sharing this grid.
     *
     *  \note The CollectiveCommunication object returned is identical to the
     *        one returned by the host grid.
     */
    const CollectiveCommunication &comm () const
    {
      return comm_;
    }

    // data handle interface different between geo and interface

    /** \brief rebalance the load each process has to handle
     *
     *  A parallel grid is redistributed such that each process has about
     *  the same load (e.g., the same number of leaf entites).
     *
     *  \note DUNE does not specify, how the load is measured.
     *
     *  \returns \b true, if the grid has changed.
     */
    bool loadBalance ()
    {
      return false ;
    }

    /** \brief rebalance the load each process has to handle
     *
     *  A parallel grid is redistributed such that each process has about
     *  the same load (e.g., the same number of leaf entites).
     *
     *  The data handle is used to communicate the data associated with
     *  entities that move from one process to another.
     *
     *  \note DUNE does not specify, how the load is measured.
     *
     *  \param  datahandle  communication data handle (user defined)
     *
     *  \returns \b true, if the grid has changed.
     */

    template< class DataHandle, class Data >
    bool loadBalance ( CommDataHandleIF< DataHandle, Data >& /* datahandle */ )
    {
      return false;
    }

    /** \brief rebalance the load each process has to handle
     *
     *  A parallel grid is redistributed such that each process has about
     *  the same load (e.g., the same number of leaf entites).
     *
     *  The data handle is used to communicate the data associated with
     *  entities that move from one process to another.
     *
     *  \note DUNE does not specify, how the load is measured.
     *
     *  \param  dataHandle  data handle following the ALUGrid interface
     *
     *  \returns \b true, if the grid has changed.
     */
    template< class DofManager >
    bool loadBalance ( DofManager& /* dofManager */ )
    {
      return false;
    }

    /** \brief View for a grid level */
    template< PartitionIteratorType pitype >
    typename Partition< pitype >::LevelGridView levelGridView ( int /* level */ ) const
    {
      typedef typename Partition< pitype >::LevelGridView View;
      typedef typename View::GridViewImp ViewImp;
      return View( ViewImp( *this ) );
    }

    /** \brief View for the leaf grid */
    template< PartitionIteratorType pitype >
    typename Partition< pitype >::LeafGridView leafGridView () const
    {
      typedef typename Traits::template Partition< pitype >::LeafGridView View;
      typedef typename View::GridViewImp ViewImp;
      return View( ViewImp( *this ) );
    }

    /** \brief View for a grid level for All_Partition */
    LevelGridView levelGridView ( int /* level */ ) const
    {
      typedef typename LevelGridView::GridViewImp ViewImp;
      return LevelGridView( ViewImp( *this ) );
    }

    /** \brief View for the leaf grid for All_Partition */
    LeafGridView leafGridView () const
    {
      typedef typename LeafGridView::GridViewImp ViewImp;
      return LeafGridView( ViewImp( *this ) );
    }

    /** \brief obtain EntityPointer from EntitySeed. */
    template< class EntitySeed >
    typename Traits::template Codim< EntitySeed::codimension >::EntityPointer
    entityPointer ( const EntitySeed &seed ) const
    {
      typedef typename Traits::template Codim< EntitySeed::codimension >::EntityPointer     EntityPointer;
      typedef typename Traits::template Codim< EntitySeed::codimension >::EntityPointerImpl EntityPointerImpl;
      typedef typename Traits::template Codim< EntitySeed::codimension >::EntityImpl        EntityImpl;
      return EntityPointer( EntityPointerImpl( EntityImpl( extraData(), seed ) ) );
    }

    /** \brief obtain EntityPointer from EntitySeed. */
    template< class EntitySeed >
    typename Traits::template Codim< EntitySeed::codimension >::Entity
    entity ( const EntitySeed &seed ) const
    {
      typedef typename Traits::template Codim< EntitySeed::codimension >::EntityImpl        EntityImpl;
      return EntityImpl( extraData(), seed );
    }

    /** \} */

    /** \name Miscellaneous Methods
     *  \{ */

    /** \brief update grid caches
     *
     *  This method has to be called whenever the underlying host grid changes.
     *
     *  \note If you adapt the host grid through this geometry grid's
     *        adaptation or load balancing methods, update is automatically
     *        called.
     */
    void update ()
    {
    }

    /** \} */

    const std::array<int, 3>& logicalCartesianSize() const
    {
      return cartDims_;
    }

    const int* globalCell() const
    {
      assert( grid_.global_cell != 0 );
      return grid_.global_cell;
    }

    void getIJK(const int c, std::array<int,3>& ijk) const
    {
      int gc = globalCell()[c];
      ijk[0] = gc % logicalCartesianSize()[0];  gc /= logicalCartesianSize()[0];
      ijk[1] = gc % logicalCartesianSize()[1];
      ijk[2] = gc / logicalCartesianSize()[1];
    }

  protected:

#if HAVE_OPM_PARSER
    UnstructuredGridType* createGrid( const Opm::Deck& deck, const std::vector< double >& poreVolumes ) const
    {
        const int* rawactnum = deck.hasKeyword("ACTNUM")
          ? deck.getKeyword("ACTNUM").getIntData().data()
          : nullptr;
        const auto eclipseGrid = std::make_shared<Opm::EclipseGrid>(deck, rawactnum);

        struct grdecl g;
        std::vector<int> actnum;
        std::vector<double> coord;
        std::vector<double> zcorn;
        std::vector<double> mapaxes;

        g.dims[0] = eclipseGrid->getNX();
        g.dims[1] = eclipseGrid->getNY();
        g.dims[2] = eclipseGrid->getNZ();

        eclipseGrid->exportMAPAXES( mapaxes );
        eclipseGrid->exportCOORD( coord );
        eclipseGrid->exportZCORN( zcorn );
        eclipseGrid->exportACTNUM( actnum );

        g.coord = coord.data();
        g.zcorn = zcorn.data();
        g.actnum = actnum.data();
        g.mapaxes = mapaxes.data();

        if (!poreVolumes.empty() && (eclipseGrid->getMinpvMode() != Opm::MinpvMode::ModeEnum::Inactive)) {
            Opm::MinpvProcessor mp(g.dims[0], g.dims[1], g.dims[2]);
            const double minpv_value  = eclipseGrid->getMinpvValue();
            // Currently the pinchProcessor is not used and only opmfil is supported
            //bool opmfil = eclipseGrid->getMinpvMode() == Opm::MinpvMode::OpmFIL;
            bool opmfil = true;
            mp.process(poreVolumes, minpv_value, actnum, opmfil, zcorn.data());
        }

        const double z_tolerance = eclipseGrid->isPinchActive() ?
            eclipseGrid->getPinchThresholdThickness() : 0.0;
        UnstructuredGridType* cgrid = create_grid_cornerpoint(&g, z_tolerance);
        if (!cgrid) {
            OPM_THROW(std::runtime_error, "Failed to construct grid.");
        }
        return cgrid;
    }
#endif

  public:
    using Base::getRealImplementation;

    typedef typename Traits :: ExtraData ExtraData;
    ExtraData extraData () const  { return this; }

    template <class EntitySeed>
    int corners( const EntitySeed& seed ) const
    {
      const int codim = EntitySeed :: codimension;
      const int index = seed.index();
      switch (codim)
      {
        case 0:
          {
            return cellVertices_[ index ].size();
          }
        case 1:
          {
            return grid_.face_nodepos[ index+1 ] - grid_.face_nodepos[ index ];
          }
        case dim:
          {
            return 1;
          }
      }
      return 0;
    }

    template <class EntitySeed>
    GlobalCoordinate
    corner( const EntitySeed& seed, const int i ) const
    {
      const int codim = EntitySeed :: codimension;
      switch (codim)
      {
        case 0:
          {
            const int coordIndex = GlobalCoordinate :: dimension * cellVertices_[ seed.index() ][ i ];
            return copyToGlobalCoordinate( grid_.node_coordinates + coordIndex );
          }
        case 1:
          {
            const int faceVertex = grid_.face_nodes[grid_.face_nodepos[seed.index()] + i];
            return copyToGlobalCoordinate( grid_.node_coordinates + GlobalCoordinate :: dimension * faceVertex );
          }
        case dim:
          {
            const int coordIndex = GlobalCoordinate :: dimension * seed.index();
            return copyToGlobalCoordinate( grid_.node_coordinates + coordIndex );
          }
      }
      return GlobalCoordinate( 0 );
    }

    template <class EntitySeed>
    int subEntities( const EntitySeed& seed, const int codim ) const
    {
      const int index = seed.index();
      switch (codim)
      {
        case 0:
          return 1;
        case 1:
          return grid_.cell_facepos[ index+1 ] - grid_.cell_facepos[ index ];
        case dim:
          return cellVertices_[ index ].size();
      }
      return 0;
    }

    template <int codim, class EntitySeedArg >
    typename Codim<codim>::EntitySeed
    subEntitySeed( const EntitySeedArg& baseSeed, const int i ) const
    {
      assert( codim >= EntitySeedArg::codimension );
      assert( i>= 0 && i<subEntities( baseSeed, codim ) );
      typedef typename Codim<codim>::EntitySeed  EntitySeed;

      // if codim equals entity seed codim just return same entity seed.
      if( codim == EntitySeedArg::codimension )
      {
        return EntitySeed( baseSeed.index() );
      }

      if( EntitySeedArg::codimension == 0 )
      {
        if ( codim == 1 )
        {
          return EntitySeed( grid_.cell_faces[ grid_.cell_facepos[ baseSeed.index() ] + i ] );
        }
        else if ( codim == dim )
        {
          return EntitySeed( cellVertices_[ baseSeed.index() ][ i ] );
        }
      }
      else if ( EntitySeedArg::codimension == 1 && codim == dim )
      {
        return EntitySeed( grid_.face_nodes[ grid_.face_nodepos[ baseSeed.index() + i ] ]);
      }

      DUNE_THROW(NotImplemented,"codimension not available");
      return EntitySeed();
    }

    const std::vector< GeometryType > &geomTypes ( const unsigned int codim ) const
    {
      static std::vector< GeometryType > emptyDummy;
      if (0 <= codim && codim < geomTypes_.size())
      {
        return geomTypes_[codim];
      }

      return emptyDummy;
    }

    int indexInInside( const typename Codim<0>::EntitySeed& seed, const int i ) const
    {
      return ( grid_.cell_facetag ) ? cartesianIndexInInside( seed, i ) : i;
    }

    int cartesianIndexInInside( const typename Codim<0>::EntitySeed& seed, const int i ) const
    {
      assert( i>= 0 && i<subEntities( seed, 1 ) );
      return grid_.cell_facetag[ grid_.cell_facepos[ seed.index() ] + i ] ;
    }

    typename Codim<0>::EntitySeed
    neighbor( const typename Codim<0>::EntitySeed& seed, const int i ) const
    {
      const int face = this->template subEntitySeed<1>( seed, i ).index();
      int nb = grid_.face_cells[ 2 * face ];
      if( nb == seed.index() )
      {
        nb = grid_.face_cells[ 2 * face + 1 ];
      }

      typedef typename Codim<0>::EntitySeed EntitySeed;
      return EntitySeed( nb );
    }

    int
    indexInOutside( const typename Codim<0>::EntitySeed& seed, const int i ) const
    {
      if( grid_.cell_facetag )
      {
        // if cell_facetag is present we assume pseudo Cartesian corner point case
        const int in_inside = cartesianIndexInInside( seed, i );
        return in_inside + ((in_inside % 2) ? -1 : 1);
      }
      else
      {
        typedef typename Codim<0>::EntitySeed EntitySeed;
        EntitySeed nb = neighbor( seed, i );
        const int faces = subEntities( seed, 1 );
        for( int face = 0; face<faces; ++ face )
        {
          if( neighbor( nb, face ).equals(seed) )
          {
            return indexInInside( nb, face );
          }
        }
        DUNE_THROW(InvalidStateException,"inverse intersection not found");
        return -1;
      }
    }

    template <class EntitySeed>
    GlobalCoordinate
    outerNormal( const EntitySeed& seed, const int i ) const
    {
      const int face  = this->template subEntitySeed<1>( seed, i ).index();
      const int normalIdx = face * GlobalCoordinate :: dimension ;
      GlobalCoordinate normal = copyToGlobalCoordinate( grid_.face_normals + normalIdx );
      const int nb = grid_.face_cells[ 2*face ];
      if( nb != seed.index() )
      {
        normal *= -1.0;
      }
      return normal;
    }

    template <class EntitySeed>
    GlobalCoordinate
    unitOuterNormal( const EntitySeed& seed, const int i ) const
    {
      const int face  = this->template subEntitySeed<1>( seed, i ).index();
      if( seed.index() == grid_.face_cells[ 2*face ] )
      {
        return unitOuterNormals_[ face ];
      }
      else
      {
        GlobalCoordinate normal = unitOuterNormals_[ face ];
        normal *= -1.0;
        return normal;
      }
    }

    template <class EntitySeed>
    GlobalCoordinate centroids( const EntitySeed& seed ) const
    {
      const int index = GlobalCoordinate :: dimension * seed.index();
      const int codim = EntitySeed::codimension;
      assert( index >= 0 && index < size( codim ) * GlobalCoordinate :: dimension );

      if( codim == 0 )
      {
        return copyToGlobalCoordinate( grid_.cell_centroids + index );
      }
      else if ( codim == 1 )
      {
        return copyToGlobalCoordinate( grid_.face_centroids + index );
      }
      else if( codim == dim )
      {
        return copyToGlobalCoordinate( grid_.node_coordinates + index );
      }
      else
      {
        DUNE_THROW(InvalidStateException,"codimension not implemented");
        return GlobalCoordinate( 0 );
      }
    }

    GlobalCoordinate copyToGlobalCoordinate( const double* coords ) const
    {
      GlobalCoordinate coordinate;
      for( int i=0; i<GlobalCoordinate::dimension; ++i )
      {
        coordinate[ i ] = coords[ i ];
      }
      return coordinate;
    }

    template <class EntitySeed>
    double volumes( const EntitySeed& seed ) const
    {
      const int index = seed.index();
      const int codim = EntitySeed::codimension;
      if( codim == 0 )
      {
        return grid_.cell_volumes[ index ];
      }
      else if ( codim == 1 )
      {
        return grid_.face_areas[ index ];
      }
      else if ( codim == dim )
      {
        return 1.0;
      }
      else
      {
        DUNE_THROW(InvalidStateException,"codimension not implemented");
        return 0.0;
      }
    }

    void init()
    {
      // copy Cartesian dimensions
      for( int i=0; i<3; ++i )
      {
        cartDims_[ i ] = grid_.cartdims[ i ];
      }

      // setup list of cell vertices
      const int numCells = size( 0 );
      cellVertices_.resize( numCells );

      // sort vertices such that they comply with the dune cube reference element
      if( grid_.cell_facetag )
      {
        typedef Dune::array<int, 3> KeyType;
        std::map< const KeyType, const int > vertexFaceTags;
        const int vertexFacePattern [8][3] = {
                                { 0, 2, 4 }, // vertex 0
                                { 1, 2, 4 }, // vertex 1
                                { 0, 3, 4 }, // vertex 2
                                { 1, 3, 4 }, // vertex 3
                                { 0, 2, 5 }, // vertex 4
                                { 1, 2, 5 }, // vertex 5
                                { 0, 3, 5 }, // vertex 6
                                { 1, 3, 5 }  // vertex 7
                               };

        for( int i=0; i<8; ++i )
        {
          KeyType key; key.fill( 4 ); // default is 4 which is the first z coord (for the 2d case)
          for( int j=0; j<dim; ++j )
          {
            key[ j ] = vertexFacePattern[ i ][ j ];
          }

          vertexFaceTags.insert( std::make_pair( key, i ) );
        }

        for (int c = 0; c < numCells; ++c)
        {
          typedef std::map<int,int> vertexmap_t;
          typedef typename vertexmap_t :: iterator iterator;

          std::vector< vertexmap_t > cell_pts( dim*2 );

          for (int hf=grid_.cell_facepos[ c ]; hf < grid_.cell_facepos[c+1]; ++hf)
          {
            const int f = grid_.cell_faces[ hf ];
            const int faceTag = grid_.cell_facetag[ hf ];

            for( int nodepos=grid_.face_nodepos[f]; nodepos<grid_.face_nodepos[f+1]; ++nodepos )
            {
              const int node = grid_.face_nodes[ nodepos ];
              iterator it = cell_pts[ faceTag ].find( node );
              if( it == cell_pts[ faceTag ].end() )
              {
                 cell_pts[ faceTag ].insert( std::make_pair( node, 1 ) );
              }
              else
              {
                // increase vertex reference counter
                (*it).second++;
              }
            }
          }

          typedef std::map< int, std::set<int> > vertexlist_t;
          vertexlist_t vertexList;

          for( int faceTag = 0; faceTag<6; ++faceTag )
          {
            for( iterator it = cell_pts[ faceTag ].begin(),
                 end = cell_pts[ faceTag ].end(); it != end; ++it )
            {
              // only consider vertices with one appearance
              if( (*it).second == 1 )
              {
                vertexList[ (*it).first ].insert( faceTag );
              }
            }
          }

          assert( int(vertexList.size()) == ( dim == 2 ) ? 4 : 8 );

          cellVertices_[ c ].resize( vertexList.size() );
          for( auto it = vertexList.begin(), end = vertexList.end(); it != end; ++it )
          {
            assert( (*it).second.size() == dim );
            KeyType key; key.fill( 4 ); // fill with 4 which is the first z coord

            std::copy( (*it).second.begin(), (*it).second.end(), key.begin() );
            auto vx = vertexFaceTags.find( key );
            assert( vx != vertexFaceTags.end() );
            if( vx != vertexFaceTags.end() )
            {
              if( (*vx).second >= int(cellVertices_[ c ].size()) )
                cellVertices_[ c ].resize( (*vx).second+1 );
              // store node number on correct local position
              cellVertices_[ c ][ (*vx).second ] = (*it).first ;
            }
          }
        }
        // if face_tag is available we assume that the elements follow a cube-like structure
        geomTypes_.resize(dim + 1);
        GeometryType tmp;
        for (int codim = 0; codim <= dim; ++codim)
        {
          tmp.makeCube(dim - codim);
          geomTypes_[codim].push_back(tmp);
        }
      }
      else // if ( grid_.cell_facetag )
      {
        for (int c = 0; c < numCells; ++c)
        {
          std::set<int> cell_pts;
          for (int hf=grid_.cell_facepos[ c ]; hf < grid_.cell_facepos[c+1]; ++hf)
          {
             int f = grid_.cell_faces[ hf ];
             const int* fnbeg = grid_.face_nodes + grid_.face_nodepos[f];
             const int* fnend = grid_.face_nodes + grid_.face_nodepos[f+1];
             cell_pts.insert(fnbeg, fnend);
          }

          cellVertices_[ c ].resize( cell_pts.size() );
          std::copy(cell_pts.begin(), cell_pts.end(), cellVertices_[ c ].begin() );
        }
        // if no face_tag is available we assume that no reference element can be
        // assigned to the elements
        geomTypes_.resize(dim + 1);
        GeometryType tmp;
        for (int codim = 0; codim <= dim; ++codim)
        {
          if( codim == dim )
          {
            tmp.makeCube(dim - codim);
          }
          else
          {
            tmp.makeNone(dim - codim);
          }
          geomTypes_[codim].push_back(tmp);
        }
      } // end else of ( grid_.cell_facetag )

      unitOuterNormals_.resize( grid_.number_of_faces );
      for( int face = 0; face < grid_.number_of_faces; ++face )
      {
         const int normalIdx = face * GlobalCoordinate :: dimension ;
         GlobalCoordinate normal = copyToGlobalCoordinate( grid_.face_normals + normalIdx );
         normal /= normal.two_norm();

         unitOuterNormals_[ face ] = normal;
      }
    }

  protected:
    std::unique_ptr< UnstructuredGridType, UnstructuredGridDeleter > gridPtr_;
    const UnstructuredGridType& grid_;

    CollectiveCommunication comm_;
    std::array< int, 3 > cartDims_;
    std::vector< std::vector< GeometryType > > geomTypes_;
    std::vector< std::vector< int > > cellVertices_;

    std::vector< GlobalCoordinate > unitOuterNormals_;

    mutable LeafIndexSet leafIndexSet_;
    mutable GlobalIdSet globalIdSet_;
    mutable LocalIdSet localIdSet_;

  private:
    // no copying
    PolyhedralGrid ( const PolyhedralGrid& );
  };



  // PolyhedralGrid::Codim
  // -------------

  template< int dim, int dimworld >
  template< int codim >
  struct PolyhedralGrid< dim, dimworld >::Codim
  : public Base::template Codim< codim >
  {
    /** \name Entity and Entity Pointer Types
     *  \{ */

    /** \brief type of entity
     *
     *  The entity is a model of Dune::Entity.

     */
    typedef typename Traits::template Codim< codim >::Entity Entity;

    /** \brief type of entity pointer
     *
     *  The entity pointer is a model of Dune::EntityPointer.
     */
    typedef typename Traits::template Codim< codim >::EntityPointer EntityPointer;

    /** \} */

    /** \name Geometry Types
     *  \{ */

    /** \brief type of world geometry
     *
     *  Models the geometry mapping of the entity, i.e., the mapping from the
     *  reference element into world coordinates.
     *
     *  The geometry is a model of Dune::Geometry, implemented through the
     *  generic geometries provided by dune-grid.
     */
    typedef typename Traits::template Codim< codim >::Geometry Geometry;

    /** \brief type of local geometry
     *
     *  Models the geomtry mapping into the reference element of dimension
     *  \em dimension.
     *
     *  The local geometry is a model of Dune::Geometry, implemented through
     *  the generic geometries provided by dune-grid.
     */
    typedef typename Traits::template Codim< codim >::LocalGeometry LocalGeometry;

    /** \} */

    /** \name Iterator Types
     *  \{ */

    template< PartitionIteratorType pitype >
    struct Partition
    {
      typedef typename Traits::template Codim< codim >
        ::template Partition< pitype >::LeafIterator
        LeafIterator;
      typedef typename Traits::template Codim< codim >
        ::template Partition< pitype >::LevelIterator
        LevelIterator;
    };

    /** \brief type of level iterator
     *
     *  This iterator enumerates the entites of codimension \em codim of a
     *  grid level.
     *
     *  The level iterator is a model of Dune::LevelIterator.
     */
    typedef typename Partition< All_Partition >::LeafIterator LeafIterator;

    /** \brief type of leaf iterator
     *
     *  This iterator enumerates the entites of codimension \em codim of the
     *  leaf grid.
     *
     *  The leaf iterator is a model of Dune::LeafIterator.
     */
    typedef typename Partition< All_Partition >::LevelIterator LevelIterator;

    /** \} */
  };

} // namespace Dune

#include <dune/grid/polyhedralgrid/persistentcontainer.hh>
#include <dune/grid/polyhedralgrid/cartesianindexmapper.hh>
#include <dune/grid/polyhedralgrid/gridhelpers.hh>

#endif // #ifndef DUNE_POLYHEDRALGRID_GRID_HH
