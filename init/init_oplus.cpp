/*
 * Copyright (C) 2022-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;
using android::base::ReadFileToString;
using android::base::Split;
using android::base::Trim;

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

/*
 * Only for read-only properties. Properties that can be wrote to more
 * than once should be set in a typical init script (e.g. init.oplus.hw.rc)
 * after the original property has been set.
 */
void vendor_load_properties() {
    auto prjname_string = GetProperty("ro.boot.prjname", "0");
    int prjname = 0;
    char* end;
    long val;

        val = strtol(prjname_string.c_str(), &end, 10);

    if (*end != '\0') {
        LOG(ERROR) << "Invalid project name format: " << prjname_string;
        return;
    }

    prjname = static_cast<int>(val);

    switch (prjname) {
        case 21623: // spartan CN
            OverrideProperty("ro.product.product.model", "RMX3372");
            OverrideProperty("ro.product.product.device", "RE5477");
            OverrideProperty("ro.product.marketname", "realme Q5 Pro");
            break;
        case 21732: // spartan IN
        case 21733: // spartan EU
            OverrideProperty("ro.product.product.model", "RMX3371");
            OverrideProperty("ro.product.product.device", "RE54E4L1");
            OverrideProperty("ro.product.marketname", "realme GT NEO 3T");
            break;
        default:
            LOG(ERROR) << "Unexpected project name: " << prjname;
    }

    if (std::string content; ReadFileToString("/proc/devinfo/ddr_type", &content)) {
        OverrideProperty("ro.boot.ddr_type", Split(Trim(content), "\t").back().c_str());
    }
}
