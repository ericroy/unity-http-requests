ROOT=..\..
OUTPUT_DIR=$(ROOT)\..\Runtime\bin
OUTPUT_BINARY=$(OUTPUT_DIR)/http_requests.win64.dll

SRC_FILES= \
  $(ROOT)\src\win64/http_requests.cc \
  $(ROOT)\src\win64/http_context.cc

all: $(OUTPUT_BINARY)

$(OUTPUT_BINARY): $(SRC_FILES)
  cl /LD /MT /EHsc /Fe$(OUTPUT_BINARY) /I$(ROOT)\include /I$(ROOT)\src $(SRC_FILES)
