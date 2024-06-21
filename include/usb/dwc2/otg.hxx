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

	enum class ahbBurstLength_t : uint8_t
	{
		single32b = 0U,
		incr32b = 1U,
		incr4x32b = 3U,
		incr8x32b = 5U,
		incr16x32b = 7U,
	};

	// Global OTG control/status register constants
	constexpr static uint32_t globalOGTCtrlStatusSessionReqStatusMask{1U << 0U};
	constexpr static uint32_t globalOGTCtrlStatusSessionReqStatusFailure{0U << 0U};
	constexpr static uint32_t globalOGTCtrlStatusSessionReqStatusSuccess{1U << 0U};
	constexpr static uint32_t globalOGTCtrlStatusSessionReq{1U << 1U};
	constexpr static uint32_t globalOGTCtrlStatusVBusValidOverrideEnable{1U << 2U};
	constexpr static uint32_t globalOGTCtrlStatusVBusValidOverrideValue{1U << 3U};
	constexpr static uint32_t globalOGTCtrlStatusASessValidOverrideEnable{1U << 4U};
	constexpr static uint32_t globalOGTCtrlStatusASessValidOverrideValue{1U << 5U};
	constexpr static uint32_t globalOGTCtrlStatusBSessValidOverrideEnable{1U << 6U};
	constexpr static uint32_t globalOGTCtrlStatusBSessValidOverrideValue{1U << 7U};
	constexpr static uint32_t globalOGTCtrlStatusHostNegStatusMask{1U << 8U};
	constexpr static uint32_t globalOGTCtrlStatusHostNegStatusFailure{0U << 8U};
	constexpr static uint32_t globalOGTCtrlStatusHostNegStatusSuccess{1U << 8U};
	constexpr static uint32_t globalOGTCtrlStatusHNPRequest{1U << 9U};
	constexpr static uint32_t globalOGTCtrlStatusHostSetHNPEnable{1U << 10U};
	constexpr static uint32_t globalOGTCtrlStatusDeviceHNPEnale{1U << 11U};
	constexpr static uint32_t globalOGTCtrlStatusEmbeddedHostEnable{1U << 12U};
	constexpr static uint32_t globalOGTCtrlStatusConnIDMask{1U << 16U};
	constexpr static uint32_t globalOGTCtrlStatusConnIDA{0U << 16U};
	constexpr static uint32_t globalOGTCtrlStatusConnIDB{1U << 16U};
	constexpr static uint32_t globalOGTCtrlStatusDebounceMask{1U << 17U};
	constexpr static uint32_t globalOGTCtrlStatusDebounceLong{0U << 17U};
	constexpr static uint32_t globalOGTCtrlStatusDebounceShort{1U << 17U};
	constexpr static uint32_t globalOGTCtrlStatusASessValid{1U << 18U};
	constexpr static uint32_t globalOGTCtrlStatusBSessValid{1U << 19U};
	constexpr static uint32_t globalOGTCtrlStatusOTGMask{1U << 20U};
	constexpr static uint32_t globalOGTCtrlStatusOTG1_3{0U << 20U};
	constexpr static uint32_t globalOGTCtrlStatusOTG2_0{1U << 20U};
	constexpr static uint32_t globalOGTCtrlStatusCurrentModeMask{1U << 21U};
	constexpr static uint32_t globalOGTCtrlStatusCurrentModeDevice{0U << 21U};
	constexpr static uint32_t globalOGTCtrlStatusCurrentModeHost{1U << 2U};

	// Global OTG interrupt status register constants
	constexpr static uint32_t globalOTGInterruptSessionEndDetected{1U << 2U};
	constexpr static uint32_t globalOTGInterruptSessionReqStatusChange{1U << 8U};
	constexpr static uint32_t globalOTGInterruptHostNegStatusChange{1U << 9U};
	constexpr static uint32_t globalOTGInterruptHostNegDetected{1U << 17U};
	constexpr static uint32_t globalOTGInterruptATimeoutChange{1U << 18U};
	constexpr static uint32_t globalOTGInterruptDebounceDone{1U << 19U};

	// Global AHB configuration register constants
	constexpr static uint32_t globalAHBConfigGlobalIntUnmask{1U << 0U};
	constexpr static uint32_t globalAHBConfigBurstLengthMask{0xfU << 1U};
	constexpr static uint32_t globalAHBConfigDMAEnable{1U << 5U};
	constexpr static uint32_t globalAHBConfigTxFIFONotEmpty{1U << 7U};
	constexpr static uint32_t globalAHBConfigPeriodicFIFOEmpty{1U << 8U};

	constexpr inline uint32_t globalAHBConfigBurstLength(const ahbBurstLength_t burstLength) noexcept
		{ return static_cast<uint32_t>(static_cast<uint8_t>(burstLength) & 0xfU) << 1U; }

	// Global reset register constants
	constexpr static uint32_t globalResetCtrlCore{1U << 0U};
	constexpr static uint32_t globalResetCtrlPartial{1U << 1U};
	constexpr static uint32_t globalResetCtrlHostFrameCounter{1U << 2U};
	constexpr static uint32_t globalResetCtrlRxFIFOFlush{1U << 4U};
	constexpr static uint32_t globalResetCtrlTxFIFOFlush{1U << 5U};
	constexpr static uint32_t globalResetCtrlTxFIFONumberMask{0x1fU << 6U};
	constexpr static size_t globalResetCtrlTxFIFONumberShift{6U};
	constexpr static uint32_t globalResetCtrlDMAReqOngoing{1U << 30U};
	constexpr static uint32_t globalResetCtrlAHBIdle{1U << 31U};
} // namespace usb::dwc2

#endif /*USB_DWC2_OTG_HXX*/
