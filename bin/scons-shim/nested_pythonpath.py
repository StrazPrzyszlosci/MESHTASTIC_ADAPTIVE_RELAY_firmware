#!/usr/bin/env python3
# trunk-ignore-all(ruff/F821)
# trunk-ignore-all(flake8/F821): For SConstruct imports
Import("env")

# pioarduino ESP32 hybrid builds import SCons.Tool.FortranCommon /
# SCons.Scanner.Fortran (removed in SCons >= 4.9) at the ELF link stage,
# so under PlatformIO 6.2.x every ESP32 build dies with
# "ModuleNotFoundError: No module named 'SCons.Tool.FortranCommon'".
# bin/scons-shim/ carries a meta-path finder that serves those two
# module names from vendored SCons 4.8.1 sources.
#
# This pre script runs inside every PlatformIO-generated SCons process
# of this project (including the nested ESP-IDF / Arduino child runs),
# before any framework builder is loaded. Two scopes:
#  1. the current process: register the finder on sys.meta_path now
#     (a PYTHONPATH set at this point would not affect an interpreter
#     that already started);
#  2. child processes (pioarduino re-invokes `pio run` and SCons for the
#     nested environments): prepend the shim dir to PYTHONPATH, so their
#     site machinery imports sitecustomize.py from it at startup. PIO
#     core (proc.py) preserves pre-existing PYTHONPATH entries when it
#     composes child environments.
import importlib.util
import os

_shim_dir = os.path.join(str(env.subst("$PROJECT_DIR")), "bin", "scons-shim")


def _register_fortran_fallback(shim_dir):
    try:
        import SCons.Tool.FortranCommon  # noqa: F401

        return  # resolvable already (SCons < 4.9 or a shim already active)
    except ImportError:
        pass
    try:
        spec = importlib.util.spec_from_file_location("scons_fortran_hook", os.path.join(shim_dir, "scons_fortran_hook.py"))
        if spec and spec.loader:
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)  # appends its finder to sys.meta_path
    except Exception:
        pass


_register_fortran_fallback(_shim_dir)

_parts = [p for p in os.environ.get("PYTHONPATH", "").split(os.pathsep) if p]
if _shim_dir not in _parts:
    os.environ["PYTHONPATH"] = os.pathsep.join([_shim_dir] + _parts)
