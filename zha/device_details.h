#include <stdint.h>

#define NUM_ENDPOINTS 3

constexpr uint8_t one_zero_byte[] = {0x00};
constexpr uint8_t one_max_byte[] = {0xFF};
constexpr uint8_t two_zero_byte[] = {0x00, 0x00};
constexpr uint8_t four_zero_byte[] = {0x00, 0x00, 0x00, 0x00};

// Reuse these to save SRAM
#define manufacturer "iSilentLLC"
#define wh_model "Water Heater"

attribute BuildStringAtt(uint16_t a_id, char *value, uint8_t size, uint8_t a_type)
{
    uint8_t *value_t = (uint8_t *)value;
    return attribute(a_id, value_t, size, a_type, 0x01);
}

const attribute manuf_attr = BuildStringAtt(MANUFACTURER_ATTR, const_cast<char *>(manufacturer), sizeof(manufacturer), ZCL_CHAR_STR);
const attribute wh_model_attr = BuildStringAtt(MODEL_ATTR, const_cast<char *>(wh_model), sizeof(wh_model), ZCL_CHAR_STR);

attribute wh_basic_attr[] = {
    manuf_attr,
    wh_model_attr,
};

attribute ssr_attr[]{{CURRENT_STATE, const_cast<uint8_t *>(one_zero_byte), sizeof(one_zero_byte), ZCL_BOOL}};

attribute metering_attr[] = {
    {MEASUREMENT_TYPE, const_cast<uint8_t *>(four_zero_byte), sizeof(four_zero_byte), ZCL_MAP32}, // Measurement Type
    {RMS_CURRENT_ATTR, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_UINT16_T},
    {RMS_VOLTAGE_ATTR, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_UINT16_T},
    {RMS_VOLTAGE_MAX, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_UINT16_T},
    {AC_FREQUENCY_ATTR, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_UINT16_T},
    {AC_FREQUENCY_MAX, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_UINT16_T},
};

attribute in_temp_attr[] = {{CURRENT_STATE, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_INT16_T}};
attribute out_temp_attr[] = {{CURRENT_STATE, const_cast<uint8_t *>(two_zero_byte), sizeof(two_zero_byte), ZCL_INT16_T}};

// dev_basic_attr

Cluster s_in_clusters[] = {Cluster(BASIC_CLUSTER_ID, wh_basic_attr, sizeof(wh_basic_attr) / sizeof(*wh_basic_attr)),
                           Cluster(ON_OFF_CLUSTER_ID, ssr_attr, sizeof(ssr_attr) / sizeof(*ssr_attr)),
                           Cluster(ELECTRICAL_MEASUREMENT, metering_attr, sizeof(metering_attr) / sizeof(*metering_attr))};
Cluster i_in_clusters[] = {Cluster(TEMP_CLUSTER_ID, in_temp_attr, sizeof(in_temp_attr) / sizeof(*in_temp_attr))};
Cluster o_in_clusters[] = {Cluster(TEMP_CLUSTER_ID, out_temp_attr, sizeof(out_temp_attr) / sizeof(*out_temp_attr))};

Cluster out_clusters[] = {};
Endpoint ENDPOINTS[NUM_ENDPOINTS] = {
    Endpoint(1, ON_OFF_OUTPUT, s_in_clusters, out_clusters, sizeof(s_in_clusters) / sizeof(*s_in_clusters), 0),
    Endpoint(2, TEMPERATURE_SENSOR, i_in_clusters, out_clusters, sizeof(i_in_clusters) / sizeof(*i_in_clusters), 0),
    Endpoint(3, TEMPERATURE_SENSOR, o_in_clusters, out_clusters, sizeof(o_in_clusters) / sizeof(*o_in_clusters), 0),
};
