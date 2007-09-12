// -*- Mode: c++ -*-
#ifndef PointerUtil_cxx
#define PointerUtil_cxx
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: PointerUtil.cxx,v 1.2 2007/08/08 13:50:02 heather Exp $
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
#include "rootUtil/PointerUtil.h"

// c++/stl headers
#include <iostream>

// ROOT Headers
#include <TTree.h>
#include <TChain.h>
#include <TFile.h>
#include <TObjArray.h>

// Other headers from this package
#include "rootUtil/Component.h"
#include "rootUtil/PointerSkim.h"
#include "rootUtil/FileUtil.h"


PointerSkim* PointerUtil::mergeSkimsFromFiles(TCollection& skimFiles, const char* fileName, const char* option) {
  // Merge a Collection of input files into a single pointer skim

  TList skims;
  TIterator* itr = skimFiles.MakeIterator();
  TObject* aFileObj(0);
  while ( (aFileObj = itr->Next()) != 0 ) {
    TFile* aFile = dynamic_cast<TFile*>(aFileObj);
    if ( aFile == 0 ) {
      std::cerr << "Input object not a file " << aFileObj->GetName() << std::endl;
      return 0;
    }

    // Open the skim
    PointerSkim* nextSkim = new PointerSkim;
    TFile* check = nextSkim->openFile(aFile->GetName());
    if ( check == 0 ) {
      std::cerr << "Failed to open skim file " << aFile->GetName() << std::endl;
      return 0;
    }
    // Append it to the list of skims
    skims.AddLast(nextSkim);
  }
  // Call the other mergre method
  return mergeSkims(skims,fileName,option);
}


PointerSkim* PointerUtil::mergeSkims(TCollection& skims, const char* fileName, const char* option) {
  // Merge a Collection of pointer skims


  TList eventTreeList;
  TList fileTreeList;

  // Create the output File if requested
  TFile* outFile(0);
  if ( fileName != 0 ) {
    outFile = TFile::Open(fileName,option);
    if ( 0 == outFile ) {
      std::cerr << "Failed to open output file " << fileName << std::endl;
      return 0;
    } else {
      std::cout << "Opened output file " << outFile->GetName() << std::endl;
    }
  }

  // The Link Tree has to be re-done
  Long64_t* eventIndex = new Long64_t(-1);
  Long64_t* metaIndexOut  = new Long64_t(-1);
  Long64_t* metaOffset = new Long64_t(0);
  TTree* linkTreeOut = new TTree("Links","Links");
  linkTreeOut->Branch("Link_EventIndex",(void*)eventIndex,"Link_EventIndex/L");
  linkTreeOut->Branch("Link_MetaIndex",(void*)metaIndexOut,"Link_MetaIndex/L");
  linkTreeOut->Branch("Link_MetaOffset",(void*)metaOffset,"Link_MetaOffset/L");

  // Loop on skims
  TIterator* itr = skims.MakeIterator();
  TObject* aSkimObj(0);
  while ( (aSkimObj = itr->Next()) != 0 ) {
    PointerSkim* aSkim = dynamic_cast<PointerSkim*>(aSkimObj);
    if ( 0 == aSkim ) {
      std::cerr << "Input object not a PointerSkim " << aSkimObj->GetName() << std::endl;
      return 0;
    }    
    
    // Add the event & file trees to their respective lists
    eventTreeList.AddLast(aSkim->eventTree());
    fileTreeList.AddLast(aSkim->fileTree());    

    Long64_t metaIndexIn(0);
    Long64_t metaIndexSave(-1);
    TTree* linkTree = aSkim->linkTree();
    linkTree->SetBranchAddress("Link_MetaIndex",&metaIndexIn);

    // Redo the data in the Link tree
    Long64_t nEvtCurrent = linkTree->GetEntries();
    for ( Long64_t iEvt(0); iEvt < nEvtCurrent; iEvt++ ) {
      Int_t nB = linkTree->GetEntry(iEvt);
      if ( nB < 0 ) {
	// There was a problem, clean up
	std::cerr << "Failed to read entry in input link tree " << std::endl;
	if ( outFile != 0 ) {
	  outFile->Close();
	  delete outFile;
	}
	delete eventIndex;
	delete metaIndexOut;
	delete metaOffset;
	delete itr;
	delete linkTree;
	return 0;
      }
      // Event Counter goes up by one
      *eventIndex += 1;
      if ( metaIndexIn != metaIndexSave ) {
	// new meta data entry, latch values
	metaIndexSave = metaIndexIn;
	*metaIndexOut += 1;
	*metaOffset = *eventIndex;
      }
      // Fill this entry in the link tree
      linkTreeOut->Fill();
    }

  }
  delete itr;
  delete eventIndex;
  delete metaIndexOut;
  delete metaOffset;

  // Just Merge the Event and File trees the Normal way
  TTree* eventTree = TTree::MergeTrees(&eventTreeList);
  TTree* fileTree = TTree::MergeTrees(&fileTreeList);

  // Build the Pointer Skim Object
  PointerSkim* theSkim = new PointerSkim(*eventTree,*linkTreeOut,*fileTree);

  // Write the Merged File, if requested
  if ( outFile != 0 ) {
    outFile->Write();
    outFile->Close();
  }
  return theSkim;  
}


#endif
