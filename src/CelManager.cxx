
/** 
* @file CelManager.cxx
* @brief definition of the class CelManager
*
* File: $Header: /nfs/slac/g/glast/ground/cvs/rootUtil/src/CelManager.cxx,v 1.9 2008/01/22 21:05:52 heather Exp $
* Authors:
*   HK, Heather Kelly, heather@lheapop.gsfc.nasa.gov
*   DC, David Chamont, LLR, chamont@poly.in2p3.fr
*/

#include <rootUtil/CelManager.h>
#include <rootUtil/CelEventComponent.h>
#include <TSystem.h>
#include <Riostream.h>
#include <TObjString.h>




//========================================================================
//
// Construction
//
//========================================================================


CelManager::CelManager( Bool_t verbose )
 : m_verbose(verbose), 
   m_fileNameWrite(""), m_initWriteDone(kFALSE), /*m_fileWrite(0),*/ m_celWrite(0), m_eventCounter(0),
   m_fileNameRead(""),  /*m_fileRead(0),*/ m_celRead(0), m_compChainCol(0), m_masterChain(0)  
 {}

CelManager::~CelManager() 
 {
  m_treeCol.clear() ;
  if (m_compChainCol) m_compChainCol->Delete() ;
  if (m_masterChain) delete m_masterChain ;
  delete m_celWrite ;
  delete m_celRead ;
 }



//========================================================================
//
// Writing Methods
//
//========================================================================


Bool_t CelManager::initWrite( const TString & celFileName, const TString & options )
 {
  Bool_t stat = kTRUE ;
  m_fileNameWrite = celFileName ;
  m_outputOptions = options ;
  // real work is delayed until we are sure
  // all the components have been declared
  return stat ;
 }

Bool_t CelManager::delayedInitWrite()
 {
  // the creation of the output file has been delayed until
  // we are sure all the components have been declared.
	
  if (m_initWriteDone==kTRUE)
   { return m_initWriteDone ; }
  
  m_celWrite = new CompositeEventList(m_fileNameWrite,m_outputOptions,&m_componentNamesCol) ;
  if (m_celWrite->isOk()==kFALSE)
//  m_fileWrite = m_celWrite->makeCelFile(m_fileNameWrite,m_outputOptions.Data()) ;
//  if (m_fileWrite==0)
   {
    std::cerr
     << "[CelManager::delayedInitWrite] "
     << "Error while making "
     << m_fileNameWrite << std::endl ;
    return m_initWriteDone ;
   } else {
   // HMK Should set to true if m_celWrite is ok, avoid throwing exception
   // in fillEvent
       m_initWriteDone=kTRUE;
   }
  
  return m_initWriteDone ;
 }


Bool_t CelManager::addComponent( const TString & compName, TTree * t )
 {
  m_componentNamesCol.Add(new TObjString(compName)) ;
  m_treeCol.push_back(t) ;
  return kTRUE ;
 }

Bool_t CelManager::fillEvent()
 {
  Bool_t stat = kTRUE ;
  try
   {
    // [David] presumably useless
    //TDirectory * saveDir = gDirectory ;
	  
    // Need to call CompositeEventList::makeFile after the AddComponent calls
    if ((m_initWriteDone==kFALSE)&&(delayedInitWrite()==kFALSE))
     { throw ; }
    // [David] presumably useless
    //m_fileWrite->cd() ;
    std::vector<TTree*>::iterator treeItr ;
    Long64_t numBytes ;
    for ( treeItr=m_treeCol.begin() ; treeItr != m_treeCol.end() ; treeItr++ )
     { numBytes = (*treeItr)->LoadTree(m_eventCounter) ; }
    m_celWrite->fillEvent(m_treeCol) ;
    
    // [David] presumably useless
    //saveDir->cd() ;
    ++m_eventCounter ;
   }
  catch(...)
   {
    std::cerr << "Error filling CEL ROOT file" << std::endl ;
    std::cerr.flush() ;
    throw ;
   }
  return stat ;
 }

Bool_t CelManager::fillFileAndTreeSet()
 {
  Bool_t stat = kTRUE ;
  try
   {
    if (m_initWriteDone==kFALSE)
     {
      std::cout << "WARNING in final write to CEL ROOT file : no event registered" << std::endl ; 
      if (delayedInitWrite()==kFALSE)
       { throw ; }
     }

    TDirectory * saveDir = gDirectory ;
    // [David] I guess this cd() is useless ?
    //m_fileWrite->cd() ;
    m_celWrite->fillFileAndTreeSet() ;
    m_celWrite->writeAndClose() ;
    delete m_celWrite ;
    m_celWrite = 0 ;
    saveDir->cd() ;
   }
  catch(...)
   {
    std::cerr << "Failed final write to CEL ROOT file" << std::endl ; 
    throw ;
   }
  return stat ;
 }



//========================================================================
//
// Reading Methods
//
//========================================================================


Bool_t CelManager::initRead( const TString & celFileName )
 {
  Bool_t stat = kTRUE ;
  
  // opening
  m_fileNameRead = celFileName ;
  m_celRead = new CompositeEventList(m_fileNameRead) ;
  if (m_celRead->isOk()==kFALSE)
   {
    std::cerr
     << "[CelManager::initRead] "
     << "Error while opening "
     << m_fileNameRead << std::endl ;
    return stat ;
   }

  // Set up our master TChain and the component TChains
  if (!m_compChainCol) m_compChainCol = new TObjArray() ;
  m_masterChain = m_celRead->newChains(m_compChainCol) ;
  TIter itr(m_compChainCol) ;
  TChain * curChain ;
  while ( (curChain = (TChain*)itr.Next()) )
   {
    curChain->SetBranchStatus("*", 1) ;
    m_chainIndexCol[curChain->GetName()] = curChain->GetTreeIndex() ;
   }
  return stat ;
 }

Long64_t CelManager::getNumEvents()
 { return (m_celRead->numEvents()) ; }

Long64_t CelManager::getEventIndexInTree( const TString & treeName, Long64_t index)
 {
  // make sure the cel index is the active TVirtualIndex
  setIndex() ;
  Long64_t retVal = m_masterChain->LoadTree(index) ;  // Causes all component TChains to be loaded correctly via CelIndex
  if (retVal<0) return retVal ;
  TChain * compChain = getChainByType(treeName) ;
  if (!compChain) return -1 ;
  // [David] before 5.16, there is a bug on TChain::GetReadEntry()
  // That'why I am working on trees
  return (compChain->GetTree()->GetReadEntry()) ;
 }

int CelManager::setIndex()
 {
  int numSet=0 ;
  std::map<TString, TVirtualIndex*>::iterator mapIt ;
  for ( mapIt=m_chainIndexCol.begin() ; mapIt != m_chainIndexCol.end() ; mapIt++ )
   {
    TChain * curChain = getChainByType(mapIt->first) ;
    if (curChain)
     {
      curChain->SetTreeIndex(mapIt->second) ;
      numSet++ ;
     }
   }
  return numSet ;
 }

TChain * CelManager::getChainByType( const TString & treeName )
 {
  TIter itr(m_compChainCol) ;
  TChain * curChain ;
  while ((curChain = (TChain*)itr.Next()))
   { if (curChain->GetName() == treeName) return curChain ; }
  return 0 ;
 }



