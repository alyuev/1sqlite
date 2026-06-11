$ErrorActionPreference='Stop'
$proj=$PSScriptRoot; $root=Split-Path $proj
$vc=Join-Path $root 'tools\MSVC600\VC98'; $common=Join-Path $root 'tools\MSVC600\Common'
$obj=Join-Path $proj 'obj'
$env:PATH="$vc\Bin;$common\MSDev98\Bin;$env:PATH"
$compat=Join-Path $root 'compat'
$env:INCLUDE="$vc\Include;$compat;$proj"
$env:LIB="$vc\Lib"
New-Item -ItemType Directory -Force $obj | Out-Null
New-Item -ItemType Directory -Force (Join-Path $proj 'build') | Out-Null
Push-Location $proj
Write-Host "=== CL spellfix.c ==="
& "$vc\Bin\CL.EXE" /nologo /c /TC /MD /O2 /DWIN32 /DNDEBUG /FIvc6compat.h "/Fo$obj\spellfix.obj" spellfix.c 2>&1 |
  Select-String 'error|fatal' | Select-Object -First 20 | ForEach-Object{Write-Host $_.Line}
if($LASTEXITCODE -ne 0){ Pop-Location; Write-Host "COMPILE FAILED ($LASTEXITCODE)"; exit 1 }
$sys=@('kernel32.lib','user32.lib')
Write-Host "=== LINK ==="
& "$vc\Bin\LINK.EXE" /nologo /DLL '/EXPORT:sqlite3_spellfix_init' "/OUT:build\spellfix.dll" "$obj\spellfix.obj" $sys | ForEach-Object{Write-Host $_}
Pop-Location
if(Test-Path "$proj\build\spellfix.dll"){ Write-Host ("OK spellfix.dll "+[int]((Get-Item "$proj\build\spellfix.dll").Length/1KB)+" KB") }
