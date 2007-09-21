// -*- Mode: c++ -*-
#ifndef CompositeEventList_cxx
#define CompositeEventList_cxx
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: CompositeEventList.cxx,v 1.4 2007/09/19 16:57:05 chamont Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*
*
*/

// This Class's header
#include "rootUtil/CompositeEventList.h"

// c++/stl headers
#include <iostream>
#include <assert.h>

// ROOT Headers
#include <TTree.h>
#include <TChain.h>
#include <TFile.h>
#include <TObjArray.h>

// Other headers from this package
#include "rootUtil/CelFileAndTreeSet.h"
#include "rootUtil/CelEventComponent.h"
#include "rootUtil/CelIndex.h"
#include "rootUtil/FileUtil.h"

ClassImp(CompositeEventList);

CompositeEventList::CompositeEventList()
 : TObject(),
   _linkTree(0), _fileTree(0), _entryTree(0),
   _linkEntry()
 {}
	    
CompositeEventList::CompositeEventList
 ( TTree & linkTree, TTree & fileTree, TTree & entryTree )
 : TObject(),
   _linkTree(&linkTree), _fileTree(&fileTree), _entryTree(&entryTree),
   _linkEntry()
 { attachToTree(linkTree,fileTree,entryTree) ; }

CompositeEventList::~CompositeEventList()
 { cleanup() ; }


TFile * CompositeEventList::makeFile
 ( const TString & fileName, const Char_t * options)
 {
  // Make a new file.  Will also delare TTree for storing the pointers
  //
  // This makes two TTrees in the new file.
  //   Events -> One entry per event.  Has the indices to point to events.
  //   Files  -> One entry per input collection.  Stores the File names and TTree names. 
  //
  TDirectory * oldDir = gDirectory ;
  TFile * newFile = TFile::Open(fileName,options) ;
  if ( 0 == newFile ) {
    // Root has already warned.  Just return NULL
    return 0 ;
  }
  newFile->cd();
  _linkTree = new TTree("Links","Links");
  _fileTree = new TTree("FileAndTreeSets","FileAndTreeSets");
  _entryTree = new TTree("ComponentEntries","ComponentEntries");
  oldDir->cd() ;
  Int_t check = makeBranches(*_linkTree,*_fileTree,*_entryTree,32000);
  if ( check < 0 ) {
    // Failed to make branches for some reason.  
    // Already warned.  clean up and return NULL
    cleanup();
    delete newFile;
    return 0;
  }
  return newFile;
}

TFile * CompositeEventList::openFile( const TString & fileName )
 {
  // Open an existing file which contains a composite event list
  //
  // This will warn and return NULL if 
  //   a) the file doesn't exist
  //   b) the file doesn't contain a composite event list
	
  TDirectory * oldDir = gDirectory ;
  TFile * newFile = TFile::Open(fileName) ;
  if ( 0 == newFile ) return 0 ;
  newFile->cd() ;
  _entryTree = dynamic_cast<TTree*>(newFile->Get("ComponentEntries")) ;
  _linkTree = dynamic_cast<TTree*>(newFile->Get("Links")) ;
  _fileTree = dynamic_cast<TTree*>(newFile->Get("FileAndTreeSets")) ;
  if  ( _entryTree == 0 || _linkTree == 0 || _fileTree == 0 ) {
    std::cerr << "Not TTrees" << std::endl;
    cleanup();
    delete newFile;
  }
  if ( _compList.size() == 0 ) {
    buildComponents(*_entryTree);
  }
  oldDir->cd();
  Int_t check = attachToTree(*_linkTree,*_fileTree,*_entryTree);
  if ( check < 0 ) {
    std::cerr << "File is not a valid skim" << std::endl;
    cleanup();
    delete newFile;   
    return 0;
  }
  return newFile;
}

UInt_t CompositeEventList::addComponent(const std::string& name){
  // Add a component by name.  This is only need when writing.  On read these are discovered.
  //
  // Returns the component index value
  CelEventComponent* comp = new CelEventComponent(name);
  TTree* t(0);
  CelTreeAndComponent tc(t,comp);
  // David: below, it should rather be _compList, no ?
  // (besides, it works this way, lucky guys)
  UInt_t retValue = _compMap.size();
  _compMap[name] = retValue;
  _compNames.push_back(name);
  _compList.push_back(tc);
  return retValue;
}

