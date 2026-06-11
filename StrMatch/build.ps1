$ErrorActionPreference='Stop'
$proj=$PSScriptRoot; $root=Split-Path $proj
$tools=Join-Path $root 'tools\MSVC600'; $vc=Join-Path $tools 'VC98'; $common=Join-Path $tools 'Common'
$onec=Join-Path $root '_1Common'; $sqldbf=Join-Path $root 'SQL_DBF'; $obj=Join-Path $proj 'obj'
$env:PATH="$vc\Bin;$common\MSDev98\Bin;$env:PATH"
$env:INCLUDE="$vc\ATL\Include;$vc\Include;$vc\MFC\Include;$onec;$onec\1CHEADERS;$sqldbf;$proj"
$env:LIB="$vc\Lib;$vc\MFC\Lib;$onec\1CHEADERS\LIBS"
New-Item -ItemType Directory -Force $obj | Out-Null
New-Item -ItemType Directory -Force (Join-Path $proj 'build') | Out-Null
$defs=@('/DWIN32','/DNDEBUG','/D_NDEBUG','/D_AFXEXT')
$cf=@('/nologo','/c','/MD','/GX','/GR','/O2','/Zm300')+$defs
Push-Location $proj
$srcs=@('StdAfx.cpp','StrMatch_core.cpp','StrMatchMain.cpp','strmatch_reg.cpp','strmatch_ext.cpp')
$fail=0
foreach($s in $srcs){
  $o=Join-Path $obj ([IO.Path]::GetFileNameWithoutExtension($s)+'.obj')
  Write-Host "=== CL $s ==="
  & "$vc\Bin\CL.EXE" @cf "/Fo$o" $s | Out-Null
  $rc=$LASTEXITCODE
  if($rc -ne 0){ & "$vc\Bin\CL.EXE" @cf "/Fo$o" $s | Select-String 'error|fatal' | Select-Object -First 5 | ForEach-Object{Write-Host $_.Line}; Write-Host "FAILED $s ($rc)"; $fail++ }
}
if($fail){ Pop-Location; Write-Host "COMPILE ERRORS: $fail"; exit 1 }
Write-Host "=== RC strmatch.rc ==="
& "$common\MSDev98\Bin\RC.EXE" /fo build\strmatch.res strmatch.rc | ForEach-Object{Write-Host $_}
if($LASTEXITCODE -ne 0){ Pop-Location; Write-Host "RC FAILED"; exit 1 }
$objs=Get-ChildItem (Join-Path $obj '*.obj')|ForEach-Object{"obj\$($_.Name)"}
$sys=@('kernel32.lib','user32.lib','gdi32.lib','advapi32.lib','ole32.lib','oleaut32.lib','uuid.lib','comctl32.lib','version.lib')
Write-Host "=== LINK ==="
& "$vc\Bin\LINK.EXE" /nologo /DLL /subsystem:windows /IGNORE:4089 /OPT:NOREF '/EXPORT:sqlite3_strmatch_init' '/OUT:build\strmatch.dll' $objs 'build\strmatch.res' $sys | ForEach-Object{Write-Host $_}
Pop-Location
if(Test-Path "$proj\build\strmatch.dll"){ Write-Host ("OK strmatch.dll " + [int]((Get-Item "$proj\build\strmatch.dll").Length/1KB) + " KB") }
