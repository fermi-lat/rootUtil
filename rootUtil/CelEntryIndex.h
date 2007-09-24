
#ifndef CelEntryIndex_h
#define CelEntryIndex_h

/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CelEntryIndex.h,v 1.3 2007/09/21 13:58:58 chamont Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*/

#include "BranchGroup.h"
#include "DataHandle.h"
#include <TString.h>
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
    CelEntryIndex( const TString & componentName ) ;
    CelEntryIndex( const CelEntryIndex & ) ;
    CelEntryIndex & operator=( const CelEntryIndex & ) ;
    ~CelEntryIndex() ;

    // Access
    inline const TString & componentName() const { return _componentName ; }
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
	TString _componentName ;
    DataHandle<Long64_t> _entryIndex ; //! Index of the current entry in the current data TTree
    DataHandle<UShort_t> _treeIndex ;  //! Index of the current data TTree in the associated CelFileAndTreeSet
  
    ClassDef(CelEntryIndex,0)

 } ;


#endif
