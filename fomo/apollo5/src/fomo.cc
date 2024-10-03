/**
 * @file Vision.cc
 * @author Ambiq
 * @brief This demo is intended to work with the vision WebBLE client
 * See README.md for more details
 * @version 0.1
 * @date 2022-11-09
 *
 * @copyright Copyright (c) 2022
 *
 */

// #include "arm_math.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
// neuralSPOT
#include "ns_ambiqsuite_harness.h"
#include "ns_debug_log.h"
#include "ns_energy_monitor.h"
#include "ns_malloc.h"
#include "ns_peripherals_button.h"
#include "ns_peripherals_power.h"
#include "ns_timer.h"
#include "ns_usb.h"
#include "ns_core.h"
#include "tusb.h"
#include "ns_camera.h"
#include "ArducamCamera.h"

// EdgeImpulse Includes
#include "edge-impulse/edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "edge-impulse/edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse/model/model-parameters/model_metadata.h"
#include "edge-impulse/model/model-parameters/model_variables.h"

// Include the model

// Locals
#include "fomo.h"

// #define GRAYSCALE
extern ArducamCamera camera; // Arducam driver assumes this is a global, so :shrug:

#if (configAPPLICATION_ALLOCATED_HEAP == 1)
size_t ucHeapSize = 32 * 1024;
uint8_t  ucHeap[(NS_MALLOC_HEAP_SIZE_IN_K + 5) * 1024] __attribute__((aligned(4)));
#endif

#define MY_RX_BUFSIZE 4096
#define MY_TX_BUFSIZE 4096
static uint8_t my_rx_ff_buf[MY_RX_BUFSIZE] __attribute__((aligned(16)));
static uint8_t my_tx_ff_buf[MY_TX_BUFSIZE] __attribute__((aligned(16)));
static ns_usb_config_t webUsbConfig = {
    .api = &ns_usb_V1_0_0,
    .deviceType = NS_USB_VENDOR_DEVICE,
    .rx_buffer = NULL,
    .rx_bufferLength = 0,
    .tx_buffer = NULL,
    .tx_bufferLength = 0,
    .rx_cb = NULL,
    .tx_cb = NULL,
    .service_cb = NULL};

// USB Data sent to WebUSB client
typedef enum {
    FIRST_CHUNK = 0x0,
    MIDDLE_CHUNK = 0x1,
    LAST_CHUNK = 0x2,
} usb_data_descriptor_e;

#define MAX_WEBUSB_FRAME 512
#define WEBUSB_HEADER_SIZE 2
#define MAX_WEBUSB_CHUNK (MAX_WEBUSB_FRAME - WEBUSB_HEADER_SIZE)
#define MAX_RESULTS 5

typedef struct usb_data {
    // Jpeg Image
    uint8_t descriptor;
    uint8_t mode; // 0 RGB, 1 JPG
    uint8_t buffer[MAX_WEBUSB_FRAME];
} usb_data_t;

typedef struct {
    char id[20]; // 20 chars max
    uint8_t x;
    uint8_t y;
    uint8_t height;
    uint8_t width;
    uint8_t confidence;  
} result_t;

typedef struct usb_metadata {
    // Jpeg Image
    uint8_t descriptor; // 0/1/2 for first/middle/last chunk, 3 for metadata
    uint8_t decode_time;
    uint8_t inference_latency;
    uint16_t microjoules_per_inference;
    uint8_t num_results;
    result_t results[MAX_RESULTS];
} usb_metadata_t;

// #define JPG_BUFF_SIZE (320*320)
#define JPG_BUFF_SIZE 1024*44
#define RGB_BUFF_SIZE CAM_BUFF_SIZE
static uint8_t jpgBuffer1[JPG_BUFF_SIZE] __attribute__((aligned(16)));
uint32_t jp1Guard;
static uint8_t jpgBuffer2[JPG_BUFF_SIZE] __attribute__((aligned(16)));
uint32_t jp2Guard;
static uint8_t rgbBuffer1[RGB_BUFF_SIZE] __attribute__((aligned(16))); // output of decoded jpg
uint32_t rgb1Guard;
static uint8_t rgbBuffer2[RGB_BUFF_SIZE] __attribute__((aligned(16))); // output of decoded jpg
uint32_t rgb2Guard;

const ns_power_config_t ns_pwr_config = {
    .api = &ns_power_V1_0_0,
    .eAIPowerMode = NS_MAXIMUM_PERF,
    .bNeedAudAdc = false,
    .bNeedSharedSRAM = true,
    .bNeedCrypto = true,
    .bNeedBluetooth = false,
    .bNeedUSB = true,
    .bNeedIOM = true,
    .bNeedAlternativeUART = false,
    .b128kTCM = false};

