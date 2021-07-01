// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORMS_ATXMEGA256A3U_CONSTANTS___HXX
#define USB_PLATFORMS_ATXMEGA256A3U_CONSTANTS___HXX

#include <cstdint>

namespace vals::usb
{
	// Control A register constants
	constexpr static const uint8_t ctrlAUSBEnable{0x80};
	constexpr static const uint8_t ctrlAModeLowSpeed{0x00};
	constexpr static const uint8_t ctrlAModeFullSpeed{0x40};
	constexpr static const uint8_t ctrlAFIFOEnable{0x20};
	constexpr static const uint8_t ctrlAStoreFrameNum{0x10};

	constexpr inline uint8_t ctrlAMaxEP(const uint8_t endpointCount) noexcept
		{ return endpointCount & 0x0FU; }

	// Control B register constants
	constexpr static const uint8_t ctrlBPullUpDuringReset{0x10};
	constexpr static const uint8_t ctrlBRemoteWakeUp{0x04};
	constexpr static const uint8_t ctrlBGlobalNACK{0x02};
	constexpr static const uint8_t ctrlBAttach{0x01};

	// Status register constants
	constexpr static const uint8_t statusReset{0x01};
	constexpr static const uint8_t statusSuspend{0x02};
	constexpr static const uint8_t statusLocalResume{0x04};
	constexpr static const uint8_t statusUpstreamResume{0x08};
	constexpr static const uint8_t statusResume{0x0C};

	// Address register constant
	constexpr static const uint8_t addressMask{0x7F};

	// Interrupt Control A register constants
	constexpr static const uint8_t intCtrlAEnableStall{0x40};
	constexpr static const uint8_t intCtrlAEnableBusError{0x40};
	constexpr static const uint8_t intCtrlAEnableBusEvent{0x40};
	constexpr static const uint8_t intCtrlAEnableSOF{0x80};
	constexpr static const uint8_t intCtrlAClearMask{0x0C};

	// Interrupt Control B register constants
	constexpr static const uint8_t intCtrlBEnableSetupComplete{0x01};
	constexpr static const uint8_t intCtrlBEnableIOComplete{0x02};
	constexpr static const uint8_t intCtrlBClearMask{0xFC};

	// Interrupt Status A register constants
	constexpr static const uint8_t itrStatusStall{0x01};
	constexpr static const uint8_t itrStatusOverflow{0x02};
	constexpr static const uint8_t itrStatusUnderflow{0x04};
	constexpr static const uint8_t itrStatusCRCError{0x08};
	constexpr static const uint8_t itrStatusReset{0x10};
	constexpr static const uint8_t itrStatusResume{0x20};
	constexpr static const uint8_t itrStatusSuspend{0x40};
	constexpr static const uint8_t itrStatusSOF{0x80};

	// Interrupt Status B register constants
	constexpr static const uint8_t itrStatusSetup{0x01};
	constexpr static const uint8_t itrStatusIOComplete{0x02};

	// Endpoint control register constants
	constexpr static const uint8_t usbEPCtrlStall{0x04};
	constexpr static const uint8_t usbEPCtrlItrDisable{0x08};
	constexpr static const uint8_t usbEPCtrlTypeMask{0xC0};

	// Endpoint status register constants
	constexpr static const uint8_t usbEPStatusDTS{0x01};
	constexpr static const uint8_t usbEPStatusNACK0{0x02};
	constexpr static const uint8_t usbEPStatusNACK1{0x04};
	constexpr static const uint8_t usbEPStatusBank{0x08};
	constexpr static const uint8_t usbEPStatusSetupComplete{0x10};
	constexpr static const uint8_t usbEPStatusIOComplete{0x20};
	constexpr static const uint8_t usbEPStatusNotReady{0x40};
	constexpr static const uint8_t usbEPStatusStall{0x80};
} // namespace vals::usb

#endif /*USB_PLATFORMS_ATXMEGA256A3U_CONSTANTS___HXX*/
