project(
	'fsmtp',
	'cpp',
	default_options: [
		'cpp_std=c++17'
	]
)

lib_openssl = dependency('openssl')
lib_threads = dependency('threads')
lib_hiredis = dependency('hiredis')
lib_catch2 = dependency('catch2')
lib_boost = dependency('boost')
lib_zlib = dependency('zlib')

subdir('env')

comp_cc = meson.get_compiler('cpp')
lib_resolv = comp_cc.find_library('resolv', required: true)
lib_cassandra = comp_cc.find_library('cassandra', required: true)
lib_jsoncpp = comp_cc.find_library('jsoncpp', required: true)
lib_boost_iostreams = comp_cc.find_library('boost_iostreams', required: true)

sources = [
	'main.cc'
]

test_sources = [
	'tests.cc'
]

subdir('lib')
subdir('tests')

executable(
	'fsmtp',
	sources,
	dependencies: [
		lib_openssl,
		lib_cassandra,
		lib_threads,
		lib_hiredis,
		lib_resolv,
		lib_jsoncpp,
		lib_boost,
		lib_zlib,
		lib_boost_iostreams
	]
)

unit_tests = executable(
	'unit_tests',
	test_sources,
	dependencies: [
		lib_openssl,
		lib_cassandra,
		lib_threads,
		lib_hiredis,
		lib_resolv,
		lib_jsoncpp,
		lib_catch2,
		lib_boost,
		lib_zlib,
		lib_boost_iostreams
	]
)

test('Unit tests', unit_tests)
