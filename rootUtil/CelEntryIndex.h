
#ifndef CelEntryIndex_h
#define CelEntryIndex_h

/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CelEntryIndex.h,v 1.2 2007/09/19 16:57:04 chamont Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*/

#include "BranchGroup.h"
#include "DataHandle.h"
class CelFileAndTreeSet ;

//
// CelEntryIndex stores information needed to point to a part of an event
// that is located in another TTree.
//
// CelEntryIndex uses CelFileAndTreeSet to associated TTrees with UShort_t keys.
//
// CelEntryIndex stores two pieces of information for each event.
//
//  _entryIndex (goes to branch XXX_EntryIndex)  is the index of the event component in the TTree
//  _treeIndex (goes to branch XXX_TreeIndex) is the index of the TTree in the associated CelFileAndTreeSet
//


class CelEntryIndex : public BranchGroup
 {
	
  public:

    // c'tors & d'tor
    CelEntryIndex() ; // Needed for ROOT
    CelEntryIndex( const std::string & componentName ) ;
    CelEntryIndex( const CelEntryIndex & ) ;
    ~CelEntryIndex() ;

    // Operators
    CelEntryIndex & operator=( const CelEntryIndex & ) ;

    // Access
    inline const std::string & componentName() const { return _componentName ; }
    inline Long64_t entryIndex() const { return _entryIndex ; }
    inline UShort_t treeIndex() const { return _treeIndex ; }

    // Methods and functions
    // set the current event
    void set( TTree & tree, CelFileAndTreeSet & handle) ;
    // read an event
    Int_t read( const CelFileAndTreeSet & handle ) ;
    // get the tree that is being read
    TTree * getTree( const CelFileAndTreeSet & handle ) const ;

    // print the info about the current event
    void printEventInfo( const char * options ) const ;  

  private :
  
    // Data
    std::string            _componentName ;  //! Name of the component this object is storing information about
    DataHandle<Long64_t>   _entryIndex ;     //! Index of the current entry in the current TTree
    DataHandle<UShort_t>   _treeIndex ;      //! Index of the current TTree in the associated CelFileAndTreeSet
  
    ClassDef(CelEntryIndex,0)               // Handles Event Level Information to point to Event Components

 } ;


#endif
