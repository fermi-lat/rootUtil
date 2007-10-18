
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CompositeEventList.cxx,v 1.12 2007/10/16 15:19:23 chamont Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*/

#include "rootUtil/CompositeEventList.h"
#include "rootUtil/CelEventComponent.h"
#include "rootUtil/CelIndex.h"
#include "rootUtil/FileUtil.h"

#include <TTree.h>
#include <TChain.h>
#include <TFile.h>
#include <TObjArray.h>
#include <TSystem.h>
#include <Riostream.h>

#include <assert.h>



//====================================================================
//
//====================================================================


ClassImp(CompositeEventList) ;

static std::string __entryTreeName("EventEntries") ;
static std::string __linkTreeName("EventLinks") ;
static std::string __fileTreeName("FileAndTreeSets") ;
static std::string __offsetTreeName("FileAndTreeOffsets") ;

CompositeEventList::CompositeEventList()
 : _entryTree(0), _linkTree(0), _fileTree(0), _offsetTree(0)
 {}
	    
CompositeEventList::~CompositeEventList()
 { deleteCelTrees() ; }

void CompositeEventList::deleteCelTrees()
 {
  delete _entryTree  ; _entryTree = 0  ;
  delete _linkTree   ; _linkTree = 0   ;
  delete _fileTree   ; _fileTree = 0   ;
  delete _offsetTree ; _offsetTree = 0 ;
 }

Bool_t CompositeEventList::checkCelTree
 ( TTree * tree, const std::string & name, Bool_t error )
 {
  if ( 0 == tree )
   {
	if ( error == kTRUE )
	 {
	  std::cerr
	    <<"[CompositeEventList::checkCelTree] "
	    <<"lacking tree "<<name
	    <<std::endl ;
	  return kFALSE ;
	 }
	else
	 {
	  std::cout
	    <<"[CompositeEventList::checkCelTree] "
	    <<"no tree "<<name
	    <<std::endl ;
	 }
	return kFALSE ;
   }
  if ( name != tree->GetName() )
   {
	std::cerr
	  << "[CompositeEventList::checkCelTree] "
	  << "a tree is called "<<tree->GetName()
	  << " instead of "<<name
	  << std::endl ;
	return kFALSE ;
   }
  return kTRUE ;
 }

Bool_t CompositeEventList::checkCelTrees()
 {
  if (checkCelTree(_entryTree,__entryTreeName,kTRUE)==kFALSE) return kFALSE ;
  if (checkCelTree(_linkTree,__linkTreeName,kTRUE)==kFALSE) return kFALSE ;
  if (checkCelTree(_fileTree,__fileTreeName,kTRUE)==kFALSE) return kFALSE ;
  checkCelTree(_offsetTree,__offsetTreeName,kFALSE) ;
  return kTRUE ;
 }



//====================================================================
// components setup
//====================================================================


UInt_t CompositeEventList::addComponent( const TString & name )
 {
  // Add a component by name.  This is only need when writing.
  // On read these are discovered.
  // Returns the component index value
  CelEventComponent * c = new CelEventComponent(name) ;
  TTree * dataTree = 0 ;
  CelTreeAndComponent tc(dataTree,c) ;
  UInt_t retValue = _compList.size() ;
  _compList.push_back(tc) ;
  _compNames.push_back(name) ;
  _compMap[name] = retValue ;
  return retValue ;
 }

// Uses the input event tree to discover the list of components
UInt_t CompositeEventList::buildComponents( TTree & entryTree )
 {
  TObjArray * array = entryTree.GetListOfBranches() ;
  Int_t i ;
  for ( i = 0 ; i < array->GetEntries() ; i++ )
   {
    TObject * obj = array->UncheckedAt(i) ;
    TString name = obj->GetName() ;
    Ssiz_t find = name.Index("_Event_EntryIndex") ;
    if (find==kNPOS) continue ;
    name.Remove(find) ;
    addComponent(name) ; 
    //std::string name(obj->GetName()) ;    
    //std::string::size_type find =  name.find("EntryIndex") ;    
    //if ( find == name.npos ) continue ;   
    //std::string compName(name,0,find) ; 
    //addComponent(compName) ;
   }
  return numComponents() ;
}

