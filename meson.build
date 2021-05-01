# https://mesonbuild.com/Builtin-options.html

project(
	'wot++',
	'cpp',
	license: 'MPL-2.0',
	version: '1.0',
	default_options: [
		'cpp_std=c++17',
		'buildtype=release',
		'native=true',
		'strip=true',
		'warning_level=2',
		'b_lto_threads=4',
		'b_lto=true',
		'b_ndebug=if-release',
	]
)

extra_cxx_opts = [
	'-pedantic',
	'-Wextra',
	'-Wnull-dereference',
	'-Wshadow',
	'-Wno-unused',
	'-Wdisabled-optimization',
	'-Wundef',
]

if meson.get_compiler('cpp').has_argument('-Wlogical-op')
	extra_cxx_opts += '-Wlogical-op'
endif

if meson.get_compiler('cpp').has_argument('-Wduplicated-branches')
	extra_cxx_opts += '-Wduplicated-branches'
endif

if meson.get_compiler('cpp').has_argument('-Wduplicated-cond')
	extra_cxx_opts += '-Wduplicated-cond'
endif

extra_opts = []
deps = []

sources = run_command('find', 'src/', '-type', 'f', '-name', '*.cpp').stdout().strip().split('\n')
headers = run_command('find', 'include/', '-type', 'f', '-name', '*.hpp').stdout().strip().split('\n')

# Set error limit.
if meson.get_compiler('cpp').get_id() == 'clang'
	add_project_arguments('-ferror-limit=2', language: 'cpp')

elif meson.get_compiler('cpp').get_id() == 'gcc'
	add_project_arguments('-fmax-errors=2', language: 'cpp')
endif


# Profiling support.
if get_option('profile')
	if meson.get_compiler('cpp').has_argument('-finstrument-functions')
		add_project_arguments('-finstrument-functions', language: 'cpp')

	else
		error('profiling is unsupported')
	endif
endif


# Native build `-march=native`.
if get_option('profile')
	if meson.get_compiler('cpp').has_argument('-march=native')
		add_project_arguments('-march=native', language: 'cpp')

	else
		error('-march=native is unsupported')
	endif
endif


# Look for libstdc++fs when using GCC or clang (except clang that emulates msvc)
if meson.get_compiler('cpp').get_argument_syntax() == 'gcc'
	deps += meson.get_compiler('cpp').find_library('stdc++fs', required: false)
endif


# Sanitizer support.
if get_option('sanitizers')
	extra_opts += 'b_sanitize=address,undefined'
	deps += meson.get_compiler('cpp').find_library('asan', required: false)
endif

if get_option('disable_run')
	add_project_arguments('-DWPP_DISABLE_RUN', language: 'cpp')
endif

if get_option('disable_colour')
	add_project_arguments('-DWPP_DISABLE_COLOUR', language: 'cpp')
endif

if get_option('disable_file')
	add_project_arguments('-DWPP_DISABLE_FILE', language: 'cpp')
endif


# REPL stuff
libreadline_dep = dependency('readline', required: false)

if get_option('disable_repl') or not libreadline_dep.found()
	add_project_arguments('-DWPP_DISABLE_REPL', language: 'cpp')
else
	deps += libreadline_dep
endif

lib = library(
	'w++',
	[sources, headers],
	include_directories: [include_directories('include/')],
	dependencies: deps,
	install: true,
	override_options: extra_opts,
	cpp_args: extra_cxx_opts
)

install_subdir('include/wpp/', install_dir: 'include/')
