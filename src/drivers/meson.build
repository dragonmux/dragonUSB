# SPDX-License-Identifier: BSD-3-Clause
enableDrivers = get_option('drivers')

drivers = []

if enableDrivers.contains('dfu')
	drivers += files('dfu.cxx')
endif