static uint32_t elapsedTime = 0;
ns_timer_config_t ei_tickTimer = {
    .api = &ns_timer_V1_0_0,
    .timer = NS_TIMER_COUNTER,
    .enableInterrupt = false,
};

// Forward declarations
void tic(); 
uint32_t toc();
void press_jpg_shutter_button(ns_camera_config_t *cfg);
void press_rgb_shutter_button(ns_camera_config_t *cfg);
void picture_dma_complete(ns_camera_config_t *cfg);
void picture_taken_complete(ns_camera_config_t *cfg);


ns_camera_config_t camera_config = {
    .api = &ns_camera_V1_0_0,
    .spiSpeed = AM_HAL_IOM_8MHZ,
    .cameraHw = NS_ARDUCAM,
    .imageMode = NS_CAM_IMAGE_MODE_320X320,
    .imagePixFmt = NS_CAM_IMAGE_PIX_FMT_JPEG,
    // .imagePixFmt = NS_CAM_IMAGE_PIX_FMT_RGB565,
    .spiConfig = {.iom = 2},
    .dmaCompleteCb = picture_dma_complete,
    .pictureTakenCb = picture_taken_complete,
};

// Camera State
typedef enum {
    WAITING_FOR_JPG_SHUTTER,
    WAITING_FOR_RGB_SHUTTER,
    WAITING_FOR_JPG_DMA_PING,
    WAITING_FOR_JPG_DMA_PONG,
    WAITING_FOR_RGB_DMA_PING,
    WAITING_FOR_RGB_DMA_PONG,
    CAMERA_IDLE_JPG_PING,
    CAMERA_IDLE_JPG_PONG,
    CAMERA_IDLE_RGB_PING,
    CAMERA_IDLE_RGB_PONG,
} camera_state_e;

// Camera FSM State
volatile camera_state_e camera_state = WAITING_FOR_JPG_SHUTTER; 
volatile bool dmaComplete = false; // ISR-land
volatile uint8_t dmaTarget = 1; // 1 = "ping", 2 = "pong"
volatile bool pictureTaken = false;
volatile bool prettyMode = true; // In pretty mode, we take jpg render it and covert to rgb for inference
                                 // Otherwise, we take RGB, render it, and do inference directly on that.
volatile bool oldPrettyMode = true;
volatile bool inferenceRunning = false;

// Camera/CPU IPC
// The various buffers are ready to be consumed by the CPU
volatile bool jpgReady1 = false; // IPC to CPU-land
volatile bool jpgReady2 = false;
volatile bool rgbReady1 = false;
volatile bool rgbReady2 = false;

// The legnth of the buffers
volatile uint32_t jpg_buffer_length1 = 0;
volatile uint32_t rgb_buffer_length1 = 0;
volatile uint32_t jpg_buffer_length2 = 0;
volatile uint32_t rgb_buffer_length2 = 0;

static uint32_t bufferOffset = 0;

static uint32_t start_jpg_dma(uint8_t bufferId);
static uint32_t start_rgb_dma(uint8_t bufferId);

uint32_t chop_off_trailing_zeros(uint8_t *buff, uint32_t length);

