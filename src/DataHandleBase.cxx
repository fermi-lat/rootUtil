// -*- Mode: c++ -*-
#ifndef DataHandleBase_cxx
#define DataHandleBase_cxx
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: DataHandleBase.cxx,v 1.2 2007/08/08 13:50:02 heather Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*
*
*/

//
// DataHandleBase
//
// DataHandleBase is an abstract base class for associating simple data types with 
// root trees.
//
// For example, the sub-classes DataHandle<T> manages a datum of type T, and gets it on and off
// TTrees while allowing us to treat the Handle as thought it were of type T itself.
//
// To declare new branches on a TTree
//   makeBranch(TTree& tree, const char* name, Int_t bufsize = 32000) const;
//   attachToTree(TTree& tree, const char* name) const;
//
// To use a BranchGroup to attach DataHandles to branches on an existing TTree
//   attachToTree(TTree& tree, const char* prefix = 0);
//

// This Class's header
#include "rootUtil/DataHandleBase.h"

// ROOT Headers
#include <TTree.h>
#include <TBranch.h>

// c++/stl headers
#include <iostream>

ClassImp(DataHandleBase);

DataHandleBase::~DataHandleBase(){
  // D'tor is a no-op
  ;
}

TBranch* DataHandleBase::makeBranch(TTree& tree, const char* name, Int_t bufsize) const {
  // Build a branch with 'name' on 'tree'
  // returns the newly built branch
  TString leafName(name);
  leafName += '/';
  leafName += branchType();
  return tree.Branch(name,void_ptr(),leafName);
}

Bool_t DataHandleBase::attachToTree(TTree& tree, const char* name) const {
  // Attach to the branch with 'name' on 'tree'
  // returns kTRUE is successful 
  TBranch* b = tree.GetBranch(name);
  if ( 0 == b ) {
    std::cerr << "Failed to attach branch " << name
	      <<  " to tree " << tree.GetName() << std::endl;
    
    return kFALSE;
  }
  b->SetAddress(void_ptr());
  return kTRUE;
}

#endif