Long64_t CompositeEventList::fillEvent(TObjArray& trees) {
  // Set up an event.  Grab the status of a set of TTrees
  //
  // Returns the entry number of the event that has just been written
  // Returns -1 if it failed to fill the event
  // This is here since CINT screws up with std::vector<TTree*> 
  std::vector<TTree*> v;
  for ( Int_t i(0); i < trees.GetEntries(); i++ ) {
    TTree* t = static_cast<TTree*>(trees.UncheckedAt(i));
    v.push_back(t);
  }
  return fillEvent(v);
}

Long64_t CompositeEventList::fillEvent(std::vector<TTree*>& trees) {
  // Set up an event.  Grab the status of a set of TTrees
  //
  // Returns the entry number of the event that has just been written  
  if ( ! checkDataFiles() ) return -1 ;
  if ( ! set(trees) ) return -1 ;
  _linkEntry.incrementEventIndex() ;
  _entryTree->Fill() ;
  _linkTree->Fill() ;
  return _linkEntry.eventIndex() ;
}

Long64_t CompositeEventList::fillFileAndTreeSet() {
  // Store up the List of TTrees that have been used so far
  //
  // Returns the entry number of the entry that has just been written    
  if ( ! checkDataFiles() ) return -1;
  _fileTree->Fill() ;
  Long64_t retValue = _linkEntry.setIndex() ;
  _linkEntry.incrementSetIndex() ;
  return retValue;
}


Int_t CompositeEventList::read( Long64_t iEvt )
 {
  // Read an event by Index
  // 
  // If successful, returns the number of bytes read
  // Failure codes:
  //   -1 -> Input Trees do not exist.  Probably not yet attached
  //   -2 -> Can't read entry on Event input tree
  //   -3 -> Can't read entry on Link input tree
  //   -4 -> Can't read entry on File input tree
  //   -5 -> Can't find a component
  //   -6 -> Can't read a component input tree
  Int_t total = readEventAddress(iEvt) ;
  if ( total < 0 ) return total ;

  std::vector< CelTreeAndComponent >::iterator itr ;
  for ( itr = _compList.begin() ; itr != _compList.end() ; itr++ ) {
    CelEventComponent* comp = itr->second;
    if ( comp == 0 ) return -5;
    Int_t nRead = comp->read();
    if ( nRead < 0 ) return -6;
    TTree* t = comp->getTree();
    itr->first = t;
    total += nRead;
  }
  return total;
}


Int_t CompositeEventList::readEventAddress( Long64_t iEvt) {
  //
  // If successful, returns the number of bytes read
  // Failure codes:
  //   -1 -> Input Trees do not exist.  Probably not yet attached
  //   -2 -> Can't read entry on Event input tree
  //   -3 -> Can't read entry on Link input tree
  //   -4 -> Can't read entry on File input tree  
  if ( !checkDataFiles() ) return -1 ;

  if ( iEvt == _linkEntry.eventIndex() ) {
    // event already read,
    return 0;
  }

  if ( iEvt == 0 ) {
    ;
  } else if ( iEvt % 10000 == 0 ) {
    std::cout << 'x' << std::flush;
  } else if ( iEvt % 1000 == 0 ) {
    std::cout << '.' << std::flush;
  } 
  
  _linkEntry.setEventIndex(iEvt) ;
  Int_t total = _entryTree->GetEntry(iEvt);
  if ( total < 0 ) return -2;
  Int_t link = _linkTree->GetEntry(iEvt);
  if ( link < 0 ) return -3;
  total += link;

  Long64_t mEvt = _linkEntry.setIndex();

  Int_t nMeta = _fileTree->GetEntry(mEvt);
  if ( nMeta < 0 ) return -4;
  total += nMeta;

  return total;
}