void camera_state_machine() {
    // return;
    // ns_delay_us(1000);
    if (inferenceRunning) {
        return;
    }
    switch (camera_state) {
    case WAITING_FOR_JPG_SHUTTER:
        if (pictureTaken) {
            // ns_lp_printf("JPG %d Picture taken %d\n", dmaTarget, toc());
            if ((dmaTarget == 1) && (jpgReady1)) {
                // ns_lp_printf("JPG 1 Idled\n");
                camera_state = CAMERA_IDLE_JPG_PING;
            } else if ((dmaTarget == 2) && (jpgReady2)){
                // ns_lp_printf("JPG 2 Idled\n");
                camera_state = CAMERA_IDLE_JPG_PONG;
            } else {
                // tic();
                // ns_lp_printf("JPG %d Starting DMA %d\n", dmaTarget, toc());
                uint32_t buffer_length = start_jpg_dma(dmaTarget);
                pictureTaken = false;
                if (dmaTarget == 1) {
                    dmaTarget = 2;
                    jpg_buffer_length1 = buffer_length;
                    // ns_lp_printf("Set dmaTarget to 2, %d\n", dmaTarget); 
                    camera_state = WAITING_FOR_JPG_DMA_PING;
                } else {
                    dmaTarget = 1;
                    jpg_buffer_length2 = buffer_length;
                    camera_state = WAITING_FOR_JPG_DMA_PONG;
                }
            }
        }
        break;
    case WAITING_FOR_RGB_SHUTTER:
        if (pictureTaken) {
            // ns_lp_printf("RGB %d Picture taken %d\n", dmaTarget, toc());

            if ((dmaTarget == 1) && (rgbReady1)) {
                // ns_lp_printf("RGB 1 Idled\n");
                camera_state = CAMERA_IDLE_RGB_PING;
            } else if ((dmaTarget == 2) && (rgbReady2)){
                // ns_lp_printf("RGB 2 Idled\n");
                camera_state = CAMERA_IDLE_RGB_PONG;
            } else {
                // tic();
                uint32_t buffer_length = start_rgb_dma(dmaTarget);
                pictureTaken = false;
                if (dmaTarget == 1) {
                    dmaTarget = 2;
                    rgb_buffer_length1 = buffer_length;
                    // ns_lp_printf("Set dmaTarget to 2, %d\n", dmaTarget); 
                    camera_state = WAITING_FOR_RGB_DMA_PING;
                } else {
                    dmaTarget = 1;
                    rgb_buffer_length2 = buffer_length;
                    camera_state = WAITING_FOR_RGB_DMA_PONG;
                }
            }
        }
        break;
    case WAITING_FOR_JPG_DMA_PING:
        if (dmaComplete) {
            // ns_lp_printf("JPG 1 DMA Complete %d\n", toc());
            dmaComplete = false;
            if (jpgReady1) {
                ns_lp_printf("Warning: JPG1 overwritten\n");
            }
            jpgReady1 = true; // signal to CPU FSM
            // Take next picture
            press_jpg_shutter_button(&camera_config);
            // ns_lp_printf("JPG 1 Shutter Pressed %d\n", toc());

            camera_state = WAITING_FOR_JPG_SHUTTER;
        }
        break;
    case WAITING_FOR_JPG_DMA_PONG:
        if (dmaComplete) {
            // ns_lp_printf("JPG 2 DMA Complete %d\n", toc());
            dmaComplete = false;
            if (jpgReady2) {
                ns_lp_printf("Warning: JPG2 overwritten\n");
            }
            jpgReady2 = true; // signal to CPU FSM

            // Take next picture
            // ns_lp_printf("JPG 2 Shutter Pressed %d\n", toc());
            press_jpg_shutter_button(&camera_config);
            camera_state = WAITING_FOR_JPG_SHUTTER;
        }
        break;
    case WAITING_FOR_RGB_DMA_PING:
        // ns_lp_printf("RGB 1 DMA Complete %d\n", toc());
        if (dmaComplete) {
            dmaComplete = false;
            if (rgbReady1) {
                ns_lp_printf("Warning: RGB1 overwritten\n");
            }
            rgbReady1 = true; // signal to CPU FSM
            // Take next picture
            press_rgb_shutter_button(&camera_config);
            camera_state = WAITING_FOR_RGB_SHUTTER;
        }
        break;
    case WAITING_FOR_RGB_DMA_PONG:
        // ns_lp_printf("RGB 2 DMA Complete %d\n", toc());
        if (dmaComplete) {
            dmaComplete = false;
            if (rgbReady2) {
                ns_lp_printf("Warning: RGB2 overwritten\n");
            }
            rgbReady2 = true; // signal to CPU FSM

            // Take next picture
            press_rgb_shutter_button(&camera_config);
            camera_state = WAITING_FOR_RGB_SHUTTER;
        }
        break;
    case CAMERA_IDLE_JPG_PING:
        // Waiting for the last picture to be processed
        if (jpgReady1) {
            // CPU loop hasn't consumed last buffer, so wait
            // ns_lp_printf("Warning1: CPU loop hasn't consumed last buffer\n");
            camera_state = CAMERA_IDLE_JPG_PING;
        } else {
            // OK to DMA the next picture
            camera_state = WAITING_FOR_JPG_SHUTTER;
        }
        break;
    case CAMERA_IDLE_JPG_PONG:
        // Waiting for the last picture to be processed
        if (jpgReady2) {
            // CPU loop hasn't consumed last buffer, so wait
            // ns_lp_printf("Warning2: CPU loop hasn't consumed last buffer\n");
            camera_state = CAMERA_IDLE_JPG_PONG;
        } else {
            // OK to DMA the next picture
            camera_state = WAITING_FOR_JPG_SHUTTER;
        }
        break;
    case CAMERA_IDLE_RGB_PING:
        // Waiting for the last picture to be processed
        if (rgbReady1) {
            // CPU loop hasn't consumed last buffer, so wait
            // ns_lp_printf("RGB 1 Warning: CPU loop hasn't consumed last buffer\n");
            camera_state = CAMERA_IDLE_RGB_PING;
        } else {
            // OK to DMA the next picture
            camera_state = WAITING_FOR_RGB_SHUTTER;
        }
        break;
    case CAMERA_IDLE_RGB_PONG:
        // Waiting for the last picture to be processed
        if (rgbReady2) {
            // CPU loop hasn't consumed last buffer, so wait
            // ns_lp_printf("RGB 2 Warning: CPU loop hasn't consumed last buffer\n");
            camera_state = CAMERA_IDLE_RGB_PONG;
        } else {
            // OK to DMA the next picture
            camera_state = WAITING_FOR_RGB_SHUTTER;
        }
        break;
    default:
        break;
    }
}

