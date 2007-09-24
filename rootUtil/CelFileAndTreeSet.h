
#ifndef CelFileAndTreeSet_h
#define CelFileAndTreeSet_h

/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CelFileAndTreeSet.h,v 1.1 2007/09/21 13:58:58 chamont Exp $
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
class TTree ;
class TFile ;
class TObjArray ;
class TArrayL64 ;

#include <map>

//
// CelFileAndTreeSet stores information needed to point to a part of an event
// that is located in another TTree.
//
// CelFileAndTreeSet keep track of the TTrees where the event components live.
//
// Lists of files and tree names are stored persistently on a TTree by 
// CelFileAndTreeSet.  These are:
//
//  _fileNames (goes to branch XXX_FileNames) is the vector of file names
//  _treeNames (goes to branch XXX_TreeNames) is the vector of tree names
// 
// At any given time a CelFileAndTreeSet may be keeping track of any number of TTree
//   
// Access to the TTree is by key
//  TTree* getTree(UShort_t key) const ;
//
// The keys are assigned when TTree are first associated with the CelFileAndTreeSet
// 

class CelFileAndTreeSet  : public BranchGroup
 {

  public :

    // construction
    CelFileAndTreeSet() ; // Needed for ROOT
    CelFileAndTreeSet( const TString & componentName ) ;
    virtual ~CelFileAndTreeSet() ;

  // Methods and functions
  // Reset this object and clear caches
  void reset();
  // Add a tree to the list refered to by this object
  UShort_t addTree(TTree& tree);
  // Get the persistent KEY for a given tree
  UShort_t getKey(TTree* tree) const;
  // Get a given tree using persistent KEY
  TTree* getTree(UShort_t key) const ;
  // Get the Event offset using persistent KEY
  Long64_t getOffset(UShort_t key) const ;

    // Override the methods in BranchGroup.  
    virtual Int_t makeBranches( TTree & celTree, const char * prefix = 0, Int_t bufsize = 32000) const ;
    virtual Int_t attachToTree( TTree & celTree, const char * prefix = 0) ;

    // Access
    inline UShort_t size() const { return _setSize ; }
    inline Long64_t entries() const { return _treesSize ; }
    inline const TString & componentName() const { return _componentName ; }

  // Printing
  // Print the list of trees
  void show(const char* options = "tf") const;
  // Print information about a single tree
  void printTreeInfo(UShort_t key, const char* options = "tf") const;

  protected :

    // Utility function to actually go and get a tree out of a file
    TTree * fetchTree( UShort_t key ) const ;

  private :

    //disable copying and assignment
    CelFileAndTreeSet( const CelFileAndTreeSet & ) ;
    CelFileAndTreeSet & operator=( const CelFileAndTreeSet & ) ;

    // data
    TString _componentName ;

    // data on branches
    DataHandle<UShort_t>  _setSize ;     //! Number of trees stored by this CelFileAndTreeSet
    TObjArray*            _fileNames ;   //! Names of files where the TTree live
    TObjArray*            _treeNames ;   //! Names of trees stored by this CelFileAndTreeSet
    DataHandle<Long64_t>  _treesSize ;   //! Number of entries on the trees stored by this CelFileAndTreeSet
    TArrayL64*            _treeOffsets ; //! Number of entries in TTrees

    // Cache for fast lookup
    mutable std::map<UShort_t,TTree*> _cache ;  //! Maps key -> tree
    mutable std::map<TTree*,UShort_t> _lookup ; //! Reverse map, for making sure we don't screw up

    ClassDef(CelFileAndTreeSet,0) // For handling referenece to trees
 } ;


#endif

