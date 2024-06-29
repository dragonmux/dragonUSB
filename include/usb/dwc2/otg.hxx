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
		// This serves as both the Host Non-Periodic Tx FIFO size, and the Device EP0 Tx FIFO size
		union
		{
			volatile uint32_t hostNonPeriodicTxFIFO;
			volatile uint32_t deviceEP0TxFIFO;
		};
		volatile uint32_t hostNonPeriodicTxStatus;
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

	// Global USB configuration register constants
	constexpr static uint32_t globalUSBConfigFSTimeoutCalMask{0x7U << 0U};
	constexpr static uint32_t globalUSBConfigPHYSelUSB2_0{0U << 6U};
	constexpr static uint32_t globalUSBConfigPHYSelUSB1_1{1U << 6U};
	constexpr static uint32_t globalUSBConfigSRPCapable{1U << 8U};
	constexpr static uint32_t globalUSBConfigHNPCapable{1U << 9U};
	constexpr static uint32_t globalUSBConfigTurnaroundTimeMask{0xfU << 10U};
	constexpr static uint32_t globalUSBConfigPHYLowPower{1U << 15U};
	constexpr static uint32_t globalUSBConfigUPLIfaceNormal{0U << 17U};
	constexpr static uint32_t globalUSBConfigUPLIfaceSerial{1U << 17U};
	constexpr static uint32_t globalUSBConfigUPLIAutoResume{1U << 18U};
	constexpr static uint32_t globalUSBConfigULPIClockSuspend{1U << 19U};
	constexpr static uint32_t globalUSBConfigULPIExtVBusDrive{1U << 20U};
	constexpr static uint32_t globalUSBConfigULPIExtVBusIndicator{1U << 21U};
	constexpr static uint32_t globalUSBConfigDataPulseTxValid{0U << 22U};
	constexpr static uint32_t globalUSBConfigDataPulseTermSel{1U << 22U};
	constexpr static uint32_t globalUSBConfigPCCI{1U << 23U};
	constexpr static uint32_t globalUSBConfigPTCI{1U << 24U};
	constexpr static uint32_t globalUSBConfigUPLIProtectEnable{0U << 25U};
	constexpr static uint32_t globalUSBConfigUPLIProtectDisable{1U << 25U};
	constexpr static uint32_t globalUSBConfigForceHostMode{1U << 29U};
	constexpr static uint32_t globalUSBConfigForceDeviceMode{1U << 30U};

	constexpr inline uint32_t globalUSBConfigFSTimeoutCal(const uint8_t timeout) noexcept
		{ return static_cast<uint32_t>(timeout & 0x7U); }
	constexpr inline uint32_t globalUSBConfigTurnaroundTime(const uint8_t timeout) noexcept
		{ return static_cast<uint32_t>(timeout & 0xfU) << 10U; }

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

	// Global interrupt status and mask regsiters constants
	constexpr static uint32_t globalItrStatusCurrentModeMask{1U << 0U};
	constexpr static uint32_t globalItrStatusCurrentModeDevice{0U << 0U};
	constexpr static uint32_t globalItrStatusCurrentModeHost{1U << 0U};
	constexpr static uint32_t globalItrModeMismatch{1U << 1U};
	constexpr static uint32_t globalItrOTG{1U << 2U};
	constexpr static uint32_t globalItrSOF{1U << 3U};
	constexpr static uint32_t globalItrRxFIFONonEmpty{1U << 4U};
	constexpr static uint32_t globalItrNonPeriodicTxFIFOEmpty{1U << 5U};
	constexpr static uint32_t globalItrGlobalInNonPeriodicNAKEffective{1U << 6U};
	constexpr static uint32_t globalItrGlobalOutNAKEffective{1U << 7U};
	constexpr static uint32_t globalItrEarlySuspend{1U << 10U};
	constexpr static uint32_t globalItrUSBSuspend{1U << 11U};
	constexpr static uint32_t globalItrUSBReset{1U << 12U};
	constexpr static uint32_t globalItrEnumDone{1U << 13U};
	constexpr static uint32_t globalItrIsochronousOutDropped{1U << 14U};
	constexpr static uint32_t globalItrEoPFrame{1U << 15U};
	constexpr static uint32_t globalItrInEndpoint{1U << 18U};
	constexpr static uint32_t globalItrOutEndpoint{1U << 19U};
	constexpr static uint32_t globalItrIsochronousInXferIncomplete{1U << 20U};
	constexpr static uint32_t globalItrPeriodicXferIncomplete{1U << 21U};
	constexpr static uint32_t globalItrDataFetchSuspended{1U << 22U};
	constexpr static uint32_t globalItrResetDetected{1U << 23U};
	constexpr static uint32_t globalItrHostPort{1U << 24U};
	constexpr static uint32_t globalItrHostChannels{1U << 25U};
	constexpr static uint32_t globalItrPeriodicTxFIFOEmpty{1U << 26U};
	constexpr static uint32_t globalItrLPM{1U << 27U};
	constexpr static uint32_t globalItrConnIDStatusChange{1U << 28U};
	constexpr static uint32_t globalItrDisconnected{1U << 29U};
	constexpr static uint32_t globalItrSessionReq{1U << 30U};
	constexpr static uint32_t globalItrWakeupDetected{1U << 31U};

	// Global receive status debug read (and pop) register constants (Device mode)
	constexpr static uint32_t globalRxStatusEPNumberMask{0xfU << 0U};
	constexpr static uint32_t globalRxStatusByteCountMask{0x03ffU << 4U};
	constexpr static size_t globalRxStatusByteCountShift{4U};
	constexpr static uint32_t globalRxStatusPIDMask{0x3U << 15U};
	constexpr static uint32_t globalRxStatusPIDData0{0x0U << 15U};
	constexpr static uint32_t globalRxStatusPIDData1{0x1U << 15U};
	constexpr static uint32_t globalRxStatusPIDData2{0x2U << 15U};
	constexpr static uint32_t globalRxStatusPIDMData{0x3U << 15U};
	constexpr static uint32_t globalRxStatusPacketStatusMask{0xfU << 17U};
	constexpr static uint32_t globalRxStatusPacketStatusGlobalOutNAK{0x1U << 17U};
	constexpr static uint32_t globalRxStatusPacketStatusOutDataReceived{0x2U << 17U};
	constexpr static uint32_t globalRxStatusPacketStatusOutXferComplete{0x3U << 17U};
	constexpr static uint32_t globalRxStatusPacketStatusSetupTransComplete{0x4U << 17U};
	constexpr static uint32_t globalRxStatusPacketStatusSetupDataReceived{0x6U << 17U};
	constexpr static uint32_t globalRxStatusFrameNumberMask{0xfU << 21U};
	constexpr static size_t globalRxStatusFrameNumberShift{21U};
	constexpr static uint32_t globalRxStatusStatusPhaseStart{1U << 27U};

	// Global receive FIFO size register constants
	constexpr static uint32_t globalRxFIFOSizeMask{0x0000ffffU};

	// Host non-periodic / Device EP0 transmit FIFO size register constants
	constexpr static uint32_t hostNonPeriodicTxFIFOStartAddrMask{0x0000ffffU};
	constexpr static uint32_t hostNonPeriodicTxFIFODepthMask{0xffff0000U};
	constexpr static size_t hostNonPeriodicTxFIFODepthShift{16U};
	constexpr static uint32_t deviceEP0TxFIFOStartAddrMask{0x0000ffffU};
	constexpr static uint32_t deviceEP0TxFIFODepthMask{0xffff0000U};
	constexpr static size_t deviceEP0TxFIFODepthShift{16U};

	// Host non-periodic transmit FIFO status register constants
	constexpr static uint32_t hostNonPeriodicTxStatusFIFOAvailableMask{0x0000ffffU};
	constexpr static uint32_t hostNonPeriodicTxStatusQueueAvailableMask{0x00ff0000U};
	constexpr static size_t hostNonPeriodicTxStatusQueueAvailableShift{16U};
	constexpr static uint32_t hostNonPeriodicTxStatusTopReqTypeMask{0x7f000000U};
	constexpr static size_t hostNonPeriodicTxStatusTopReqTypeShift{24U};

	// Global general core configuration register constants
	constexpr static uint32_t globalCoreConfigDataDetectEnable{1U << 0U};
	constexpr static uint32_t globalCoreConfigPrimaryDetectEnable{1U << 1U};
	constexpr static uint32_t globalCoreConfigSecondaryDetectStatus{1U << 2U};
	constexpr static uint32_t globalCoreConfigPullDetectMask{1U << 3U};
	constexpr static uint32_t globalCoreConfigPullDetectNormal{0U << 3U};
	constexpr static uint32_t globalCoreConfigPullDetectPS2{1U << 3U};
	constexpr static uint32_t globalCoreConfigIntPHYDisable{0U << 16U};
	constexpr static uint32_t globalCoreConfigIntPHYEnable{1U << 16U};
	constexpr static uint32_t globalCoreConfigBatChargeDetectEnable{1U << 17U};
	constexpr static uint32_t globalCoreConfigDataDetectEnable{1U << 18U};
	constexpr static uint32_t globalCoreConfigPrimaryDetectEnable{1U << 19U};
	constexpr static uint32_t globalCoreConfigSecondaryDetectEnable{1U << 20U};
	constexpr static uint32_t globalCoreConfigVBusDetectEnable{1U << 21U};
} // namespace usb::dwc2

#endif /*USB_DWC2_OTG_HXX*/
