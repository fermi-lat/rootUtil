# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rootUtil/rootUtilLib.py,v 1.1 2008/08/15 21:42:39 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['rootUtil'])
    env.Tool('addLibrary', library = env['rootGuiLibs'])
    env.Tool('addLibrary', library = env['minuitLibs'])
def exists(env):
    return 1;
