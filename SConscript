# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rootUtil/SConscript,v 1.6 2009/01/23 00:07:53 ecephas Exp $ 
# Authors: David Chamont <chamont@llr.in2p3.fr>
# Version: rootUtil-01-04-00
import os
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('rootUtilLib', depsOnly = 1)

rootUtilRootcint = libEnv.Rootcint('rootUtil/rootUtil_rootcint',
                                   ['rootUtil/BranchGroup.h','rootUtil/BgDataHandleBase.h',
                                    'rootUtil/BgDataHandle.h', 'rootUtil/CompositeEventList.h',
                                    'rootUtil/CelEventComponent.h','rootUtil/CelEventEntry.h',
                                    'rootUtil/CelFileAndTreeSet.h','rootUtil/CelFileAndTreeOffset.h','rootUtil/CelEventIDs.h',
                                    'rootUtil/CelEventLink.h', 'rootUtil/CelUtil.h', 'rootUtil/ComponentsInfo.h',
                                    'rootUtil/TestData.h','rootUtil/LinkDef.h'],
		includes = [''])
libEnv['rootcint_node'] = rootUtilRootcint
 
rootUtil = libEnv.SharedLibrary('rootUtil', listFiles(['src/*.cxx']) + ['rootUtil/rootUtil_rootcint.cxx'])

progEnv.Tool('rootUtilLib')
testRootInterface = progEnv.Program('testRootInterface',
                                    ['apps/testRootInterface.cxx'])
celRelocate = progEnv.Program('celRelocate',['apps/celRelocate.cxx'])
celConvert = progEnv.Program('celConvert',['apps/celConvert.cxx'])
celInspect = progEnv.Program('celInspect',['apps/celInspect.cxx'])
digiMeritRecon = progEnv.Program('digiMeritRecon',['apps/digiMeritRecon.cxx'])
mergeSkim = progEnv.Program('mergeSkim',['apps/mergeSkim.cxx'])
testGenerateData = progEnv.Program('testGenerateData',['apps/testGenerateData.cxx'])
testMakeCel = progEnv.Program('testMakeCel',['apps/testMakeCel.cxx'])
testReadCel = progEnv.Program('testReadCel',['apps/testReadCel.cxx'])


progEnv.Tool('registerTargets', package = 'rootUtil',
             rootcintSharedCxts = [[rootUtil, libEnv]], 
	testAppCxts = [[testMakeCel,progEnv], [testReadCel, progEnv],
                       [testGenerateData, progEnv],  [testRootInterface, progEnv]],
	binaryCxts = [[celRelocate,progEnv], [celConvert, progEnv],
                      [celInspect, progEnv], [digiMeritRecon,progEnv],
                      [mergeSkim,progEnv]],
	includes = listFiles(['rootUtil/*.h']))








