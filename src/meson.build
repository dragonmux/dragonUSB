# SPDX-License-Identifier: BSD-3-Clause

substrate = dependency(
	'substrate',
	required: true,
	version: '>=0.0.1',
	fallback: ['substrate', 'substrate_dep'],
	default_options: [
		'build_tests=false',
		'build_library=false'
	],
	native: false
)

if get_option('chip').startswith('tm4c')
	platform = dependency(
		'dragonTI',
		required: true,
		version: '>=0.0.1',
		fallback: ['dragonTI', 'dragonTI_dep'],
		native: false
	)
else
	error('Must define the target device to build the stack for')
endif

subdir(get_option('chip'))

dragonUSBSrc = [
	platformSrcs, 'device.cxx'
]

dragonUSB = static_library(
	'dragonUSB',
	dragonUSBSrc,
	include_directories: [include_directories('../include')],
	dependencies: [platform, substrate],
	gnu_symbol_visibility: 'inlineshidden',
	build_by_default: true,
	native: false
)
