
#ifndef CelEventEntry_h
#define CelEventEntry_h

/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CelEventEntry.h,v 1.4 2007/09/24 16:11:41 chamont Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*/

#include "BranchGroup.h"
#include "BgDataHandle.h"
#include <TString.h>
class CelFileAndTreeSet ;

//
// CelEventEntry stores information needed to point to a part of an event
// that is located in another TTree.
//
// CelEventEntry uses CelFileAndTreeSet to associated TTrees with UShort_t keys.
//
// CelEventEntry stores two pieces of information for each event.
//
//  _entryIndex (goes to branch XXX_EntryIndex)  is the index of the event component in the TTree
//  _treeIndex (goes to branch XXX_TreeIndex) is the index of the TTree in the associated CelFileAndTreeSet
//


class CelEventEntry : public BranchGroup
 {
	
  public:

    // c'tors & d'tor
    CelEventEntry() ; // Needed for ROOT
    CelEventEntry( const TString & componentName ) ;
    CelEventEntry( const CelEventEntry & ) ;
    CelEventEntry & operator=( const CelEventEntry & ) ;
    ~CelEventEntry() ;

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
    BgDataHandle<Long64_t> _entryIndex ; //! Index of the current entry in the current data TTree
    BgDataHandle<UShort_t> _treeIndex ;  //! Index of the current data TTree in the associated CelFileAndTreeSet
  
    ClassDef(CelEventEntry,0)

 } ;


#endif