TChain* CompositeEventList::buildChain(UInt_t index) {
  // Build the TChain for a component. 
  if ( ! checkDataFiles() ) {
    std::cerr << "No Data Files" << std::endl;
    return 0;
  }
  CelEventComponent* comp = getComponent(index);
  if ( 0 == comp ) {
    std::cerr << "No component with index " << index << std::endl;
    return 0;
  }
  TChain* chain(0);
  std::string chainName;
  for ( Long64_t i(0); i < _fileTree->GetEntries(); i++ ) {
    Int_t nB = _fileTree->GetEntry(i);    
    if ( nB < 0 ) {
      std::cerr << "Could not read Tree entry " << i << " for component " << _compNames[i] << std::endl;
      delete chain;
      return 0;
    }
    // this will build a new chain if needed
    Bool_t isOk = comp->addToChain(chain);
    if ( !isOk ) {
      std::cerr << "Could not add tree " << i << " to chain for component " << _compNames[i] << std::endl;
      delete chain;
      return 0;
    }
  }
  return chain;
}

TVirtualIndex* CompositeEventList::buildEventIndex(UInt_t index, Long64_t& offset, TTree* tree) {
  // Build the TChain for a component. 
  if ( ! checkDataFiles() ) {
    // FIXME: Warn?
    return 0;
  }
  TVirtualIndex* vIdx = CelIndex::buildIndex(*this,_compNames[index],tree,offset);
  return vIdx;
}

TChain* CompositeEventList::buildLinks(TObjArray* chainList, Bool_t setFriends){
  // Build the event chain

  if ( 0 == _entryTree ) return 0;
  TFile* f = FileUtil::getFile(*_entryTree);
  if ( 0 == f ) return 0;
    
  TChain* newChain = new TChain("Events");  
  Int_t check = newChain->Add(f->GetName());
  if ( check < 0 ) {
    delete newChain;
    return 0;
  }

  // Build all the Chains
  Long64_t offset(0);
  for ( UInt_t i(0); i < nComponent(); i++ ) {
    std::cout << "Building links for " << _compNames[i] << std::endl;
    TChain* c = buildChain(i);
    c->SetBranchStatus("*",0);
    if ( 0 == c ) {
      // FIXME: warn?
      return 0;
    }
    TVirtualIndex* vIdx = buildEventIndex(i,offset,c);
    if ( 0 == vIdx ) {
      // FIXME: warn?
      return 0;
    }     
    if ( 0 != chainList ) {
      chainList->AddLast(c);
    }
    if ( setFriends ) {
       newChain->AddFriend(c,_compNames[i].c_str());
    }
  }
  return newChain;
}


Long64_t CompositeEventList::entries() const { 
  // return the number of entries
  return _entryTree != 0 ? _entryTree->GetEntries() : 0;
}

void CompositeEventList::printout(const char* options, UInt_t nEvent, UInt_t startEvent){
  // Dump a set of event component pointers and the list of TTree where they live
  //
  // One line per event
  UInt_t lastEvt = startEvent+nEvent;
  std::cout << std::endl << "Printing Events: " << std::endl;
  for ( UInt_t iEvt(startEvent); iEvt < lastEvt; iEvt++ ) {
    read(iEvt);
    dumpEvent(options);
  }

  std::cout << std::endl << "Printing Trees: " << std::endl;
  listTrees(options);
}

void CompositeEventList::dumpEvent(const char* options){
  // Dump a single event on one line
  // 
  // The format is:
  //  Event:     comp1 tree0:event0   comp1 tree1:event1 ... 
  std::cout << "Event " << _linkEntry.eventIndex() << ":\t";
  for ( std::vector< CelTreeAndComponent >::const_iterator itr = _compList.begin();
	itr != _compList.end(); itr++ ) {
    const CelEventComponent* comp = itr->second;
    if ( comp == 0 ) return;
    std::cout << comp->componentName() << ' ';
    comp->dumpEvent(options);
    std::cout << "\t";
  } 
  std::cout << std::endl;
}