CelEventComponent * CompositeEventList::getComponent( UInt_t index ) const
 { return (index < _compList.size()) ? _compList[index].second : 0 ; }

CelEventComponent * CompositeEventList::getComponent( const TString & name ) const
 {
  std::map<TString,UInt_t>::const_iterator itrFind = _compMap.find(name) ;
  return itrFind != _compMap.end() ? getComponent(itrFind->second) : 0 ;
 }



//====================================================================
// write use-case
//====================================================================


TFile * CompositeEventList::makeCelFile
 ( const TString & celFileName, const Char_t * options )
 {
  // expand
  TString fileName = celFileName ;
  if (gSystem->ExpandPathName(fileName)==kTRUE)
   {
    std::cerr
      << "[CompositeEventList::makeFile] "
      << "Failed to expand some variable in filename : "
      << celFileName
      << std::endl ;
    return 0 ;
   }

  // make file
  TDirectory * oldDir = gDirectory ;
  TFile * newFile = TFile::Open(fileName,options) ;
  if ((newFile==0)||(!newFile->IsOpen()))
   {
	std::cerr
	  << "[CompositeEventList::openFile] "
      << "Failed to open file : "
      << fileName
      << std::endl ;
	return 0 ;
   }
  
  // make trees
  newFile->cd() ;
  _entryTree = new TTree(__entryTreeName.c_str(),__entryTreeName.c_str()) ;
  _linkTree = new TTree(__linkTreeName.c_str(),__linkTreeName.c_str()) ;
  _fileTree = new TTree(__fileTreeName.c_str(),__fileTreeName.c_str()) ;
  _offsetTree = new TTree(__offsetTreeName.c_str(),__offsetTreeName.c_str()) ;
  oldDir->cd() ;
  
  // make branches
  Int_t check = makeCelBranches(_entryTree,_linkTree,_fileTree,_offsetTree,32000) ;
  if ( check < 0 )
   {
    // Failed to make branches for some reason.  
    // Already warned.  clean up and return NULL
    deleteCelTrees() ;
    delete newFile ;
    return 0 ;
   }
  
  // finally ok
  return newFile ;
 }

Int_t CompositeEventList::makeCelBranches
 ( TTree * entryTree, TTree * linkTree,  TTree * fileTree,  TTree * offsetTree, Int_t bufsize ) const
 {
  Int_t n, total = 0 ;
  if (linkTree!=0)
   {
	n = _currentLink.makeBranches(*linkTree,0,bufsize) ;
	if ( n < 0 ) return n ;
    total += n ;
   }
  std::vector< CelTreeAndComponent >::const_iterator compItr ;
  for ( compItr = _compList.begin() ; compItr != _compList.end() ; compItr++ )
   {
    const CelEventComponent * comp = compItr->second ;
    if ( comp == 0 ) return -1 ;
    n = comp->makeBranches(entryTree,fileTree,offsetTree,bufsize) ;
    if ( n < 0 ) return n ;
    total += n ;
   }
  return total ;
 }



//====================================================================
// merge use-case
//====================================================================


