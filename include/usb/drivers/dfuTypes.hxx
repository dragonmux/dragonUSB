#ifndef USB_DRIVERS_DFU_TYPES__HXX
#define USB_DRIVERS_DFU_TYPES__HXX

#include <cstdint>
#include <array>

namespace usb::dfu::types
{
	enum class request_t : uint8_t
	{
		detach = 0,
		download = 1,
		upload = 2,
		getStatus = 3,
		clearStatus = 4,
		getState = 5,
		abort = 6
	};

	enum class dfuState_t : uint8_t
	{
		applicationIdle,
		applicationDetach,
		dfuIdle,
		downloadSync,
		downloadBusy,
		downloadIdle,
		manifestSync,
		manifest,
		manifestWaitReset,
		uploadIdle,
		error
	};

	enum class dfuStatus_t : uint8_t
	{
		ok = 0, // No error
		target = 1, // File is not targeted for use by this device
		file = 2, // File is for this device but fails some vendor-specific verification test
		wirte = 3, // Device is unable to write memory
		erase = 4, // Memory erase failed
		checkErased = 5, // Memory erase check failed
		program = 6, // Program memory failed
		verify = 7, // Program memory failed verification
		address = 8, // Cannot program memory due to received address that is out of range
		notDone = 9, // Received 0-length download packet but device does not yet have all the data
		firmware = 10, // Device's firmware is corrupt. It cannot return to run-time (non-DFU) operations
		vendor = 11, // Vendor-specific error (description contained in string given by iString)
		usbReset = 12, // Device detected unexpected USB reset signaling
		usbPOR = 13, // Devices experienced unexpected POR
		unknown = 14, // Something when twrong but we have no idea what
		stalledPacket = 15, // Device stalled due to an unexpected request
	};

	struct config_t final
	{
		dfuStatus_t status;
		std::array<uint8_t, 3> pollTimeout;
		dfuState_t state;
		uint8_t string;
	};
} // namespace usb::dfu::types

#endif /*USB_DRIVERS_DFU_TYPES__HXX*/