void picture_dma_complete(ns_camera_config_t *cfg) {
    // ns_lp_printf("DMA Complete CB\n");
    dmaComplete = true;
    camera_state_machine();
}

void picture_taken_complete(ns_camera_config_t *cfg) {
    pictureTaken = true;
    // ns_lp_printf("Picture taken CB\n");
    camera_state_machine();
}

void tic() {
    uint32_t oldTime = elapsedTime;
    elapsedTime = ns_us_ticker_read(&ei_tickTimer); 
    if (elapsedTime == oldTime) {
        // We've saturated the timer, reset it
        ns_timer_clear(&ei_tickTimer);
    }
    // ns_lp_printf("TIC %d\n", elapsedTime);
}
uint32_t toc() { return ns_us_ticker_read(&ei_tickTimer) - elapsedTime; }

void press_rgb_shutter_button(ns_camera_config_t *cfg) {
    // ns_lp_printf("RGB Pressing RGB Shutter\n");
    camera_config.imageMode = NS_CAM_IMAGE_MODE_96X96;
    camera_config.imagePixFmt = NS_CAM_IMAGE_PIX_FMT_RGB565;   
    int err = ns_press_shutter_button(cfg);
    // ns_lp_printf("RGB Pressed RGB Shutter\n");
}

void press_jpg_shutter_button(ns_camera_config_t *cfg) {
    // ns_lp_printf("JPG Pressing JPG Shutter\n");
    camera_config.imageMode = NS_CAM_IMAGE_MODE_320X320;
    camera_config.imagePixFmt = NS_CAM_IMAGE_PIX_FMT_JPEG; 
    int err = ns_press_shutter_button(cfg);
}

static uint32_t start_jpg_dma(uint8_t bufferId) {
    // ns_lp_printf("Starting JPG DMA\n");
    uint32_t camLength = 0;
    if (bufferId == 1) {
        // ns_lp_printf("Starting JPG DMA 1 to addr 0x%x\n", jpgBuffer1);
        camLength = ns_start_dma_read(&camera_config, jpgBuffer1, &bufferOffset, JPG_BUFF_SIZE);
    } else {
        // ns_lp_printf("Starting JPG DMA 2 to addr 0x%x\n", jpgBuffer2);
        camLength = ns_start_dma_read(&camera_config, jpgBuffer2, &bufferOffset, JPG_BUFF_SIZE);
    }
    bufferOffset = 1;
    return camLength;
}

static uint32_t start_rgb_dma(uint8_t bufferId) {
    // ns_lp_printf("Starting RGB DMA\n");
    uint32_t camLength = 0;
    if (bufferId == 1) {
        // ns_lp_printf("RGB 1 Starting DMA to addr 0x%x\n", jpgBuffer1);
        camLength = ns_start_dma_read(&camera_config, rgbBuffer1, &bufferOffset, RGB_BUFF_SIZE);
    } else {
        // ns_lp_printf("RGB 2 Starting DMA to addr 0x%x\n", jpgBuffer2);
        camLength = ns_start_dma_read(&camera_config, rgbBuffer2, &bufferOffset, RGB_BUFF_SIZE);
    }
    bufferOffset = 0;
    return camLength;
}

uint32_t chop_off_trailing_zeros(uint8_t *buff, uint32_t length) {
    uint32_t index;
    // ns_lp_printf("Chopping off trailing zeros len %d addr 0x%x\n", length, buff);
    for (index = length - 1; index >= 0; index--) {
        if (buff[index] != 0) {
            break;
        }
    }

    if ((index + 1) == 0) {
        return 0;
    }
    // ns_lp_printf("Chopped length %d\n", index + 1);
    return index + 1;
}

uint32_t fpsElapsed = 0;

