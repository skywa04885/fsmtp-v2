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

subdir('env')

comp_cc = meson.get_compiler('cpp')
lib_resolv = comp_cc.find_library('resolv', required: true)
lib_cassandra = comp_cc.find_library('cassandra', required: true)
lib_jsoncpp = comp_cc.find_library('jsoncpp', required: true)

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
		lib_jsoncpp
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
		lib_catch2
	]
)

test('Unit tests', unit_tests)
