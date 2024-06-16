import sys
import app
import dbg

sys.path.append("lib")

class TraceFile:
	def write(self, msg):
		dbg.Trace(msg)

class TraceErrorFile:
	def write(self, msg):
		dbg.TraceError(msg)
		dbg.RegisterExceptionString(msg)

class LogBoxFile:
	def __init__(self):
		self.stderrSave = sys.stderr
		self.msg = ""

	def __del__(self):
		self.restore()

	def restore(self):
		sys.stderr = self.stderrSave

	def write(self, msg):
		self.msg = self.msg + msg

	def show(self):
		dbg.LogBox(self.msg,"Error")

sys.stdout = TraceFile()
sys.stderr = TraceErrorFile()

#
# pack file support (must move to system.py, systemrelease.pyc)
#

import marshal
import imp
import pack

class pack_file_iterator(object):
	def __init__(self, packfile):
		self.pack_file = packfile
		
	def next(self):
		tmp = self.pack_file.readline()
		if tmp:
			return tmp
		raise StopIteration

_chr = __builtins__.chr

class pack_file(object):

	def __init__(self, filename, mode = 'rb'):
		assert mode in ('r', 'rb')
		if not pack.exist(filename):
			raise IOError, 'No file or directory'
		self.data = pack.get(filename)
		if mode == 'r':
			self.data=_chr(10).join(self.data.split(_chr(13)+_chr(10)))

	def __iter__(self):
		return pack_file_iterator(self)

	def read(self, len = None):
		if not self.data:
			return ''
		if len:
			tmp = self.data[:len]
			self.data = self.data[len:]
			return tmp
		else:
			tmp = self.data
			self.data = ''
			return tmp

	def readline(self):
		return self.read(self.data.find(_chr(10))+1)

	def readlines(self):
		return [x for x in self]

__builtins__.pack_open = pack_open = pack_file


old_import = __import__
import sys
import imp
from types import ModuleType as _ModuleType

def _process_result(code, fqname):
	is_module = isinstance(code, _ModuleType)

	module = code if is_module else imp.new_module(fqname)

	sys.modules[fqname] = module

	if not is_module:
		exec code in module.__dict__

	module = sys.modules.get(fqname, None)
	if module:
		module.__name__ = fqname

	return module


module_do = lambda x:None

def __pack_import(name, globals=None, locals=None, fromlist=None, level=0): # Dodano argument 'level' z wartością domyślną

	if name in sys.modules:
		return sys.modules[name]

	filename = name + '.py'

	if pack.exist(filename):
		dbg.Trace('importing from pack %s\\n' % name)

		newmodule = _process_result(compile(pack_file(filename,'r').read(),filename,'exec'),name)

		module_do(newmodule)
		return newmodule
		#return imp.load_module(name, pack_file(filename,'r'),filename,('.py','r',imp.PY_SOURCE))
	else:
		dbg.Trace('importing from lib %s\\n' % name)
		return old_import(name,globals,locals,fromlist)

def splitext(p):
	root, ext = '', ''
	for c in p:
		if c in ['/']:
			root, ext = root + ext + c, ''
		elif c == '.':
			if ext:
				root, ext = root + ext, c
			else:
				ext = c
		elif ext:
			ext = ext + c
		else:
			root = root + c
	return root, ext

class PythonExecutioner: 

	def Run(kPESelf, sFileName, kDict): 
		if kPESelf.__IsCompiledFile__(sFileName): 
			kCode=kPESelf.__LoadCompiledFile__(sFileName) 
		else: 
			kCode=kPESelf.__LoadTextFile__(sFileName) 

		exec(kCode, kDict) 

	def __IsCompiledFile__(kPESelf, sFileName): 

		sBase, sExt = splitext(sFileName) 
		sExt=sExt.lower() 

		if sExt==".pyc" or sExt==".pyo": 
			return 1 
		else: 
			return 0 

	def __LoadTextFile__(kPESelf, sFileName): 
		sText=pack_open(sFileName,'r').read() 
		return compile(sText, sFileName, "exec") 

	def __LoadCompiledFile__(kPESelf, sFileName): 
		kFile=pack_open(sFileName)

		if kFile.read(4)!=imp.get_magic(): 
			raise 

		kFile.read(4) 

		kData=kFile.read() 
		return marshal.loads(kData) 