CompositeEventList::CompositeEventList
 ( TTree * entryTree, TTree * linkTree, TTree * fileTree )
 : _entryTree(entryTree), _linkTree(linkTree), _fileTree(fileTree)
 {
  // prepare components
  buildComponents(*entryTree) ;
  
  // prepare and connect new offset tree
  _offsetTree = new TTree(__offsetTreeName.c_str(),__offsetTreeName.c_str()) ;
  Int_t check = makeCelBranches(0,0,0,_offsetTree,32000) ;
  if ( check < 0 )
   {
    // Failed to make branches for some reason.  
    // Already warned.  clean up and return NULL
    deleteCelTrees() ;
    return ;
   }
  
  // attach all trees, and insert first 0 offsets
  attachToTree(_entryTree,_linkTree,_fileTree,_offsetTree) ;
  std::vector< CelTreeAndComponent >::iterator compItr ;
  for ( compItr = _compList.begin() ; compItr != _compList.end() ; compItr++ )
   {
    CelEventComponent * comp = compItr->second ;
    if ( comp == 0 )
     { std::cerr<<"[CompositeEventList::CompositeEventList(,,,)] problem with components"<<std::endl ; return ; }
    comp->currentFileOffset().reset() ;
   }
  _offsetTree->Fill() ;
  
  // compute the offsets
  _fileTree->SetBranchStatus("*",kFALSE) ;
  _fileTree->SetBranchStatus("*_Set_Entries",kTRUE) ;
  Long64_t nEntries, iSet, nSet = _fileTree->GetEntries() ;
  for ( iSet = 0 ; iSet < (nSet-1) ; iSet++ )
   {
    Int_t nB = _fileTree->GetEntry(iSet) ;
    if ( nB < 0 )
     { std::cerr << "Failed to read entry in file tree " << std::endl ; }
    for ( compItr = _compList.begin() ; compItr != _compList.end() ; compItr++ )
     {
      CelEventComponent * comp = compItr->second ;
      if ( comp == 0 )
       { std::cerr<<"[CompositeEventList::CompositeEventList(,,,)] problem with components"<<std::endl ; return ; }
      nEntries = comp->currentFileSet().entries() ;
      comp->currentFileOffset().increment(nEntries) ;
     }
    _offsetTree->Fill() ;
   }
 }


 
//====================================================================
// read use-case
//====================================================================


TFile * CompositeEventList::openCelFile( const TString & celFileName )
 {
  // Open an existing file which contains a composite event list
  // This will warn and return 0 if 
  //   *) there is a failure when expanding the name
  //   *) there is a failure when opening the file
  //   *) the file doesn't contain all elements of composite event list
  // The component are discovered on the fly IF THEY
  // HAVE NOT BEEN PREDECLARED
	
  // expand
  TString fileName = celFileName ;
  if (gSystem->ExpandPathName(fileName)==kTRUE)
   {
    std::cerr
      << "[CompositeEventList::openFile] "
      << "Failed to expand some variable in filename : "
      << celFileName
      << std::endl ;
	return 0 ;
   }

  // open
  TDirectory * oldDir = gDirectory ;
  TFile * newFile = TFile::Open(fileName) ;
  if ((newFile==0)||(!newFile->IsOpen()))
   {
	std::cerr
	  << "[CompositeEventList::openFile] "
      << "Failed to open file : "
      << fileName
      << std::endl ;
	return 0 ;
   }
  
  // explore
  newFile->cd() ;
  _entryTree = dynamic_cast<TTree*>(newFile->Get(__entryTreeName.c_str())) ;
  _linkTree = dynamic_cast<TTree*>(newFile->Get(__linkTreeName.c_str())) ;
  _fileTree = dynamic_cast<TTree*>(newFile->Get(__fileTreeName.c_str())) ;
  _offsetTree = dynamic_cast<TTree*>(newFile->Get(__offsetTreeName.c_str())) ;
  if  ( _entryTree==0 || _linkTree==0 || _fileTree==0 || _offsetTree==0 )
   {
    std::cerr
	  << "[CompositeEventList::openFile] "
      << "Lacking cel tree(s)"
      << std::endl ;
    deleteCelTrees() ;
    delete newFile ;
    return 0 ;
   }
  oldDir->cd() ;
  
  // attach
  if (_compList.size()==0)
   { buildComponents(*_entryTree) ; } 
  Int_t check = attachToTree(_entryTree,_linkTree,_fileTree,_offsetTree) ;
  if (check<0)
   {
    std::cerr
	  << "[CompositeEventList::openFile] "
      << "File is not a valid composite event list"
      << std::endl ;
    deleteCelTrees() ;
    delete newFile ;   
    return 0 ;
   }
  
  // everything finally OK
  return newFile ;
 }

