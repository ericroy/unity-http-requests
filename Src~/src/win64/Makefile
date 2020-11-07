SRC_ROOT=..\..
OUTPUT_DIR=$(SRC_ROOT)\..\Assets\Plugins\Editor
OUTPUT_BINARY=$(OUTPUT_DIR)\UnityHttpRequests.dll
TEMP_PATTERNS=*.obj *.tlh *.tli

SRC_FILES= \
  $(SRC_ROOT)\src\win64\unity_http_requests.cc \
  $(SRC_ROOT)\src\win64\http_context.cc

all: UnityHttpRequests

UnityHttpRequests: $(OUTPUT_BINARY)

clean:
  del /Q /F $(TEMP_PATTERNS) $(OUTPUT_BINARY) 2>NUL

$(OUTPUT_BINARY): $(SRC_FILES)
  cl /LD /MT /EHsc /Fe$(OUTPUT_BINARY) /DUHR_DLL_EXPORT /I$(SRC_ROOT)\include /I$(SRC_ROOT)\src $(SRC_FILES) WinHttp.lib
  del /Q /F $(TEMP_PATTERNS) 2>NUL