static void render_image(uint32_t camLength, uint8_t *buff, uint32_t pixFmt) {
    // Max xfer is 512, so we have to chunk anything bigger
    int remaining = camLength;
    int offset = bufferOffset;
    if (pixFmt == 0) {
        offset = 0;
    }
    ns_lp_printf("Rendering image len = %d buff is 0x%x, offset is %d\n",camLength, buff, offset);
    // ns_lp_printf("web available %d\n",tud_vendor_write_available()); 

    // Calculate FPS and restart timer
    // uint32_t newTime = ns_us_ticker_read(&ei_tickTimer);
    // uint32_t elapsed = newTime - fpsElapsed;
    // fpsElapsed = newTime;
    // ns_lp_printf("FPS: %d\n", (100000000 / elapsed));
    // tic();

    while (remaining > 0) {
        usb_data_t data;
        if (offset == bufferOffset) {
            // ns_lp_printf("First chunk\n"); 
            data.descriptor = FIRST_CHUNK;
            // print first 10 bytes
            // for (int i = 0; i < 10; i++) {
            //     ns_lp_printf("0x%x ", buff[i]);
            // }
            // ns_lp_printf("\n");
        } else {
            // ns_lp_printf("Middle chunk\n");
            data.descriptor = MIDDLE_CHUNK;
        };

        // data.mode = camera_config.imagePixFmt == NS_CAM_IMAGE_PIX_FMT_JPEG ? 1 : 0;
        if (pixFmt == 0) {
            data.mode = 0; // rgb
        } else {
            data.mode = 1;
        }


        int chunkSize = remaining > MAX_WEBUSB_CHUNK ? MAX_WEBUSB_CHUNK : remaining;
        memcpy(data.buffer, &buff[offset], chunkSize);
        remaining -= chunkSize;
        offset += chunkSize;
        if (remaining == 0) {
            // ns_lp_printf("Last chunk\n");
            data.descriptor = LAST_CHUNK;
        }

        webusb_send_data((uint8_t *)&data, chunkSize + WEBUSB_HEADER_SIZE);
        int numr = 0;

        while (tud_vendor_write_available() < MAX_WEBUSB_CHUNK) {
            ns_delay_us(400);
            // ns_lp_printf("+");
            if (numr++ > 20) {
                // ns_lp_printf("-%d\n ",tud_vendor_write_available() );
                numr = 0;
                tud_vendor_write_flush();
                break;
            }
        }
        ns_delay_us(400);

    }
    tud_vendor_write_flush();
}

uint32_t inferenceTimePerFrame = 0;
uint32_t decodeTimePerFrame = 0;

void render_result(ei_impulse_result_t *inf) {

    if (inf == NULL) {
        ns_lp_printf("No results\n");
        return;
    }

    // Send metadata
    usb_metadata_t metadata;
    metadata.descriptor = 3;
    metadata.decode_time = decodeTimePerFrame/1000;
    // ns_lp_printf("Decode time per frame: %d\n", decodeTimePerFrame);
    metadata.inference_latency = inferenceTimePerFrame/1000;
    metadata.microjoules_per_inference = 266; // TODO: Get this from the energy monitor
    int result_index = 0;

    for (size_t ix = 0; ix < inf->bounding_boxes_count; ix++) {
        auto bb = inf->bounding_boxes[ix];
        if (bb.value == 0.0f) {
            break;
        } 
        else if (ix > MAX_RESULTS) {
            break;
        }
        else {
            ns_lp_printf("bb.value %f\n", bb.value);
        }
        
        strncpy(metadata.results[result_index].id, bb.label, 20);
        metadata.results[result_index].x = bb.x;
        metadata.results[result_index].y = bb.y;
        metadata.results[result_index].height = bb.height;
        metadata.results[result_index].width = bb.width;
        metadata.results[result_index].confidence = bb.value * 100;
        result_index++;
    }
    metadata.num_results = result_index;
        ns_lp_printf("Inference latency: %d, num_results %d, avail %d\n", metadata.inference_latency, result_index, tud_vendor_write_available());
    ns_lp_printf("Meta number of  results: %d, descriptor %d\n", metadata.num_results, metadata.descriptor);
    if (metadata.num_results > 0) {
        ns_lp_printf("Inference latency: %d, num_results %d, avail %d\n", metadata.inference_latency, result_index, tud_vendor_write_available());
        // print first 10 bytes of metadata array
        for (int i = 0; i < 10; i++) {
            ns_lp_printf("0x%x ", ((uint8_t *)&metadata)[i]);
        }
        ns_lp_printf("\n");
        for (int i = 0; i < 10; i++) {
            ns_lp_printf("%d ", ((uint8_t *)&metadata)[i]);
        }
        ns_lp_printf("\n");

    }
    webusb_send_data((uint8_t *)&metadata, sizeof(usb_metadata_t));
    tud_vendor_write_flush();
    int numr = 0;

    while (tud_vendor_write_available() < MAX_WEBUSB_FRAME) {
        ns_delay_us(400);
        // ns_lp_printf(".");
        if (numr++ > 100) {
                // ns_lp_printf("`%d\n ",tud_vendor_write_available() );
                numr = 0;
                tud_vendor_write_flush();
                break;
        }
    }
    ns_delay_us(400);
}

typedef struct {
    int8_t contrast;
    int8_t brightness;
    int8_t ev;
} cam_settings_t;

