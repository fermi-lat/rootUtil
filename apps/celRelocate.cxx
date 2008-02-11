
#if !defined(__CINT__) || defined(__MAKECINT__)
#  include <TFile.h>
#  include <TTree.h>
#  include <TSystem.h>
#  include <TObjString.h>
#  include <TObjArray.h>
#  include <TMap.h>
#  include <cstdlib>
#  include <fstream>
#  include <iostream>
#  include <vector>
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include "apps/winutil/XGetopt.h"
#endif


void relocateInputFilePath( TObjString * inputFilePath )
 {
  TString path = inputFilePath->GetString() ;
  Ssiz_t find ;
  find = path.Last('/') ;
  if (find!=kNPOS) 
   { path.Remove(0,find+1) ; }
  inputFilePath->SetString(path) ;
 }

void relocateCelFile( const char * fileName )
 {
  std::cout << "==== " << fileName << std::endl ;
  TString oldFileName = fileName ;
  oldFileName += '~' ;
  TFile oldFile(oldFileName,"READ") ;
  TTree * oldEntryTree = (TTree *)oldFile.Get("EventEntries") ;
  TTree * oldLinkTree = (TTree *)oldFile.Get("EventLinks") ;
  TTree * oldFileTree = (TTree *)oldFile.Get("FileAndTreeSets") ;
  TTree * oldOffsetTree = (TTree *)oldFile.Get("FileAndTreeOffsets") ;
  
  TObjArray setFileNames ;
  TObjArray * componentFileNames = 0 ;
  TMap componentNames ;
  TObjString * componentName  = 0 ;
  TIter * fileIter = 0 ;
  TObjString * inputFilePath = 0 ;
  TObjArray * branches = oldFileTree->GetListOfBranches() ;
  TBranch * branch = 0 ;
  Ssiz_t find ;
  TString name ;
  Int_t i ;
  for ( i = 0 ; i < branches->GetEntries() ; i++ )
   {
    branch = (TBranch *)branches->UncheckedAt(i) ;
    name = branch->GetName() ;
    find = name.Index("_Set_FileNames") ;
    if (find==kNPOS) continue ;
    name.Remove(find) ;
    componentFileNames = new TObjArray ;
    setFileNames.Add(componentFileNames) ;
    componentName = new TObjString(name) ;
    componentNames.Add(componentFileNames,componentName) ;
    branch->SetAddress(&setFileNames[setFileNames.GetEntries()-1]) ;
   }

  TFile newFile(fileName,"RECREATE") ;
  TTree * newEntryTree = oldEntryTree->CloneTree(-1,"fast") ;
  TTree * newLinkTree = oldLinkTree->CloneTree(-1,"fast") ;
  TTree * newOffsetTree = oldOffsetTree->CloneTree(-1,"fast") ;  
  newEntryTree->Write() ;
  newLinkTree->Write() ;
  newOffsetTree->Write() ;
  TTree * newFileTree = oldFileTree->CloneTree(0,"fast") ;
  Long64_t iSet, nSets = oldFileTree->GetEntries() ;
  TIter componentIter(&setFileNames) ;
  for ( iSet = 0 ; iSet<nSets ; ++iSet )
   {
    oldFileTree->GetEntry(iSet) ;
    componentIter.Reset() ;
    while ((componentFileNames=((TObjArray *)componentIter.Next())))
     {
      componentName = (TObjString *)componentNames.GetValue(componentFileNames) ;
      //std::cout<<"== Set "<<iSet<<" / Component "<<componentName->GetString()<<std::endl ;
      fileIter = new TIter(componentFileNames) ;
      while ((inputFilePath=((TObjString *)fileIter->Next())))
       { relocateInputFilePath(inputFilePath) ; }
      delete fileIter ;
      fileIter = 0 ;
     }
    newFileTree->Fill() ;
   }
  newFileTree->Write() ;
  newFile.Close() ;
  oldFile.Close() ;
 }

Bool_t backupFile( const char * fileName )
 {
  TString name = fileName ;
  while (gSystem->AccessPathName(name)==kFALSE)
   { name += '~' ; }
  if (name==fileName)
   {
    std::cerr<<"[celRelocate.exe] cannot find "<<name<<std::endl ;
    return kFALSE ;
   }
  TString previous = name ;
  previous.Chop() ;
  Bool_t res ;
  while (name!=fileName)
   {
    res = TFile::Cp(previous,name,kFALSE) ;
    if (res==kFALSE)
     {
      std::cerr<<"[celRelocate.exe] cannot backup "<<previous<<std::endl ;
      return kFALSE ;
     }
    name.Chop() ;
    previous.Chop() ;
   }
  
  return kTRUE ;
 }

void usage()
 {
  std::string m_theApp("celRelocate.exe") ;

  std::cout << std::endl
       << m_theApp << std::endl
       << "Transform the input data file paths within the Composite Event Lists" << std::endl
       << std::endl ;
  
  std::cout << "Usage:" << std::endl
       << "\t" << m_theApp << " [options] CEL Files" << std::endl 
       << std::endl
       << std::endl
       << "\tOPTIONS for all jobs" << std::endl
       << "\t   -h                : print this message" << std::endl
       << std::endl ;
 }
  

int main(int argn, char** argc)
 {
# ifdef WIN32
    gSystem->Load("libTreePlayer.dll");
# endif

  std::string printFlags;

  //  options
  int opt;
  while ( (opt = getopt(argn, argc, "h")) != EOF )
   {
    switch (opt)
     {
    case 'h':   // help      
      usage() ;
      return 1 ;
    case '?':
      usage();
      return 2;
    default:
      std::cerr << opt << " not parsable..." << std::endl;
      return 2;      
     }
   }

  if ( argn - optind == 0 )
   {
    std::cerr << "celRelocate.exe" << " requires some CEL files" << std::endl
	 << "Try " << "celRelocate.exe" << " -h" << std::endl ;
    return 3 ;
   }

  for ( int idx = optind ; idx < argn ; idx++ )
   {
    if (backupFile(argc[idx])==kFALSE)
     { return 4 ; }
    relocateCelFile(argc[idx]) ;
   }

  return 0 ;
 }