Int_t CompositeEventList::attachToTree( TTree * entryTree, TTree * linkTree,  TTree * fileTree,  TTree * offsetTree )
 {
  // Attach to the branches where all the infomation is stored
  // 
  // Called by openFile()
  Int_t n, total = 0 ;
  if (linkTree!=0)
   {
    n = _currentLink.attachToTree(*linkTree) ;
    if ( n < 0 )
     {
      std::cerr << "Failed to attach Link Tree" << std::endl ;    
      return n ;
     }
    total += n ;
   }
  UInt_t idx = 0 ;
  CelTreesAndComponents::const_iterator itr ;
  for ( itr = _compList.begin() ; itr != _compList.end(); itr++ )
   {
    CelEventComponent * comp = itr->second ;
    if ( comp == 0 )
     { 
      std::cerr << "Failed to get component " << idx << std::endl ;
      return -1 ;
     }
    Int_t nB = comp->attachToTree(entryTree,fileTree,offsetTree) ;
    if ( nB < 0 )
     {
      std::cerr
        << "Failed to attached component " << comp->componentName() << " to trees " 
		<< entryTree->GetName() << ' ' 
		<< fileTree->GetName() << ' ' 
		<< offsetTree->GetName() << std::endl ;
      return nB;
     }
    total += nB ;
    idx++;
   }
  return total ;
 }

// If successful, returns the number of bytes read
// Failure codes:
//   -1 -> Al least one cel internal tree does not exist.
//   -2 -> Can't read entry on ComponentEntries
//   -3 -> Can't read entry on CompositeEvents
//   -4 -> Can't read entry on FileAndTreeSets
Int_t CompositeEventList::shallowRead( Long64_t eventIndex )
 {
  if ( !checkCelTrees() ) return -1 ;
  
  if ( eventIndex == _currentLink.eventIndex() )
   { return 0 ; }

  // is it useful below, for a shallow (=>fast) read ???
  if ( eventIndex != 0 )
   {
    if ( eventIndex % 10000 == 0 )
     { std::cout << 'x' << std::flush ; }
    else if ( eventIndex % 1000 == 0 )
     { std::cout << '.' << std::flush ; } 
   }
  
  //_currentLink.changeEventIndex(eventIndex) ; // ??
  
  // event entries
  Int_t totalRead = _entryTree->GetEntry(eventIndex) ;
  if ( totalRead < 0 ) return -2 ;
  Int_t eventInfoRead = _linkTree->GetEntry(eventIndex) ;
  if ( eventInfoRead < 0 ) return -3 ;
  totalRead += eventInfoRead ;

  Long64_t setIndex = _currentLink.setIndex() ;
  if (_fileTree->GetReadEntry()!=setIndex)
   {
    Int_t setRead = _fileTree->GetEntry(setIndex) ;
    if ( setRead < 0 ) return -4 ;
    totalRead += setRead ;
    Int_t offsetRead = _offsetTree->GetEntry(setIndex) ;
    if ( offsetRead < 0 ) return -4 ;
    totalRead += offsetRead ;
   }

  return totalRead ;
 }

// If successful, returns the number of bytes read
// Failure codes:
//   -1 to -4 -> see shallowRead()
//   -5 -> Can't find a component
//   -6 -> Can't read a component input tree
Int_t CompositeEventList::deepRead( Long64_t eventIndex )
 {
  Int_t total = shallowRead(eventIndex) ;
  if ( total < 0 ) return total ;
  CelTreesAndComponents::iterator itr ;
  for ( itr = _compList.begin() ; itr != _compList.end() ; itr++ )
   {
    CelEventComponent * comp = itr->second ;
    if ( comp == 0 ) return -5 ;
    Int_t nRead = comp->read() ;
    if ( nRead < 0 ) return -6 ;
    TTree * t = comp->getTree() ;
    itr->first = t ;
    total += nRead ;
   }
  return total ;
 }

