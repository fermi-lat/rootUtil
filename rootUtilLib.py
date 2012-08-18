# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rootUtil/rootUtilLib.py,v 1.3 2008/10/27 17:49:11 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['rootUtil'])
        if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
	    env.Tool('findPkgPath', package = 'rootUtil') 
    env.Tool('addLibrary', library = env['rootLibs'])
    env.Tool('addLibrary', library = env['minuitLibs'])
    env.Tool('addLibrary', library = env['rootGuiLibs'])
    if env['PLATFORM']=='win32' and env.get('CONTAINERNAME','')=='GlastRelease':
        env.Tool('findPkgPath', package = 'facilities') 
def exists(env):
    return 1;
