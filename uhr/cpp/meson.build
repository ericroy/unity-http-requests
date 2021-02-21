project('uhr', 'cpp',
  version: '0.1',
  default_options: [
    'prefix=' + (meson.current_source_dir() / 'build' / '.prefix'),
    'warning_level=3',
  ]
)

cmake = import('cmake')

mbedtls_options = cmake.subproject_options()
mbedtls_options.add_cmake_defines({
  'CMAKE_POSITION_INDEPENDENT_CODE': true,
  'ENABLE_TESTING': false,
  'ENABLE_PROGRAMS': false,
})
mbedtls_subproject = cmake.subproject('mbedtls', options: mbedtls_options)
message(mbedtls_subproject.target_list())
#mbedtls_include_dirs = include_directories('subprojects/mbedtls/include')

curl_options = cmake.subproject_options()
curl_options.add_cmake_defines({
  'CMAKE_POSITION_INDEPENDENT_CODE': true,
  'BUILD_SHARED_LIBS': false,
  'BUILD_CURL_EXE': false,
  'BUILD_TESTING': false,
  'HTTP_ONLY': true,
  'CMAKE_USE_LIBSSH2': false,
  'CMAKE_USE_OPENSSL': false,
  'CMAKE_USE_MBEDTLS': host_machine.system() != 'windows',
  'CMAKE_USE_SCHANNEL': host_machine.system() == 'windows',
})
curl_subproject = cmake.subproject('curl', options: curl_options)
message(curl_subproject.target_list())

#dependencies: mbedtls_target,
#include_directories: mbedtls_include_dirs,

uhr_deps = [
  mbedtls_subproject.dependency('mbedtls'),
  mbedtls_subproject.dependency('mbedcrypto'),
  mbedtls_subproject.dependency('mbedx509'),
  curl_subproject.dependency('libcurl'),
]
compiler = meson.get_compiler('cpp')
if host_machine.system() == 'windows'
  uhr_deps += [
    compiler.find_library('ws2_32'),
    compiler.find_library('crypt32'),
  ]
endif
uhr = library('uhr',
  'src/ca_certs.cc',
  'src/ca_certs.h',
	'src/session.cc',
  'src/session.h',
	'src/init.cc',
  'src/init.h',
	'src/logging.cc',
  'src/logging.h',
	'src/main.cc',
	'src/request.cc',
  'src/request.h',
	'src/request_builder.cc',
  'src/request_builder.h',
	'src/util.cc',
  'src/util.h',
  override_options: ['cpp_std=c++17'],
  cpp_args: ['-DUHR_BUILDING_SHARED', '-DNOMINMAX'],
  include_directories: [include_directories('../include')],
  dependencies: uhr_deps,
  install: true,
)

