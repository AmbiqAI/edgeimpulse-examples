# Moified version of autogenerated file - created by NeuralSPOT make nest, modified for this project
# DEFINES += CFG_TUSB_MCU=OPT_MCU_APOLLO4 NEURALSPOT apollo4p_evb PART_apollo4p AM_PART_APOLLO4P AM_PACKAGE_BGA __FPU_PRESENT gcc TF_LITE_STATIC_MEMORY TF_LITE_STRIP_ERROR_STRINGS
INCLUDES += neuralspot/ns-core/includes-api neuralspot/ns-harness/includes-api neuralspot/ns-peripherals/includes-api neuralspot/ns-ipc/includes-api neuralspot/ns-audio/includes-api neuralspot/ns-usb/includes-api neuralspot/ns-utils/includes-api neuralspot/ns-rpc/includes-api neuralspot/ns-i2c/includes-api extern/AmbiqSuite/R4.3.0/boards/apollo4p_evb/bsp extern/AmbiqSuite/R4.3.0/CMSIS/ARM/Include extern/AmbiqSuite/R4.3.0/CMSIS/AmbiqMicro/Include extern/AmbiqSuite/R4.3.0/devices extern/AmbiqSuite/R4.3.0/mcu/apollo4p extern/AmbiqSuite/R4.3.0/mcu/apollo4p/hal/mcu extern/AmbiqSuite/R4.3.0/utils extern/AmbiqSuite/R4.3.0/third_party/FreeRTOSv10.1.1/Source/include extern/AmbiqSuite/R4.3.0/third_party/FreeRTOSv10.1.1/Source/portable/GCC/AMapollo4 extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/common extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/osal extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/class/cdc extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/device  extern/SEGGER_RTT/R7.70a/RTT extern/SEGGER_RTT/R7.70a/Config extern/erpc/R1.9.1/includes-api
# INCLUDES += neuralspot/ns-core/includes-api neuralspot/ns-harness/includes-api neuralspot/ns-peripherals/includes-api neuralspot/ns-ipc/includes-api neuralspot/ns-audio/includes-api neuralspot/ns-usb/includes-api neuralspot/ns-utils/includes-api neuralspot/ns-rpc/includes-api neuralspot/ns-i2c/includes-api extern/AmbiqSuite/R4.3.0/boards/apollo4p_evb/bsp extern/AmbiqSuite/R4.3.0/CMSIS/AmbiqMicro/Include extern/AmbiqSuite/R4.3.0/devices extern/AmbiqSuite/R4.3.0/mcu/apollo4p extern/AmbiqSuite/R4.3.0/mcu/apollo4p/hal/mcu extern/AmbiqSuite/R4.3.0/utils extern/AmbiqSuite/R4.3.0/third_party/FreeRTOSv10.1.1/Source/include extern/AmbiqSuite/R4.3.0/third_party/FreeRTOSv10.1.1/Source/portable/GCC/AMapollo4 extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/common extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/osal extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/class/cdc extern/AmbiqSuite/R4.3.0/third_party/tinyusb/src/device extern/CMSIS/CMSIS_5-5.9.0/CMSIS/Core/Include extern/CMSIS/CMSIS_5-5.9.0/CMSIS/NN/Include extern/CMSIS/CMSIS_5-5.9.0/CMSIS/DSP/Include extern/CMSIS/CMSIS_5-5.9.0/CMSIS/DSP/PrivateInclude extern/tensorflow/0c46d6e/. extern/tensorflow/0c46d6e/third_party extern/tensorflow/0c46d6e/third_party/flatbuffers/include extern/SEGGER_RTT/R7.70a/RTT extern/SEGGER_RTT/R7.70a/Config extern/erpc/R1.9.1/includes-api
libs += libs/ns-harness.a libs/ns-peripherals.a libs/ns-ipc.a libs/ns-audio.a libs/ns-usb.a libs/ns-utils.a libs/ns-rpc.a libs/ns-i2c.a libs/ambiqsuite.a libs/segger_rtt.a libs/erpc.a libs/libam_hal.a libs/libam_bsp.a
