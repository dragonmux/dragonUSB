// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_DWC2_OTG_HXX
#define USB_DWC2_OTG_HXX

#include <cstdint>
#include <cstddef>
#include <array>

namespace usb::dwc2
{
	// NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
	struct channel_t final
	{
		volatile uint32_t characteristics;
		volatile uint32_t splitCtrl;
		volatile uint32_t itrStatus;
		volatile uint32_t itrMask;
		volatile uint32_t transferSize;
		volatile uint32_t dmaAddress;
		volatile uint32_t reserved;
		volatile uint32_t dmaAddressBuffer;
	};

	struct inEP_t final
	{
		volatile uint32_t ctrl;
		const volatile uint32_t reserved1;
		volatile uint32_t itrStatus;
		const volatile uint32_t reserved2;
		volatile uint32_t transferSize;
		volatile uint32_t dmaAddress;
		volatile uint32_t transmitFIFOStatus;
		const volatile uint32_t reserved3;
	};

	struct outEP_t final
	{
		volatile uint32_t ctrl;
		const volatile uint32_t reserved1;
		volatile uint32_t itrStatus;
		const volatile uint32_t reserved2;
		volatile uint32_t transferSize;
		volatile uint32_t dmaAddress;
		std::array<const volatile uint32_t, 2> reserved3;
	};

	struct otg_t final
	{
		volatile uint32_t globalOGTCtrlStatus;
		volatile uint32_t globalOTGInterrupt;
		volatile uint32_t globalAHBConfig;
		volatile uint32_t globalUSBConfig;
		volatile uint32_t globalResetCtrl;
		volatile uint32_t globalItrStatus;
		volatile uint32_t globalItrMask;
		volatile uint32_t globalRxStatusRead;
		volatile uint32_t globalRxStatusPop;
		volatile uint32_t globalRxFIFOSize;
		volatile uint32_t globalNonPeriodicTxFIFOSize;
		volatile uint32_t globalNonPeriodicTxStatus;
		std::array<const volatile uint32_t, 2> reserved1;
		volatile uint32_t globalCoreConfig;
		volatile uint32_t coreID;
		std::array<const volatile uint32_t, 5> reserved2;
		volatile uint32_t globalLPMConfig;
		std::array<const volatile uint32_t, 42> reserved3;
		volatile uint32_t hostPeriodicTxFIFOSize;
		std::array<volatile uint32_t, 8> deviceInEPTxFIFOSize;
		std::array<const volatile uint32_t, 183> reserved4;
		volatile uint32_t hostConfig;
		volatile uint32_t hostFrameInterval;
		volatile uint32_t hostFrameNumber;
		const volatile uint32_t reserved5;
		volatile uint32_t hostPeriodicTxStatus;
		volatile uint32_t hostChannelsItrStatus;
		volatile uint32_t hostChannelsItrMask;
		volatile uint32_t hostFrameListBaseAddr;
		std::array<const volatile uint32_t, 8> reserved6;
		volatile uint32_t hostPortCtrlStatus;
		std::array<const volatile uint32_t, 47> reserved7;
		std::array<channel_t, 16> hostChannel;
		std::array<const volatile uint32_t, 64> reserved8;
		volatile uint32_t deviceConfig;
		volatile uint32_t deviceCtrl;
		volatile uint32_t deviceStatus;
		const volatile uint32_t reserved9;
		volatile uint32_t deviceInEPItrMask;
		volatile uint32_t deviceOutEPItrMask;
		volatile uint32_t deviceAllEPItrStatus;
		volatile uint32_t deviceAllEPItrMask;
		std::array<const volatile uint32_t, 2> reserved10;
		volatile uint32_t deviceVBusDischargeTime;
		volatile uint32_t deviceVBusPulsingTime;
		volatile uint32_t deviceThresholdCtrl;
		volatile uint32_t deviceInEPEmptyItrMask;
		volatile uint32_t deviceEPItrStatus;
		volatile uint32_t deviceEPItrMask;
		const volatile uint32_t reserved11;
		volatile uint32_t deviceEP1InItrMask;
		std::array<const volatile uint32_t, 16> reserved12;
		volatile uint32_t deviceEP1OutItrMask;
		std::array<const volatile uint32_t, 30> reserved13;
		std::array<inEP_t, 8> deviceInEP;
		std::array<const volatile uint32_t, 64> reserved14;
		std::array<outEP_t, 8> deviceOutEP;
		std::array<const volatile uint32_t, 128> reserved15;
		volatile uint32_t powerClockGateCtrl;
	};
	// NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
} // namespace usb::dwc2

#endif /*USB_DWC2_OTG_HXX*/
