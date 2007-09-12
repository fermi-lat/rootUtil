// -*- Mode: c++ -*-
#ifndef DataHandleBase_h
#define DataHandleBase_h
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: DataHandleBase.h,v 1.1 2007/08/08 13:50:02 heather Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*
*
*/

// Root headers
#include <Rtypes.h>

// forward declares
class TBranch;
class TTree;

//
// DataHandleBase is an abstract base class for associating simple data types with 
// root trees.
//
// For example, the sub-classes DataHandle<T> manages a datum of type T, and gets it on and off
// TTrees while allowing us to treat the Handle as thought it were of type T itself.
//
// To declare new branches on a TTree
//   makeBranch(TTree& tree, const char* name, Int_t bufsize = 32000) const;
//
// To use a BranchGroup to attach DataHandles to branches on an existing TTree
//   attachToTree(TTree& tree, const char* prefix = 0);
//


class DataHandleBase {
public:

  // C'tors and D'tor
  // Default c'tor
  DataHandleBase(){;}
  // Copy c'tor
  DataHandleBase(const DataHandleBase& other){;}
  // D'tor
  virtual ~DataHandleBase();

  // Methods & functions
  // Tree Interactions
  virtual TBranch* makeBranch(TTree& tree, const char* name, Int_t bufsize = 32000) const;
  virtual Bool_t attachToTree(TTree& tree, const char* name) const;

  // Type of branch to make,  Returns a Char_t that root uses in TTree::Branch()
  virtual Char_t branchType() const = 0;

protected:

  // The address of the memory where the data is located.  This is passes to Root.
  virtual void* void_ptr() const = 0;

  ClassDef(DataHandleBase,0) // Base class for associating simple data types with TTrees
};


#endif
