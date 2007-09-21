/** @file LinkDef.h
    @brief for rootcint
 $Header: /nfs/slac/g/glast/ground/cvs/rootUtil/rootUtil/LinkDef.h,v 1.5 2007/09/19 16:57:05 chamont Exp $

*/
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class DataHandleBase;
#pragma link C++ class BranchGroup;

#pragma link C++ class DataHandle<Bool_t>;
#pragma link C++ class DataHandle<Char_t>;
#pragma link C++ class DataHandle<UChar_t>;
#pragma link C++ class DataHandle<UShort_t>;
#pragma link C++ class DataHandle<Short_t>;
#pragma link C++ class DataHandle<UShort_t>;
#pragma link C++ class DataHandle<Int_t>;
#pragma link C++ class DataHandle<UInt_t>;
#pragma link C++ class DataHandle<Long64_t>;
#pragma link C++ class DataHandle<ULong64_t>;
#pragma link C++ class DataHandle<Float_t>;
#pragma link C++ class DataHandle<Double_t>;

#pragma link C++ class CelEntryIndex;
#pragma link C++ class CelFileAndTreeSet;
#pragma link C++ class CelEventLink;
#pragma link C++ class CelEventComponent;
#pragma link C++ class CompositeEventList;
#pragma link C++ class CelIndex;

#pragma link C++ namespace CelUtil;

#endif
