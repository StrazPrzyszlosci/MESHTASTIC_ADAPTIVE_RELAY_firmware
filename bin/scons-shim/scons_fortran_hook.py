# SCons FortranCommon fallback, loaded via sitecustomize.py in this
# directory (see nested_pythonpath.py): SCons >= 4.9 removed
# SCons.Tool.FortranCommon and SCons.Scanner.Fortran, which ESP-IDF-era
# pioarduino build scripts still import on ESP32 hybrid builds under
# platformio 6.2.x (tool-scons >= 4.9 packaging). The two fallback modules
# shipped next to this file are verbatim SCons 4.8.1 sources (MIT,
# Copyright The SCons Foundation). The finder only kicks in when normal
# resolution fails, so installations with SCons < 4.9 are unaffected.
import importlib.abc
import importlib.util
import os
import sys

_PATCH_DIR = os.path.dirname(os.path.abspath(__file__))
_FALLBACKS = {
    "SCons.Tool.FortranCommon": "SCons-4.8.1-FortranCommon.py",
    "SCons.Scanner.Fortran": "SCons-4.8.1-ScannerFortran.py",
}


class _PatchFileLoader(importlib.abc.Loader):
    def __init__(self, path, package):
        self._path = path
        self._package = package

    def create_module(self, spec):
        return None

    def exec_module(self, module):
        module.__package__ = self._package
        with open(self._path) as f:
            code = compile(f.read(), self._path, "exec")
        exec(code, module.__dict__)


class _SConsPatchFinder(importlib.abc.MetaPathFinder):
    def find_spec(self, fullname, path=None, target=None):
        fname = _FALLBACKS.get(fullname)
        if fname is None:
            return None
        p = os.path.join(_PATCH_DIR, fname)
        if not os.path.isfile(p):
            return None
        package = fullname.rsplit(".", 1)[0]
        return importlib.util.spec_from_loader(fullname, _PatchFileLoader(p, package))


try:
    sys.meta_path.append(_SConsPatchFinder())
except Exception:
    pass
