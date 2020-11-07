ROOT=..\..
OUTPUT_DIR=$(ROOT)\..\Runtime\bin
OUTPUT_BINARY=$(OUTPUT_DIR)\http_requests.win64.dll
TEMP_PATTERNS=*.obj *.tlh *.tli

SRC_FILES= \
  $(ROOT)\src\win64\http_requests.cc \
  $(ROOT)\src\win64\http_context.cc

all: http_requests

http_requests: $(OUTPUT_BINARY)

clean:
  del /Q /F $(TEMP_PATTERNS) $(OUTPUT_BINARY) 2>NUL

$(OUTPUT_BINARY): $(SRC_FILES)
  cl /LD /MT /EHsc /Fe$(OUTPUT_BINARY) /DUHR_EXPORT /I$(ROOT)\include /I$(ROOT)\src $(SRC_FILES)
  del /Q /F $(TEMP_PATTERNS) 2>NUL