DEVICE_GATEWAY_VERSION = 1.0
DEVICE_GATEWAY_SITE = $(abspath $(BR2_EXTERNAL_NRF_STM_PATH)/..)
DEVICE_GATEWAY_SITE_METHOD = local
DEVICE_GATEWAY_SUPPORTS_IN_SOURCE_BUILD = NO
DEVICE_GATEWAY_CONF_OPTS = -DBUILD_TESTING=OFF
DEVICE_GATEWAY_OVERRIDE_SRCDIR_RSYNC_EXCLUSIONS = \
    --exclude=/build/ \
    --exclude=/buildroot-external/
define DEVICE_GATEWAY_INSTALL_INIT_SYSV
    $(INSTALL) -D -m 0755 $(BR2_EXTERNAL_NRF_STM_PATH)/package/device-gateway/S60device-gateway \
        $(TARGET_DIR)/etc/init.d/S60device-gateway
endef
$(eval $(cmake-package))
