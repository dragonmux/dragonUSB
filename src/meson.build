# SPDX-License-Identifier: BSD-3-Clause
substrate = dependency(
	'substrate',
	required: true,
	version: '>=0.0.1',
	fallback: ['substrate', 'substrate_dep'],
	default_options: [
		'build_tests=false',
		'build_library=false'
	]
).partial_dependency(compile_args: true, includes: true)

if get_option('chip').startswith('tm4c')
	platform = dependency(
		'dragonTI',
		required: true,
		version: '>=0.0.1',
		fallback: ['dragonTI', 'dragonTI_dep']
	).partial_dependency(compile_args: true, includes: true)
else
	error('Must define the target device to build the stack for')
endif

subdir(get_option('chip'))
subdir('drivers')

dragonUSBSrc = [
	platformSrcs, drivers
]

buildDefs = [
	'-DUSB_INTERFACES=@0@'.format(get_option('interfaces')),
	'-DUSB_ENDPOINTS=@0@'.format(get_option('endpoints')),
	'-DUSB_BUFFER_SIZE=@0@'.format(get_option('epBufferSize')),
	'-DUSB_CONFIG_DESCRIPTORS=@0@'.format(get_option('configDescriptors')),
	'-DUSB_INTERFACE_DESCRIPTORS=@0@'.format(get_option('ifaceDescriptors')),
	'-DUSB_ENDPOINT_DESCRIPTORS=@0@'.format(get_option('endpointDescriptors')),
	'-DUSB_STRINGS=@0@'.format(get_option('strings'))
]

dragonUSB = static_library(
	'dragonUSB',
	dragonUSBSrc,
	cpp_args: buildDefs,
	include_directories: [include_directories('../include')],
	dependencies: [platform, substrate],
	gnu_symbol_visibility: 'inlineshidden',
	build_by_default: true
)
