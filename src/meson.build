# SPDX-License-Identifier: BSD-3-Clause
substrate = subproject(
	'substrate',
	required: true,
	version: '>=0.0.1',
	default_options: [
		'build_tests=false',
		'build_library=false'
	]
).get_variable(
	'substrate_dep'
).partial_dependency(
	compile_args: true,
	includes: true
)

chip = get_option('chip')

if chip.startswith('tm4c')
	platform = subproject(
		'dragonTI',
		required: true,
		version: '>=0.0.1',
	).get_variable(
		'dragonTI_dep'
	).partial_dependency(
		compile_args: true,
		includes: true
	)
elif chip.startswith('atxmega')
	platform = subproject(
		'dragonAVR',
		required: true,
		version: '>=0.0.1',
	).get_variable(
		'dragonAVR_dep'
	)
else
	error('Must define the target device to build the stack for')
endif

subdir(get_option('chip'))
subdir('drivers')

dragonUSBSrc = [
	platformSrcs, drivers, 'core.cxx', 'device.cxx'
]

buildDefs = [
	'-DUSB_INTERFACES=@0@'.format(get_option('interfaces')),
	'-DUSB_ENDPOINTS=@0@'.format(get_option('endpoints')),
	'-DUSB_BUFFER_SIZE=@0@'.format(get_option('epBufferSize')),
	'-DUSB_CONFIG_DESCRIPTORS=@0@'.format(get_option('configDescriptors')),
	'-DUSB_INTERFACE_DESCRIPTORS=@0@'.format(get_option('ifaceDescriptors')),
	'-DUSB_ENDPOINT_DESCRIPTORS=@0@'.format(get_option('endpointDescriptors')),
	'-DUSB_STRINGS=@0@'.format(get_option('strings')),
]

if 'dfu' in get_option('drivers')
	buildDefs += [
		'-DUSB_DFU_FLASH_PAGE_SIZE=@0@'.format(get_option('dfuFlashPageSize')),
		'-DUSB_DFU_FLASH_BUFFER_SIZE=@0@'.format(get_option('dfuFlashBufferSize')),
		'-DUSB_DFU_FLASH_ERASE_SIZE=@0@'.format(get_option('dfuFlashEraseSize')),
	]
endif

allowPIC = cxx.has_argument('-fPIC')

dragonUSB = static_library(
	'dragonUSB',
	dragonUSBSrc,
	cpp_args: buildDefs,
	include_directories: [include_directories('../include')],
	dependencies: [platform, substrate],
	gnu_symbol_visibility: 'inlineshidden',
	build_by_default: true,
	pic: allowPIC
)
