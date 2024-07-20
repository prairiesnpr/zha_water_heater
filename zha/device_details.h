#include <stdint.h>
#include <XBee.h>
#include <zha_constants.h>

#define NUM_ENDPOINTS 3


static uint8_t* manuf = (uint8_t*)"iSilentLLC";

static attribute dev_basic_attr[] {{0x0004, manuf, 10, ZCL_CHAR_STR}, {0x0005, (uint8_t*)"Water Heater", 12, ZCL_CHAR_STR}};
static attribute in_temp_basic_attr[] {{0x0004, manuf, 10, ZCL_CHAR_STR}, {0x0005, (uint8_t*)"In Temp", 7, ZCL_CHAR_STR}};
static attribute out_temp_basic_attr[] {{0x0004, manuf, 10, ZCL_CHAR_STR}, {0x0005, (uint8_t*)"Out Temp", 8, ZCL_CHAR_STR}};

static attribute ssr_attr[] {{0x0000, 0x00, 1, ZCL_BOOL}};
static attribute metering_attr[] = {{INSTANTANEOUS_DEMAND, 0x0000, 2, ZCL_UINT16_T}};
static attribute in_temp_attr[] = {{0x0000, 0x0000, 2, ZCL_UINT16_T}};
static attribute out_temp_attr[] = {{0x0000, 0x0000, 2, ZCL_UINT16_T}};

//dev_basic_attr

static Cluster s_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, dev_basic_attr, 2), Cluster(ON_OFF_CLUSTER_ID, ssr_attr, 1), Cluster(METERING_CLUSTER_ID, metering_attr, 1)};
static Cluster i_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, in_temp_basic_attr, 2), Cluster(TEMP_CLUSTER_ID, in_temp_attr, 1)};
static Cluster o_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, out_temp_basic_attr, 2), Cluster(TEMP_CLUSTER_ID, out_temp_attr, 1)};

static Cluster out_clusters[] = {};
static Endpoint ENDPOINTS[NUM_ENDPOINTS] = {
  Endpoint(1, ON_OFF_OUTPUT, s_in_clusters, out_clusters, 3, 0),
  Endpoint(2, TEMPERATURE_SENSOR, i_in_clusters, out_clusters, 2, 0),
  Endpoint(3, TEMPERATURE_SENSOR, o_in_clusters, out_clusters, 2, 0),
};

