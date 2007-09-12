// -*- Mode: c++ -*-
#ifndef BranchGroup_h
#define BranchGroup_h
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: BranchGroup.h,v 1.1 2007/08/08 13:50:02 heather Exp $
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

// c++ and stl headers
#include <map>
#include <string>
#include <list>

// forward declares
class TBranch;
class TTree;
class DataHandleBase;

//
// Branch Group is a abstract class to manage a group set of values that
// get written to a TTree
//
// Any class that uses DataHandle to interact with TTree should inherit 
// from BranchGroup
//
// To use a BranchGroup to declare new branches on a TTree
//   makeBranches(TTree& tree, const char* prefix = 0, Int_t bufsize = 32000) 
//
// To use a BranchGroup to attach DataHandles to branches on an existing TTree
//   attachToTree(TTree& tree, const char* prefix = 0);
//
// In either case the branches will have names "<prefix><branchName>"
//

class BranchGroup {
public:

  // C'tors and D'tor
  // Default c'tor
  BranchGroup(){;}
  // D'tor
  virtual ~BranchGroup();


  // Methods & functions
  // Tree Interactions
  virtual Int_t makeBranches(TTree& tree, const char* prefix = 0, Int_t bufsize = 32000) const;
  virtual Int_t attachToTree(TTree& tree, const char* prefix = 0) const;

  // Access and setting
  // Get a branch by name
  DataHandleBase* getBranch(const char* name);

  // Add a branch to this group
  Bool_t addBranch(const char* name, DataHandleBase& branch);

protected:

private:
  
  //disable copying and assignment
  BranchGroup(const BranchGroup& other);
  BranchGroup& operator=(const BranchGroup& other);

  // Data
  std::list<std::string>                _branchNameList;  //! list of branch names, here for maintaining ordering
  std::map<std::string,DataHandleBase*> _branchMap;       //! map from branch names to handles, here for lookup

  ClassDef(BranchGroup,0)   // Abstract Base class for objects associating DataHandle with TTrees
};


#endif
