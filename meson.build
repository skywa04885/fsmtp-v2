project(
	'fsmtp',
	'cpp',
	default_options: [
		'cpp_std=c++17'
	]
)

lib_ncurses = dependency('ncurses')
lib_openssl = dependency('openssl')
# lib_cassandra = dependency('cassandra')
lib_threads = dependency('threads')
lib_hiredis = dependency('hiredis')

subdir('env')

comp_cc = meson.get_compiler('cpp')
lib_resolv = comp_cc.find_library('resolv', required: true)
lib_cassandra = comp_cc.find_library('cassandra', required: true)

jsoncpp_project = subproject('jsoncpp')
lib_jsoncpp = jsoncpp_project.get_variable('jsoncpp_dep')

sources = [
	'main.cc'
]

subdir('lib')

executable(
	'fsmtp',
	sources,
	dependencies: [
		lib_ncurses,
		lib_openssl,
		lib_cassandra,
		lib_threads,
		lib_hiredis,
		lib_resolv,
		lib_jsoncpp
	]
)