uint8_t mapCameraValuesToArducamScale(int8_t in) {
    // 0 is 0, negative numbers map to positive even uints, positive map to odd
    if (in > 3) {
        in = 3;
    } else if (in < -3) {
        in = -3;
    }

    if (in == 0) {
        return 0;
    } else if (in < 0) {
        return abs(in) * 2;
    } else {
        return (abs(in) * 2) - 1;
    }
}

void setCameraSettings(ArducamCamera *camera, int8_t contrast, int8_t brightness, int8_t ev) {
    setContrast(camera, (CAM_CONTRAST_LEVEL)mapCameraValuesToArducamScale(contrast));
    setBrightness(camera, (CAM_BRIGHTNESS_LEVEL) mapCameraValuesToArducamScale(brightness));
    setEV(camera, (CAM_EV_LEVEL) mapCameraValuesToArducamScale(ev));
}

void msgReceived(const uint8_t *buffer, uint32_t length, void *args) {
    // Do something with the received message (we shouldn't get one, but need the cb)
    
    // The message contains information about how to set the camera
    cam_settings_t settings;
    settings.contrast = buffer[0];
    settings.brightness = buffer[1];
    settings.ev = buffer[2];
    // oldPrettyMode = prettyMode;
    prettyMode = buffer[3];
    ns_lp_printf("Received camera settings: contrast %d, brightness %d, ev %d\n", settings.contrast, settings.brightness, settings.ev);
    setCameraSettings(&camera, settings.contrast, settings.brightness, settings.ev);

    ns_lp_printf("Received %d bytes: %s\n", length, buffer);
}

void r565_to_rgb(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t r5 = (color & 0xF800) >> 11;
    uint8_t g6 = (color & 0x07E0) >> 5;
    uint8_t b5 = (color & 0x001F);
    *r = ( r5 * 527 + 23 ) >> 6;
    *g = ( g6 * 259 + 33 ) >> 6;
    *b = ( b5 * 527 + 23 ) >> 6;
}

static bool debug_nn = false;
static bool inferOnRgb1 = true;
// #define FEAT_TEST
int ei_get_data(size_t offset, size_t length, float *out_ptr) {
    uint32_t outindex = 0;
    for (int i = offset; i < offset+length; i++) {
        #ifdef FEAT_TEST
            uint32_t intpix = features[offset + i];
            float cpix = (intpix&0xff0000) + (intpix&0x00ff00) + (intpix&0x0000ff);
            float pix = features[offset + i];
            if (i < 10) {
                ns_lp_printf("Feature %d: orig 0x%x, 0x%x, %f, cpix %f\n", i, intpix, pix, pix, cpix);
            }
        #else
        #ifdef GRAYSCALE
            float pix = (grayScaleBuffer[offset + i]<<16)+(grayScaleBuffer[offset + i]<<8)+grayScaleBuffer[offset + i];
        #else
            // Convert RGB565 to RGB888 to pix
            // Grab the 565 pixel at offset + i
            uint16_t pixel;
            if (inferOnRgb1) {
                pixel = (rgbBuffer1[i * 2 +1] << 8) | rgbBuffer1[i * 2];
            } else {
                pixel = (rgbBuffer2[i * 2 +1] << 8) | rgbBuffer2[i * 2];
            }
            // uint16_t pixel = (rgbBuffer1[i * 2 +1] << 8) | rgbBuffer1[i * 2];
            uint8_t r, g, b;
            r565_to_rgb(pixel, &r, &g, &b);
            float pix = (r << 16) + (g << 8) + b;
            // if (i<10) {
            //     ns_lp_printf("Pixel %d: 0x%x, r %d, g %d, b %d, pix %f\n", i, pixel, r, g, b, pix);
            // }
        #endif
        #endif
        out_ptr[outindex++] = pix;
    }
    return 0;
}

static void checkGuards() {
    if (jp1Guard != 0xdeadbeef) {
        ns_lp_printf("JPG1 Guard failed, val %d\n", jp1Guard);
    }
    if (jp2Guard != 0xdeadbeef) {
        ns_lp_printf("JPG2 Guard failed, val %d\n", jp2Guard);
    }
    if (rgb1Guard != 0xdeadbeef) {
        ns_lp_printf("RGB1 Guard failed val %d\n", rgb1Guard);
    }
    if (rgb2Guard != 0xdeadbeef) {
        ns_lp_printf("RGB2 Guard failed val %d\n" , rgb2Guard);
    }
        jp1Guard = 0xdeadbeef;
    jp2Guard = 0xdeadbeef;
    rgb1Guard = 0xdeadbeef;
    rgb2Guard = 0xdeadbeef;
}

