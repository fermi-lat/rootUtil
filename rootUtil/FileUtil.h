// -*- Mode: c++ -*-
#ifndef FileUtil_h
#define FileUtil_h
/*
* Project: GLAST
* Package: rootUtil
*    File: $Id: FileUtil.h,v 1.1 2007/08/08 13:50:02 heather Exp $
* Authors:
*   EC, Eric Charles,    SLAC              echarles@slac.stanford.edu
*
* Copyright (c) 2007
*                   Regents of Stanford University. All rights reserved.
*
*
*
*/

// ROOT Headers
#include <Rtypes.h>

// forward declares
class TTree;
class TFile;


namespace FileUtil {
  
  // use OxFFFF as a guard value to persist NULL pointers
  const UShort_t NOKEY(0xFFFF); 

  // Open a file.  Return NULL if failed
  TFile* openFile(const char* fileName);

  // Get the TFile associated to a TTree.  Return NULL if failed
  TFile* getFile(TTree& tree);
  
}

#endif
