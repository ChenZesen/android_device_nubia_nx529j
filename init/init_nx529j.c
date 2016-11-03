/*
   Copyright (c) 2014, The Linux Foundation. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <sys/sysinfo.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

#include "init_msm.h"

#define VIRTUAL_SIZE "/sys/class/graphics/fb0/virtual_size"
#define BOARD_PLATFORM_SUBTYPE "/sys/devices/soc0/platform_subtype_id"
#define BUF_SIZE 64

char const *heapstartsize;
char const *heapgrowthlimit;
char const *heapsize;
char const *heapminfree;

void init_msm_properties(unsigned long msm_id, unsigned long msm_ver, char *board_type)
{
    char platform[PROP_VALUE_MAX];
    int rc;
    unsigned long virtual_size = 0;
    char str[BUF_SIZE];
    unsigned long subtype_id = -1;
    const unsigned long POLARIS_ID = 64;

    UNUSED(msm_id);
    UNUSED(msm_ver);

    rc = property_get("ro.board.platform", platform);
    if (!rc || !ISMATCH(platform, ANDROID_TARGET)){
        return;
    }

    if (strncmp(board_type, "QRD", 4) == 0) {
        rc = read_file2(BOARD_PLATFORM_SUBTYPE, str,
                        sizeof(str));
        if (rc) {
            subtype_id = strtoul(str, NULL, 0);
        }
        if (subtype_id == POLARIS_ID) {
            property_set(PROP_LCDDENSITY, "280");
            return;
        }
    }

    rc = read_file2(VIRTUAL_SIZE, str, sizeof(str));
    if (rc) {
        virtual_size = strtoul(str, NULL, 0);
    }

    if(virtual_size >= 1080) {
        property_set(PROP_LCDDENSITY, "480");
    } else if (virtual_size >= 720) {
        // For 720x1280 resolution
        property_set(PROP_LCDDENSITY, "320");
    } else if (virtual_size >= 480) {
        // For 480x854 resolution QRD.
        property_set(PROP_LCDDENSITY, "240");
    } else
        property_set(PROP_LCDDENSITY, "320");

    check_device();

    property_set("dalvik.vm.heapstartsize", heapstartsize);
    property_set("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    property_set("dalvik.vm.heapsize", heapsize);
    property_set("dalvik.vm.heaptargetutilization", "0.75");
    property_set("dalvik.vm.heapminfree", heapminfree);
    property_set("dalvik.vm.heapmaxfree", "8m");
}

void check_device()
{
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram > 2048ull * 1024 * 1024) {
        // from - phone-xxhdpi-3072-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "384m";
        heapsize = "1024m";
        heapminfree = "512k";
    } else {
        // from - phone-xxhdpi-2048-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heapminfree = "2m";
    }
}

