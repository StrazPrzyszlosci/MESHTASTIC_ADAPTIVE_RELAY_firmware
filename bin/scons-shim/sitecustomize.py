# Auto-imported by Python's site machinery at interpreter startup: this
# directory is prepended to PYTHONPATH by ../nested_pythonpath.py (a
# PlatformIO extra script), and platformio/proc.py preserves existing
# PYTHONPATH entries when it composes the environment for every child
# process of a hybrid build. Registers a meta-path finder that serves
# SCons.Tool.FortranCommon / SCons.Scanner.Fortran (removed in SCons >=
# 4.9, still imported by ESP-IDF-era pioarduino build scripts) from the
# vendored SCons 4.8.1 modules in this directory - only when normal
# import fails, so nothing else is affected.
try:
    import scons_fortran_hook  # noqa: F401
except Exception:
    pass
