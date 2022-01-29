#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd "$here/.."

# UHR_TESTS_OS:  One of windows, mac, linux, ios, android

# Use an appropriate default for UHR_TESTS_OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    test_os=${UHR_TESTS_OS:-linux}
elif [[ "$OSTYPE" == "darwin"* ]]; then
    test_os=${UHR_TESTS_OS:-mac}
elif [[ "$OSTYPE" == "msys" ]]; then
    test_os=${UHR_TESTS_OS:-windows}
else
    test_os=${UHR_TESTS_OS:-}
fi

# Based on the test_os, decide on a c# constant to set
if [[ "$test_os" == "windows" ]]; then
    constants="UHR_TESTS_WINDOWS"
elif [[ "$test_os" == "linux" ]]; then
    constants="UHR_TESTS_LINUX"
elif [[ "$test_os" == "mac" ]]; then
    constants="UHR_TESTS_MAC"
elif [[ "$test_os" == "ios" ]]; then
    constants="UHR_TESTS_IOS"
elif [[ "$test_os" == "android" ]]; then
    constants="UHR_TESTS_ANDROID"
else
    echo "UHR_TESTS_OS must be one of: windows, mac, linux, ios, android"
    exit 1
fi

config=Debug

dotnet restore unity/uhr_tests.csproj
dotnet build unity/uhr_tests.csproj \
    --configuration "$config" \
    --no-incremental \
    --no-restore \
    "-p:Constants=$constants"
dotnet test unity/uhr_tests.csproj \
    --configuration "$config" \
    --no-build \
    --verbosity normal