void CompositeEventList::listTrees(const char* options){
  // Dump the list of TTree where our events live
  //
  // One tree per line, the format is:
  //   File Tree
  for ( std::vector< CelTreeAndComponent >::const_iterator itr = _compList.begin();
	itr != _compList.end(); itr++ ) {
    const CelEventComponent* comp = itr->second;
    if ( comp == 0 ) return;
    std::cout << comp->componentName() << std::endl;
    comp->listTrees(options);
  }   
}



Int_t CompositeEventList::buildComponents(TTree& tree){
  // Uses the input event tree to discover the list of components
  //
  // Called by openFile()
  TObjArray* array = tree.GetListOfBranches();
  for ( Int_t i(0); i < array->GetEntries(); i++ ) {
    TObject* obj = array->UncheckedAt(i);
    std::string name(obj->GetName());    
    std::string::size_type find =  name.find("EntryIndex");
    if ( find == name.npos ) continue;
    std::string compName(name,0,find);
    addComponent(compName);
  }
  return nComponent();
}


Bool_t CompositeEventList::set(std::vector<TTree*>& trees) {
  // Latch the values in a set of TTree
  //
  // Called by fillEvent()
  assert ( checkDataFiles() ) ;
  if (trees.size() != _compList.size()) {
    std::cerr << "Wrong number of trees.  " 
	      << _compList.size() << " components and " << trees.size() << " trees." << std::endl;
    return kFALSE;
  }
  
  UInt_t idx(0);  
  for ( std::vector< TTree* >::iterator itr = trees.begin();
	itr != trees.end(); itr++ ) {  
    CelEventComponent* comp = getComponent(idx);
    assert ( 0 != comp );
    comp->set(**itr);
    idx++;
  }
  return kTRUE;
}


Int_t CompositeEventList::makeBranches( TTree & linkTree,  TTree & fileTree, TTree & eventTree, Int_t bufsize) const {
  // Make the Branches we store everything on
  // 
  // Called by makeFile()
  Int_t total = _linkEntry.makeBranches(linkTree,"Link_",bufsize);
  if ( total < 0 ) return total;
  for ( std::vector< CelTreeAndComponent >::const_iterator itr = _compList.begin();
	itr != _compList.end(); itr++ ) {
    const CelEventComponent* comp = itr->second;
    if ( comp == 0 ) return -1;
    Int_t nMake = comp->makeBranches(fileTree,eventTree,bufsize);
    if ( nMake < 0 ) return nMake;
    total += nMake;
  }
  return total;
}



Int_t CompositeEventList::attachToTree( TTree & linkTree,  TTree & fileTree, TTree & eventTree )
 {
  // Attach to the branches where all the infomation is stored
  // 
  // Called by openFile()
  Int_t total = _linkEntry.attachToTree(linkTree,"Link_");
  if ( total < 0 ) {
    std::cerr << "Failed to open Link Tree" << std::endl;    
    return total;
  }
  UInt_t idx(0);
  for ( std::vector< CelTreeAndComponent >::const_iterator itr = _compList.begin();
	itr != _compList.end(); itr++ ) {
    CelEventComponent* comp = itr->second;
    if ( comp == 0 ) { 
      std::cerr << "Failed to get component " << idx << std::endl;
      return -1;
    }
    Int_t nB = comp->attachToTree(eventTree,fileTree);
    if ( nB < 0 ) {
      std::cerr << "Failed to attached component " << comp->componentName() << " to trees " 
		<< eventTree.GetName() << ' ' << fileTree.GetName() << std::endl;
      return nB;
    }
    total += nB;
    idx++;
  }
  return total;
}

void CompositeEventList::cleanup() {
  // cleanup in case there were problems with files
  delete _entryTree; _entryTree = 0;
  delete _linkTree; _linkTree = 0;
  delete _fileTree; _fileTree = 0;
}

Bool_t CompositeEventList::checkDataFiles() {
  // make sure that files exist.  pre-requisite for processing
  if ( 0 == _entryTree ) return kFALSE;
  if ( 0 == _linkTree ) return kFALSE;  
  if ( 0 == _fileTree ) return kFALSE;  
  return kTRUE;
}

#endif
