
#ifndef CompositeEventList_h
#define CompositeEventList_h

/*
* Project: GLAST
* Package: rootUtil
* File: $Id: CompositeEventList.h,v 1.18 2007/12/07 14:44:04 chamont Exp $
* Authors:
*   EC, Eric Charles , SLAC, echarles@slac.stanford.edu
*   DC, David Chamont, LLR, chamont@llr.in2p3.fr
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*/

#include "CelEventLink.h"
class CelEventComponent ;
class CelIndex ;

#include <TObject.h>
#include <TString.h>
class TTree ;
class TFile ;
class TChain ;
class TVirtualIndex ;
class TCollection ;
class TObjArray ;
class TObjString ;

#include <map>
#include <vector>

//
// The class CompositeEventList permits to represent in memory
// a list of composite events, where each event is described as
// a set of entries in several separate trees. Each tree contains
// a different kind of data, and is also called a "component".
//
// An instance of CompositeEventList can be read from and/or
// written to a ROOT file. We should also consider the construction
// of a CompositeEventList on-the-fly, so to support the good old
// way to read/write events as was done by former RootIo writers
// and readers.
//
// The information is stored on 5 trees
// 
//   TO BE DONE : EventIDs: 3 branches, 1 entry per event
//      Event_ProductionID (char *)
//      Event_RunID        (Long64_t)
//      Event_EventID      (Long64_t)
// 
//   EventEntries: 2 Branches per component, 1 entry per event
//      <Comp>_Event_EntryIndex (Long64_t) : Index of the entry in the tree it lives on
//      <Comp>_Event_TreeIndex  (Short_t)  : Index of the tree in the associated set of files and trees
//
//   EventLinks: 2 branches, 1 entry per event
//      Event_SetIndex  (Long64_t) : Index of the associated set of files and trees
//      Event_SetOffset (Long64_t) : Number of events in previous sets
// 
//   FileAndTreeSets: 5 Branches per component, 1 entry per set
//      <Comp>_Set_Size        -> Number of Files and Trees in the current set
//      <Comp>_Set_FileNames   -> TClonesArray<TObjString> with the file names
//      <Comp>_Set_TreeNames   -> TClonesArray<TObjString> with the tree names (should all be the same ?)
//      <Comp>_Set_TreeOffsets -> TArrayL64 with the offsets for each Tree in the current set
//      <Comp>_Set_Entries     -> Total number of entries in all trees from the current set
//
// Note : the pair (RunID/EventID) is expected to be unique for GLAST real data. For
// what concerns the simulation data, the triplet (ProductionID/RunID/EventID) should
// be unique.
//   

class CompositeEventList : public TObject
 {

  public :

    // Construction, components, state
    explicit CompositeEventList
     ( const TString & celFileName = "",
       const TString & options = "READ",
       const TObjArray * componentNames = 0 ) ;
    Bool_t isOk() ;
    ~CompositeEventList() ;

    // Writing interface
    Long64_t fillEvent( const TObjArray & trees ) ;
    Long64_t fillEvent( const std::vector<TTree*> & ) ;
    Long64_t fillEvent( const std::vector<TChain*> & ) ;
    Long64_t fillFileAndTreeSet() ;
    void writeAndClose() ;
    
    // Reading interface
    // read only the event information
    Int_t shallowRead( Long64_t eventIndex ) ;
//    // declare the address where to store the data
//    void setDataAddress
//     ( const TString & componentName,
//       const TString & branchName,
//       void** address ) ;
    // UNUSED ? read the event information and data (data trees made on the fly ??)
    Int_t deepRead( Long64_t eventIndex ) ;   
    // USED ? Build all the Chains for all the components
    TChain * newChain( UInt_t componentIndex ) ;
    TChain * newChains( TObjArray * chainList = 0, Bool_t setFriends = kTRUE ) ;

    // Accessors
    UInt_t numComponents() const ;
    Long64_t numEvents() const ;
    Long64_t numFileAndTreeSets() const ;
    const TString & fileName() const ;
    const TString & componentName( UInt_t componentIndex ) const ;
    Long64_t currentEventIndex() const ;
    Long64_t currentSetIndex() const ;
    
    
    /// PRINTING
    // Dump a set of event component pointers and the list of TTree where they live
    void printInfo( const char * options ="", UInt_t nEvent=0, UInt_t startEvent=0) ;
    // Dump a single event
    void printEventInfo( const char * options ="" ) ;
    // Dump the list of TTree where our events live
    void printSetsInfo( const char * options ) ;

    /// FOR CelUtil
    TTree * entryTree() { return _entryTree ; }
    TTree * linkTree() { return _linkTree ; }
    TTree * fileTree() { return _fileTree ; }
    TTree * offsetTree() { return _offsetTree ; }
    CompositeEventList( TTree * eventTree, TTree * fileTree, TTree * entryTree ) ;
    
	/// FOR CelIndex
    CelEventComponent * getComponent( const TString & name ) const ;
  
     
  private :
	  
//    typedef std::pair<TTree*,CelEventComponent*> CelTreeAndComponent ;
//	  typedef std::vector<CelTreeAndComponent> CelTreesAndComponents ;

    // Disable copying and assignment
    CompositeEventList( const CompositeEventList & ) ;
    CompositeEventList & operator=( const CompositeEventList & ) ;

    // Manipulation of cel internal trees
    void deleteCurrentFile() ;
    Bool_t checkCelTrees() ;
    Bool_t checkCelTree( TTree *, const std::string & name, Bool_t error ) ;
    Int_t makeCelBranches( TTree * entryTree, TTree * linkTree, TTree * fileTree, TTree * offsetTree, Int_t bufsize = 32000) const;
    Int_t attachToTree( TTree * entryTree, TTree * linkTree, TTree * fileTree, TTree * offsetTree ) ;

    // Components utilities
    UInt_t CompositeEventList::declareComponent( const TString & name ) ;
    UInt_t CompositeEventList::declareComponents( const TObjArray * componentNames ) ;
    UInt_t discoverComponents() ;
    CelEventComponent * getComponent( UInt_t index ) const ;
    // UNUSED ? Get a Tree that is being read by a deepRead
    TTree * getTree( UInt_t componentIndex ) const ;
    TTree * getTree( const TString & componentName ) const ;

    // specific construction methods
    void prepareRead() ;
    void prepareRecreate() ;
    
    // cel data
    Bool_t  _isOk ;
    TString _fileName ;
    TString _openingOptions ;
    TFile * _currentFile ;
    TTree * _entryTree ; 
    TTree * _linkTree ; 
    TTree * _fileTree ; 
    TTree * _offsetTree ; 
    
    // current event info
    CelEventLink _currentLink ;

    // components stuff
    std::vector<CelEventComponent*> _compList ;  //! vector of components
    std::vector<TString> _compNames ;   //! names of components
    std::map<TString,UInt_t> _compMap ; //! components lookup map to get the index in the list above from the name

    ClassDef(CompositeEventList,0)

 } ;


#endif
