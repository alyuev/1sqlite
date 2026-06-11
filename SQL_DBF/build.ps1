# build.ps1 -- build 1sqlite.dll with portable VC6 (no VCVARS32, Unicode-safe).
# ASCII-only on purpose: WinPS 5.1 reads BOM-less .ps1 as ANSI and mangles non-ASCII.
# Paths are derived from $PSScriptRoot so no Cyrillic literal is needed in this file.
# Usage:
#   powershell -File build.ps1            # compile all modules + link
#   powershell -File build.ps1 -Only utex # compile a single module
#   powershell -File build.ps1 -Fts5      # build the FTS5 variant (separate dll)
param([string]$Only = "", [switch]$Fts5)

$ErrorActionPreference = 'Stop'
$proj   = $PSScriptRoot                 # ...\SQL_DBF
$root   = Split-Path $proj             # ...\SQLite
$vc     = Join-Path $root 'tools\MSVC600\VC98'
$common = Join-Path $root 'tools\MSVC600\Common'
$zlib   = Join-Path $root 'zlib\zlib-1.3.1'
$onec   = Join-Path $root '_1Common'
$libs   = Join-Path $onec '1CHEADERS\LIBS'
$compat = Join-Path $root 'compat'       # stdint.h shim + vc6compat.h for modern SQLite
$obj    = Join-Path $proj 'obj'

$env:PATH    = "$vc\Bin;$common\MSDev98\Bin;$env:PATH"
$env:INCLUDE = "$vc\ATL\Include;$vc\Include;$vc\MFC\Include;$zlib;$proj;$compat"
$env:LIB     = "$vc\Lib;$vc\MFC\Lib;$libs"

New-Item -ItemType Directory -Force $obj | Out-Null

# Defines/flags mirror the Release config in 1sqlite.cbp
$defs = @(
  '/DWIN32','/DNDEBUG','/D_NDEBUG','/D_AFXEXT',
  '/DSQLITE_ENABLE_LOAD_EXTENSION=1','/DSQLITE_OMIT_DEPRECATED=1','/DSQLITE_THREADSAFE=0',
  '/DSQLITE_OMIT_AUTHORIZATION=1','/DSQLITE_OMIT_TCL_VARIABLE=1','/DSQLITE_TEMP_STORE=2',
  '/DSQLITE_ENABLE_EXPLAIN_COMMENTS=1','/DSQLITE_ENABLE_STAT4',
  '/DSQLITE_DISABLE_DIRSYNC','/DSQLITE_DEFAULT_MEMSTATUS=0',
  '/DSQLITE_ENABLE_MATH_FUNCTIONS',  # step2: x87 FPU math funcs — suspect for FormEx FP-state conflict
  # Тривиальные фичи, встроенные в амальгаму с 3.51 (вкл. только флагом, авто-регистрация):
  '/DSQLITE_ENABLE_PERCENTILE',      # median(), percentile(), percentile_cont(), percentile_disc()
  '/DSQLITE_ENABLE_CARRAY'           # carray() — передача массива значений в запрос
)
if( $Fts5 ) { $defs += '/DSQLITE_ENABLE_FTS5' }   # full-text search (separate variant, like the author's _FTS5 dll)
$cflags = @('/nologo','/c','/MD','/GX','/GR','/O2','/Zm200') + $defs

# Component modules (Unit list from .cbp) + dev_serv from _1Common
$cppFiles = @(
  'SQL_DBF.cpp','StdAfx.cpp','calcjournal.cpp','database.cpp','dataprovider.cpp',
  'docheaders.cpp','doctables.cpp','filtermachine.cpp','journal.cpp','longstrreader.cpp',
  'metaparser.cpp','phisicalinfo.cpp','referencetabinfo.cpp','register.cpp',
  'strategycash.cpp','utex.cpp','vtab_info.cpp'
)
# dev_serv.cpp is #included by StdAfx.cpp (see StdAfx.cpp), so it is NOT a separate unit.
$extraCpp = @()
$cFiles   = @('SQLite\sqlite3.c')