// Build the TChain for a component. 
TChain * CompositeEventList::buildChain( UInt_t componentIndex )
 {
  // get component
  if (!checkCelTrees()) { return 0 ; }
  CelEventComponent * comp = getComponent(componentIndex) ;
  if (comp==0)
   {
    std::cerr<<"No component with index "<<componentIndex<<std::endl ;
    return 0 ;
   }
  
  // the chain is made on the fly because we
  // do not know the tree name.
  TChain * chain =0 ;
  Long64_t i ;
  for ( i=0 ; i<_fileTree->GetEntries() ; i++ )
   {
    Int_t nB = _fileTree->GetEntry(i) ;    
    if (nB<0)
     {
      std::cerr
        << "Could not read _fileTree set " << i
        << " for component " << _compNames[i]
        << std::endl ;
      delete chain ;
      return 0 ;
     }
    // this will build a new chain if needed
    Bool_t isOk = comp->addToChain(chain) ;
    if (!isOk)
     {
      std::cerr
        << "Could not add tree " << i 
        << " to chain for component " << _compNames[i] 
        << std::endl ;
      delete chain ;
      return 0 ;
     }
   }
  return chain ;
 }

// Build event and data chains
TChain * CompositeEventList::buildAllChains( TObjArray * chainList, Bool_t setFriends )
 {
  // preconditions
  if (!checkCelTrees()) { return 0 ; }
  
  // build main chain
  TFile * f = FileUtil::getFile(*_entryTree) ;
  if (f==0) return 0 ;    
  TChain * mainChain = new TChain("CompositeEvents") ;  
  Int_t check = mainChain->Add(f->GetName()) ;
  if ( check < 0 )
   {
    delete mainChain ;
    return 0 ;
   }

  // build components chains
  UInt_t i ;
  for ( i=0 ; i<numComponents() ; i++ )
   {
    std::cout<<"Building chain for "<<_compNames[i]<<std::endl ;
    TChain * c = buildChain(i) ;
    if (c==0) { return 0 ; }
    c->SetBranchStatus("*",0) ; // ???
    TVirtualIndex * vIdx = new CelIndex(*this,_compNames[i],c) ;
    if (vIdx==0) { return 0 ; }     
    c->SetTreeIndex(vIdx) ;
    if (chainList!=0) { chainList->AddLast(c) ; }
    if ( setFriends )
     { mainChain->AddFriend(c,_compNames[i].Data()) ; }
  }
  
  // end
  return mainChain ;
 }



//====================================================================
//
//====================================================================


Long64_t CompositeEventList::fillEvent( TObjArray & trees )
 {
  // Set up an event.  Grab the status of a set of TTrees
  //
  // Returns the entry number of the event that has just been written
  // Returns -1 if it failed to fill the event
  // This is here since CINT screws up with std::vector<TTree*> 
  std::vector<TTree*> v;
  for ( Int_t i(0); i < trees.GetEntries(); i++ )
   {
    TTree* t = static_cast<TTree*>(trees.UncheckedAt(i));
    v.push_back(t);
  }
  return fillEvent(v);
 }

