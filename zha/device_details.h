#include <stdint.h>

#define NUM_ENDPOINTS 3

static uint8_t *manuf = (uint8_t *)"iSilentLLC";

static attribute dev_basic_attr[]{{MANUFACTURER_ATTR, manuf, 10, ZCL_CHAR_STR},
                                  {MODEL_ATTR, (uint8_t *)"Water Heater", 12, ZCL_CHAR_STR}};
static attribute in_temp_basic_attr[]{{MANUFACTURER_ATTR, manuf, 10, ZCL_CHAR_STR},
                                      {MODEL_ATTR, (uint8_t *)"In Temp", 7, ZCL_CHAR_STR}};
static attribute out_temp_basic_attr[]{{MANUFACTURER_ATTR, manuf, 10, ZCL_CHAR_STR},
                                       {MODEL_ATTR, (uint8_t *)"Out Temp", 8, ZCL_CHAR_STR}};

static attribute ssr_attr[]{{CURRENT_STATE, 0x00, 1, ZCL_BOOL}};
static attribute metering_attr[] = {
    {MEASUREMENT_TYPE, 0x00000000, 4, ZCL_MAP32}, // Measurement Type
    {RMS_CURRENT, 0x0000, 2, ZCL_UINT16_T},
    {RMS_VOLTAGE, 0x0000, 2, ZCL_UINT16_T},
    {RMS_VOLTAGE_MAX, 0x0000, 2, ZCL_UINT16_T},
    {AC_FREQUENCY, 0x0000, 2, ZCL_UINT16_T},
    {AC_FREQUENCY_MAX, 0x0000, 2, ZCL_UINT16_T}};
static attribute in_temp_attr[] = {{CURRENT_STATE, 0x0000, 2, ZCL_INT16_T}};
static attribute out_temp_attr[] = {{CURRENT_STATE, 0x0000, 2, ZCL_INT16_T}};

// dev_basic_attr

static Cluster s_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, dev_basic_attr, sizeof(dev_basic_attr) / sizeof(*dev_basic_attr)),
                                  Cluster(ON_OFF_CLUSTER_ID, ssr_attr, sizeof(ssr_attr) / sizeof(*ssr_attr)),
                                  Cluster(ELECTRICAL_MEASUREMENT, metering_attr, sizeof(metering_attr) / sizeof(*metering_attr))};
static Cluster i_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, in_temp_basic_attr, sizeof(in_temp_basic_attr) / sizeof(*in_temp_basic_attr)),
                                  Cluster(TEMP_CLUSTER_ID, in_temp_attr, sizeof(in_temp_attr) / sizeof(*in_temp_attr))};
static Cluster o_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, out_temp_basic_attr, sizeof(out_temp_basic_attr) / sizeof(*out_temp_basic_attr)),
                                  Cluster(TEMP_CLUSTER_ID, out_temp_attr, sizeof(out_temp_attr) / sizeof(*out_temp_attr))};

static Cluster out_clusters[] = {};
static Endpoint ENDPOINTS[NUM_ENDPOINTS] = {
    Endpoint(1, ON_OFF_OUTPUT, s_in_clusters, out_clusters, sizeof(s_in_clusters) / sizeof(*s_in_clusters), 0),
    Endpoint(2, TEMPERATURE_SENSOR, i_in_clusters, out_clusters, sizeof(i_in_clusters) / sizeof(*i_in_clusters), 0),
    Endpoint(3, TEMPERATURE_SENSOR, o_in_clusters, out_clusters, sizeof(o_in_clusters) / sizeof(*o_in_clusters), 0),
};
