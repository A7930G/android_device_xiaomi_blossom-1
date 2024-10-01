#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

PRODUCT_MAKEFILES := \
    $(LOCAL_DIR)/voltage_blossom.mk
    $(LOCAL_DIR)/lineage_blossom.mk

COMMON_LUNCH_CHOICES := \
    voltage_blossom-user \
    voltage_blossom-userdebug \
    voltage_blossom-eng
    lineage_blossom-user \
    lineage_blossom-userdebug \
    lineage_blossom-eng