// Insert an event in the CEL. Grab the status of a set of TTrees
// Returns the entry number of the event that has just been written  
Long64_t CompositeEventList::fillEvent( std::vector<TTree *> & trees )
 {
  if ( ! checkCelTrees() ) return -1 ;
  
  if (trees.size() != _compList.size())
   {
    std::cerr
      << "Do not know how to match " << _compList.size() << " components "
      << "with " << trees.size() << " trees."
      << std::endl ;
    return -1 ;
   }
  
  // TODO : check tree and component names ?
  UInt_t idx(0) ;
  std::vector< TTree* >::iterator itr ;
  for ( itr = trees.begin() ; itr != trees.end() ; itr++ )
   {  
    CelEventComponent * comp = getComponent(idx) ;
    assert ( 0 != comp ) ;
    comp->registerEntry(**itr) ;
    idx++ ;
   }
  
  _currentLink.incrementEventIndex() ;
  _entryTree->Fill() ;
  _linkTree->Fill() ;
  return _currentLink.eventIndex() ;
}

// Store up the List of TTrees that have been used so far
// Returns the entry number of the entry that has just been written    
Long64_t CompositeEventList::fillFileAndTreeSet()
 {
  if ( ! checkCelTrees() ) return -1 ;
  _fileTree->Fill() ;
  _offsetTree->Fill() ;

  Long64_t retValue = _currentLink.setIndex() ;
  _currentLink.incrementFileSetIndex() ;
  
  CelTreesAndComponents::iterator itr ;
  for ( itr = _compList.begin() ; itr != _compList.end() ; itr++ )
   {
    CelEventComponent * comp = itr->second ;
    if ( comp == 0 ) return -1 ;
    comp->nextSet() ;
   }
  
  return retValue ;
}


Long64_t CompositeEventList::numEvents() const { 
  // return the number of entries
  return _linkTree != 0 ? _linkTree->GetEntries() : 0 ;
}



//Bool_t CompositeEventList::set(std::vector<TTree*>& trees) {
//  // Latch the values in a set of TTree
//  //
//  // Called by fillEvent()
//  assert ( checkCelTrees() ) ;
//  if (trees.size() != _compList.size()) {
//    std::cerr << "Wrong number of trees.  " 
//	      << _compList.size() << " components and " << trees.size() << " trees." << std::endl;
//    return kFALSE;
//  }
//  
//  UInt_t idx(0);  
//  for ( std::vector< TTree* >::iterator itr = trees.begin();
//	itr != trees.end(); itr++ ) {  
//    CelEventComponent* comp = getComponent(idx);
//    assert ( 0 != comp );
//    comp->set(**itr);
//    idx++;
//  }
//  return kTRUE;
//}
//


//====================================================================
// print and debug
//====================================================================


// Dump a set of event component pointers and the list of TTree where they live
void CompositeEventList::printout
 ( const char * options, UInt_t nEvent, UInt_t startEvent )
 {
  std::cout << std::endl << "Printing Events: " << std::endl ;
  UInt_t lastEvt = startEvent+nEvent, iEvt ;
  for ( iEvt = startEvent ; iEvt < lastEvt ; iEvt++ )
   {
    shallowRead(iEvt) ;
    dumpEvent(options) ;
   }
  std::cout << std::endl << "Printing Trees: " << std::endl ;
  listTrees(options) ;
 }

// Dump a single event on one line
void CompositeEventList::dumpEvent( const char * options )
 {
  std::cout << "Event " << _currentLink.eventIndex() << ":\t" ;
  CelTreesAndComponents::const_iterator itr ;
  for ( itr = _compList.begin() ; itr != _compList.end() ; itr++ )
   {
    const CelEventComponent * comp = itr->second ;
    if ( comp == 0 ) return ; // ??
    std::cout << comp->componentName() << ' ';
    comp->dumpEvent(options) ;
    std::cout << "\t";
   } 
  std::cout << std::endl ;
 }

// Dump the list of TTree where our events live
void CompositeEventList::listTrees( const char * options )
 {
  CelTreesAndComponents::const_iterator itr ;
  for ( itr = _compList.begin() ; itr != _compList.end() ; itr++ )
   {
    const CelEventComponent * comp = itr->second ;
    if ( comp == 0 ) return ; // ??
    std::cout << comp->componentName() << std::endl ;
    comp->listTrees(options) ;
  }   
}