int main(void) {
    // int err;
    ns_core_config_t ns_core_cfg = {.api = &ns_core_V1_0_0};
    usb_handle_t usb_handle = NULL;

    // Power configuration (mem, cache, peripherals, clock)
    NS_TRY(ns_core_init(&ns_core_cfg), "Core init failed.\n");
    NS_TRY(ns_power_config(&ns_pwr_config), "Power config failed.\n");
    am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);

    ns_itm_printf_enable();
    ns_interrupt_master_enable();

    elapsedTime = 0;
    NS_TRY(ns_timer_init(&ei_tickTimer), "Timer Init Failed\n");

    // NS_TRY(ns_peripheral_button_init(&button_config), "Button Init Failed\n");

    // WebUSB Setup
    webusb_register_raw_cb(msgReceived, NULL);
    webUsbConfig.rx_buffer = my_rx_ff_buf;
    webUsbConfig.rx_bufferLength = MY_RX_BUFSIZE;
    webUsbConfig.tx_buffer = my_tx_ff_buf;
    webUsbConfig.tx_bufferLength = MY_TX_BUFSIZE;
    NS_TRY(ns_usb_init(&webUsbConfig, &usb_handle), "USB Init Failed\n");
    ns_lp_printf("USB Init Success\n");

    // Camera Setup
    NS_TRY(ns_camera_init(&camera_config), "Camera Stop Failed\n");

    ns_lp_printf("Camera Init Success\n");
    ns_stop_camera(&camera_config);

    // EI Init
    signal_t signal_fomo;
    
    // signal_fomo.total_length = impulse_517398_0.input_width * impulse_517398_0.input_height;
    signal_fomo.total_length = impulse_522036_0.input_width * impulse_522036_0.input_height;
    signal_fomo.get_data = &ei_get_data;
    ei_impulse_result_t result_fomo = { 0 };
    uint32_t camLength = 0;

    ns_lp_printf("📸 TinyVision Demo\n\n");

    ns_start_camera(&camera_config);
    setBrightness(&camera, CAM_BRIGHTNESS_LEVEL_DEFAULT);
    setEV(&camera, CAM_EV_LEVEL_DEFAULT);
    setContrast(&camera, CAM_CONTRAST_LEVEL_DEFAULT);
    setAutoExposure(&camera, true); 
    ns_delay_us(100000);

    // Kick off camera FSM
    ns_lp_printf("pressing shutter button for picture\n");
    prettyMode = true;
    oldPrettyMode = true;
    if (prettyMode) {
        camera_state = WAITING_FOR_JPG_SHUTTER;
        press_jpg_shutter_button(&camera_config);
    } else {
        camera_state = WAITING_FOR_RGB_SHUTTER; 
        press_rgb_shutter_button(&camera_config);
    }

    // press_jpg_shutter_button(&camera_config);

    app_state_e state = TAKING_JPG_IMAGE;
    EI_IMPULSE_ERROR err;
    bool bb_found;
    ei_impulse_result_bounding_box_t bb;
    bool firstTime = true;
    tic();

    // Set guards
    jp1Guard = 0xdeadbeef;
    jp2Guard = 0xdeadbeef;
    rgb1Guard = 0xdeadbeef;
    rgb2Guard = 0xdeadbeef;

    uint8_t next_jpg_pic = 1;
    uint8_t next_rgb_pic = 1;

    while (1) {
        if (oldPrettyMode != prettyMode) {
            ns_lp_printf("Switching modes from old %d to new %d\n", oldPrettyMode, prettyMode);
            // wait for camera state machie to quiesce
            while ((dmaComplete == false) && (camera_state < CAMERA_IDLE_JPG_PING)) {
                ns_delay_us(10);
            }
            pictureTaken = false;
            dmaComplete = false;
            decodeTimePerFrame = 0;

            // Switch modes
            if (prettyMode) {
                ns_lp_printf("Switching to pretty mode\n");
                jpgReady1 = false;
                jpgReady2 = false;
                camera_state = WAITING_FOR_JPG_SHUTTER;
                press_jpg_shutter_button(&camera_config);
            } else {
                ns_lp_printf("Switching to non-pretty mode\n");
                rgbReady1 = false;
                rgbReady2 = false;
                decodeTimePerFrame = 0;
                camera_state = WAITING_FOR_RGB_SHUTTER; 
                press_rgb_shutter_button(&camera_config);
            }
            oldPrettyMode = prettyMode;
        }
        if ( jpgReady1 || jpgReady2 || rgbReady1 || rgbReady2) {
            // JPG is just for rendering
            if (jpgReady1 || jpgReady2) {
                if ((jpgReady1) && (next_jpg_pic == 1)) {
                    // ns_lp_printf("JPG 1 Ready %d\n", toc());
                    uint32_t my_buffer_length = chop_off_trailing_zeros(jpgBuffer1, jpg_buffer_length1); // Remove trailing zeros, calc new length
                    render_image(my_buffer_length, jpgBuffer1, 1);
                    // ns_lp_printf("JPG 1 Decode Starts %d\n", toc());
                    tic();
                    camera_decode_image(jpgBuffer1, my_buffer_length, rgbBuffer1, 96, 96, 3);
                    decodeTimePerFrame = toc();
                    // ns_lp_printf("JPG 1 Decode Ends %d\n", toc());
                    inferOnRgb1 = true;
                    jpgReady1 = false;
                    next_jpg_pic = 2;
                    camera_state_machine();

                    // ns_lp_printf("JPG 1 EI Starts. Set jpgRead1 %d and next_jpg_pic %d jpgReady2 %d\n", jpgReady1, next_jpg_pic, jpgReady2);
                    tic();
                    err = run_classifier(&impulse_handle_522036_0, &signal_fomo, &result_fomo, debug_nn);
                    inferenceTimePerFrame = toc();
                    render_result(&result_fomo);

                    // ns_lp_printf("JPG 1 Inference time was %d\n", inferenceTimePerFrame);
                    camera_state_machine();
                    camera_state_machine();

                    // render_image(18432, rgbBuffer1, 0); 
                } else if ((jpgReady2) && (next_jpg_pic == 2)) {
                    // ns_lp_printf("JPG 2 Ready %d\n", toc());

                    uint32_t my_buffer_length = chop_off_trailing_zeros(jpgBuffer2, jpg_buffer_length2); // Remove trailing zeros, calc new length
                    render_image(my_buffer_length, jpgBuffer2, 1);
                    tic();
                    camera_decode_image(jpgBuffer2, my_buffer_length, rgbBuffer2, 96, 96, 3);
                    decodeTimePerFrame = toc();
                    inferOnRgb1 = false;
                    jpgReady2 = false;
                    next_jpg_pic = 1;
                    camera_state_machine();
                    tic();
                    // ns_lp_printf("JPG 2 Set jpgRead2 %d and next_jpg_pic %d, jpgRead1 %d\n", jpgReady2, next_jpg_pic, jpgReady1);
                    err = run_classifier(&impulse_handle_522036_0, &signal_fomo, &result_fomo, debug_nn);
                    inferenceTimePerFrame = toc();
                    render_result(&result_fomo);

                    // ns_lp_printf("JPG 2 Inference time was %d\n", inferenceTimePerFrame);
                    camera_state_machine();
                    camera_state_machine();
                }
                else {
                    // ns_lp_printf("jpgReady1 %d, jpgReady2 %d, next_jpg_pic %d, inferOnRgb1 %d \n", 
                        // jpgReady1, jpgReady2, next_jpg_pic, inferOnRgb1);
                }
            } else if (rgbReady1 || rgbReady2) {
                decodeTimePerFrame = 0;
                if ((rgbReady1) && (next_rgb_pic == 1)) {
                    // ns_lp_printf("RGB 1 Ready len %d %d\n", rgb_buffer_length1, toc());
                    render_image(rgb_buffer_length1, rgbBuffer1, 0);
                    inferOnRgb1 = false;
                    next_rgb_pic = 2;
                    rgbReady1 = false;
                    camera_state_machine();
                    tic();
                    inferenceRunning = true;
                    err = run_classifier(&impulse_handle_522036_0, &signal_fomo, &result_fomo, debug_nn);
                    inferenceRunning = false;
                    inferenceTimePerFrame = toc();
                    camera_state_machine();
                    render_result(&result_fomo);

                    // ns_lp_printf("RGB 1 Inference time was %d\n", inferenceTimePerFrame);
                    camera_state_machine();
                    camera_state_machine();
                }

                if ((rgbReady2) && (next_rgb_pic == 2)) {
                    // ns_lp_printf("RGB 2 Ready len %d %d\n", rgb_buffer_length2, toc());
                    render_image(rgb_buffer_length2, rgbBuffer2, 0);
                    inferOnRgb1 = false;
                    next_rgb_pic = 1;
                    rgbReady2 = false;
                    camera_state_machine();

                    tic();
                    inferenceRunning = true;
                    err = run_classifier(&impulse_handle_522036_0, &signal_fomo, &result_fomo, debug_nn);
                    inferenceRunning = false;
                    inferenceTimePerFrame = toc();
                    camera_state_machine();
                    render_result(&result_fomo);

                    // ns_lp_printf("RGB 2 Inference time was %d\n", inferenceTimePerFrame);
                    camera_state_machine();
                    camera_state_machine();
                }
            }
        } else {
            ns_deep_sleep();
        }
    }
}