function Compile($src, $dstObj) {
  $a = $cflags + @("/Fo$dstObj", $src)
  # modern SQLite amalgamation needs the VC6 compat shim force-included
  if( $src -match 'sqlite3\.c$' ) { $a = @('/FIvc6compat.h') + $a }
  $out = & "$vc\Bin\CL.EXE" @a 2>&1
  $rc = $LASTEXITCODE
  $out | ForEach-Object { Write-Host $_ }
  return $rc
}

Push-Location $proj
$fail = 0
if ($Only) {
  $targets = @(($cppFiles + $cFiles + $extraCpp) | Where-Object { $_ -match [regex]::Escape($Only) })
  if (-not $targets) { $targets = @("$Only.cpp") }
} else {
  $targets = $cppFiles + $cFiles + $extraCpp
}

foreach ($src in $targets) {
  $name = [IO.Path]::GetFileNameWithoutExtension($src)
  $dstObj = Join-Path $obj "$name.obj"
  Write-Host "=== CL $src ===" -ForegroundColor Cyan
  $rc = Compile $src $dstObj
  if ($rc -ne 0) { Write-Host "  FAILED ($rc)" -ForegroundColor Red; $fail++ }
}
Pop-Location

Write-Host ""
if ($fail) { Write-Host "Compile errors: $fail module(s) failed." -ForegroundColor Red; exit 1 }
Write-Host "All requested modules compiled OK." -ForegroundColor Green

if ($Only) { return }

# ---- Link phase ----
# Link cwd MUST be the project dir: 1cheaders.inl pulls the 13 1C import libs via
# #pragma comment(lib, "../_1Common/1cheaders/libs/<x>.lib") -- paths relative to cwd.
# MFC (mfc42/mfcs42) and CRT (msvcrt) are pulled by MFC header pragmas under _AFXDLL.
$env:LIB = "$env:LIB;$zlib"          # so zlib.lib resolves
$syslibs = @('kernel32.lib','user32.lib','gdi32.lib','advapi32.lib','ole32.lib',
             'oleaut32.lib','uuid.lib','odbc32.lib','comctl32.lib','version.lib')
Push-Location $proj
New-Item -ItemType Directory -Force (Join-Path $proj 'build') | Out-Null

# Compile the version resource (SQL_DBF.rc -> build\SQL_DBF.res). cwd=proj so res\SQL_DBF.rc2 resolves.
$rc = "$common\MSDev98\Bin\RC.EXE"
Write-Host "=== RC SQL_DBF.rc ===" -ForegroundColor Cyan
$ro = & $rc /i "$vc\MFC\Include" /i "$vc\Include" /i "$proj" /fo 'build\SQL_DBF.res' 'SQL_DBF.rc' 2>&1
$rcrc = $LASTEXITCODE
$ro | ForEach-Object { Write-Host $_ }
if ($rcrc -ne 0) { Pop-Location; Write-Host "RC failed ($rcrc)." -ForegroundColor Red; exit 1 }

$objsRel = Get-ChildItem (Join-Path $obj '*.obj') | ForEach-Object { "obj\$($_.Name)" }
$outRel  = if( $Fts5 ) { 'build\1sqlite_built_fts5.dll' } else { 'build\1sqlite_built.dll' }
$linkArgs = @('/nologo','/DLL','/subsystem:windows','/IGNORE:4089',"/OUT:$outRel") + $objsRel + @('build\SQL_DBF.res','zlib.lib') + $syslibs
Write-Host "=== LINK $outRel ===" -ForegroundColor Cyan
$lo = & "$vc\Bin\LINK.EXE" @linkArgs 2>&1
$rc = $LASTEXITCODE
$lo | ForEach-Object { Write-Host $_ }
Pop-Location
if ($rc -ne 0) { Write-Host "LINK failed ($rc)." -ForegroundColor Red; exit 1 }
Write-Host "LINK OK -> $outRel" -ForegroundColor Green
