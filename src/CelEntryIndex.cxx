
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CelEntryIndex.cxx,v 1.3 2007/09/21 13:58:58 chamont Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*/

#include "rootUtil/CelEntryIndex.h"
#include "rootUtil/FileUtil.h"
#include "rootUtil/DataHandle.h"
#include "rootUtil/CelFileAndTreeSet.h"

#include <TTree.h>
#include <Riostream.h>



//====================================================================
// 
//====================================================================


ClassImp(CelEntryIndex) ;

CelEntryIndex::CelEntryIndex()
 : _componentName(),
   _entryIndex(-1,*this,"EntryIndex"),  
   _treeIndex(FileUtil::NOKEY,*this,"TreeIndex")
 { DataHandleInstance::init() ; }
	      
CelEntryIndex::CelEntryIndex( const TString & componentName )
 : _componentName(componentName),
   _entryIndex(-1,*this,"EntryIndex"),  
   _treeIndex(FileUtil::NOKEY,*this,"TreeIndex")
 { DataHandleInstance::init() ; }	      
	      
CelEntryIndex::CelEntryIndex( const CelEntryIndex & other)
 : BranchGroup(),
   _componentName(other._componentName),
   _entryIndex(other._entryIndex,*this,"EntryIndex"),  
   _treeIndex(other._treeIndex,*this,"TreeIndex")
 { DataHandleInstance::init() ; }	      

// D'tor is a no-op
CelEntryIndex::~CelEntryIndex()
 {}

// Assignment operator
CelEntryIndex & CelEntryIndex::operator=( const CelEntryIndex & other )
 {
  if ( this == &other ) return *this ;
  _componentName = other.componentName() ;
  _entryIndex  = other.entryIndex() ;
  _treeIndex = other.treeIndex() ;
  return *this ;
 }



//====================================================================
// 
//====================================================================


// Grab the current status from 'tree'
//
// The event index is set from tree.GetReadEntry();
// The tree index is looked up in handle
void CelEntryIndex::set( TTree & tree, CelFileAndTreeSet & handle )
 {
  _entryIndex = tree.GetReadEntry() ;
  UShort_t tIdx = handle.getKey(&tree) ;
  if ( tIdx == FileUtil::NOKEY )
   { tIdx = handle.addTree(tree) ; }
  _treeIndex = tIdx ;
 }

// Read an event using handle to look up the correct tree
//
// The tree lookup uses _treeIndex to get the tree from 'handle'
// Then the Entry _entryIndex is loaded using LoadTree(_entryIndex);
Int_t CelEntryIndex::read( const CelFileAndTreeSet & handle )
 {
  TTree * t = handle.getTree(_treeIndex) ;
  if ( 0 == t ) return -1 ;
  return t->LoadTree(_entryIndex) ;
 }

// Use 'handle' to look up a tree 
//
// The tree lookup uses _treeIndex to get the tree from 'handle'
TTree * CelEntryIndex::getTree( const CelFileAndTreeSet & handle ) const
 {
  return handle.getTree(_treeIndex) ;
 }

// Print the information about the current event to cout
// 
// The print format is treeIndex:entryIndex
void CelEntryIndex::printEventInfo( const char * /* options */ ) const
 {
  std::cout << _treeIndex << ':' << _entryIndex ;
 }

