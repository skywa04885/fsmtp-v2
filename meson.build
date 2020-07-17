project('fsmtp', 'cpp')

lib_ncurses = dependency('ncurses')
lib_openssl = dependency('openssl')
lib_cassandra = dependency('cassandra')
lib_threads = dependency('threads')

executable(
	'fsmtp',
	'main.cc',
	dependencies: [
		lib_ncurses,
		lib_openssl,
		lib_cassandra,
		lib_threads
	]
)