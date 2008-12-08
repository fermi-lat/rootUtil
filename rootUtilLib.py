# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/rootUtil/rootUtilLib.py,v 1.2 2008/09/10 14:47:55 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['rootUtil'])
    env.Tool('addLibrary', library = env['rootLibs'])
    env.Tool('addLibrary', library = env['minuitLibs'])
    env.Tool('addLibrary', library = env['rootGuiLibs'])
def exists(env):
    return 1;