def execfile(fileName, dict): 
	kPE=PythonExecutioner() 
	kPE.Run(fileName, dict) 

def exec_add_module_do(mod):
	global execfile
	mod.__dict__['execfile'] = execfile

import __builtin__
__builtin__.__import__ = __pack_import
module_do = exec_add_module_do

"""
#
# PSYCO installation (must move to system.py, systemrelease.pyc)
#
try:
	import psyco
	#from psyco.classes import *

	def bind_me(bindable_list):
		try:
			for x in bindable_list:
				try:
					psyco.bind(x)
				except:
					pass
		except:
			pass		

	_prev_psyco_old_module_do = module_do
	def module_bind(module):
		_prev_psyco_old_module_do(module)
		#print 'start binding' + str(module)
		try:
			psyco.bind(module)
		except:
			pass
		for x in module.__dict__.itervalues():
			try:
				psyco.bind(x)
			except:
				pass		
		#print 'end binding'

	dbg.Trace("PSYCO installed\\n")

except Exception, msg:
	bind_me = lambda x:None
	dbg.Trace("No PSYCO support : %s\\n" % msg)
"""

def GetExceptionString(excTitle):
	(excType, excMsg, excTraceBack)=sys.exc_info()
	excText=""
	excText+=_chr(10)

	import traceback
	traceLineList=traceback.extract_tb(excTraceBack)

	for traceLine in traceLineList:
		if traceLine[3]:
			excText+="%s(line:%d) %s - %s" % (traceLine[0], traceLine[1], traceLine[2], traceLine[3])
		else:
			excText+="%s(line:%d) %s"  % (traceLine[0], traceLine[1], traceLine[2])

		excText+=_chr(10)
	
	excText+=_chr(10)
	excText+="%s - %s:%s" % (excTitle, excType, excMsg)		
	excText+=_chr(10)

	return excText

def ShowException(excTitle):
	excText=GetExceptionString(excTitle)
	dbg.TraceError(excText)
	app.Abort()

	return 0

def RunMainScript(name):
	try:		
		execfile(name, __main__.__dict__)
	except RuntimeError, msg:
		msg = str(msg)

		import localeInfo
		if localeInfo.error:
			msg = localeInfo.error.get(msg, msg)

		dbg.LogBox(msg)
		app.Abort()

	except:	
		msg = GetExceptionString("Run")
		dbg.LogBox(msg)
		app.Abort()
	
import debugInfo
debugInfo.SetDebugMode(__DEBUG__)

class DBG:
	# States
	STATE_SYS_ERR, STATE_CONSOLE, STATE_LOGBOX = range(0, 3)

	# Functionss
	FUNC_ADRESS_DICT = {
		STATE_SYS_ERR : lambda arg : dbg.TraceError(arg),
		STATE_CONSOLE : lambda arg : dbg.Tracen(arg),
		STATE_LOGBOX : lambda arg : dbg.LogBox(arg),
	}

	@staticmethod
	def Execute(state, lines):
		""" A execute method which allow you to send unlimited arguments lines for debugging. """
		# If the state is console (Tracef), check if compiled executable is in debug-mode or not.
		isDebugMode = True if not __DEBUG__ else False
		if state == DBG.STATE_CONSOLE and not isDebugMode:
			return

		# If there's just one argument, convert it into a tuple.
		if not isinstance(lines, (tuple, list)):
			lines = tuple([lines])

		for line in lines:
			execute = DBG.FUNC_ADRESS_DICT.get(state, DBG.STATE_SYS_ERR)
			# By using custom string formatting it takes a format string and an arbitrary set of positional and keyword arguments.
			# Convert all data types into a specific string.
			execute('{}'.format(line))

__builtin__.TraceError = lambda *args : DBG.Execute(DBG.STATE_SYS_ERR, args)
__builtin__.Tracef = lambda *args : DBG.Execute(DBG.STATE_CONSOLE, args)
__builtin__.LogBox = lambda *args : DBG.Execute(DBG.STATE_LOGBOX, args)
__builtin__.sys_err = __builtin__.TraceError

RunMainScript("prototype.py")

