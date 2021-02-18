$PSScriptRoot
Push-Location -Path $PSScriptRoot/..

New-Item -ItemType Directory -Force -Path .build
New-Item -ItemType Directory -Force -Path .prefix

Push-Location .build
& cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=../.prefix ../uhr/cpp
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& nmake
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& nmake install
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
