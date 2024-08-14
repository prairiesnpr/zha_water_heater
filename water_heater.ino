#include <SoftwareSerial.h>
#include <arduino-timer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
#include <EmonLib.h>
#include <xbee_zha.h>
#include "zha/device_details.h"

#define WATER_TEMP_BUS 7
#define SSR_PIN 5
#define AMP_PIN 16

#define SW_AMP_ENDPOINT 1
#define IN_TEMP_ENDPOINT 2
#define OUT_TEMP_ENDPOINT 3

// Define SoftSerial TX/RX pins
#define ssRX 10
#define ssTX 11

#define START_LOOPS 100

uint8_t start_fails = 0;
uint8_t init_status_sent = 0;

void (*resetFunc)(void) = 0;

auto sensor_timer = timer_create_default(); // create a timer with default settings
auto state_timer = timer_create_default();  // create a timer with default settings

unsigned long loop_time = millis();
unsigned long last_msg_time = loop_time - 1000;

EnergyMonitor emon1;

// One wire temp sensors
OneWire oneWire(WATER_TEMP_BUS);
DallasTemperature sensors(&oneWire);
// Water Input Temp
DeviceAddress inputThermometer = {0x28, 0xDB, 0x8F, 0x77, 0x91, 0x15, 0x02, 0x84};
// Water Output Temp
DeviceAddress outputThermometer = {0x28, 0x7D, 0xA3, 0x77, 0x91, 0x0D, 0x02, 0x29};

SoftwareSerial nss(ssRX, ssTX);

void setup()
{
  pinMode(SSR_PIN, OUTPUT);
  pinMode(AMP_PIN, INPUT);

  Serial.begin(9600);
  Serial.println(F("Startup"));
  sensors.begin();
  nss.begin(9600);

  Serial.print(F("Found "));
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(F(" Temp dev."));

  sensors.setResolution(inputThermometer, 9);
  sensors.setResolution(outputThermometer, 9);

  // emon init
  // emon1.current(AMP_PIN, 111.1);    // Current: input pin, calibration.
  emon1.current(AMP_PIN, 0);

  zha.Start(nss, zhaClstrCmd, zhaWriteAttr, NUM_ENDPOINTS, ENDPOINTS);

  // Set up callbacks
  zha.registerCallbacks(atCmdResp, zbTxStatusResp, otherResp, zdoReceive);

  Serial.println(F("CB Conf"));

  // Ensure on power failure that we turn the water heater on
  digitalWrite(SSR_PIN, HIGH);

  sensor_timer.every(30000, update_sensors); // 30 seconds
  state_timer.every(6000000, update_state);  // 10 min

  wdt_enable(WDTO_8S);
}

bool read_temp(DeviceAddress thermometer, int16_t *temp)
{
  bool success = sensors.requestTemperaturesByAddress(thermometer);

  if (!success)
  {
    Serial.println(F("T addr not found"));
    return false;
  }

  delay(10); // No idea why this is needed

  float TempC = sensors.getTempC(thermometer);
  if (TempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println(F("T Disconnect"));
    return false;
  }

  Serial.print(F("Temp: "));
  Serial.print(String(TempC, 2));
  Serial.println(F("°C"));

  *temp = (int16_t)(TempC * 100.0);
  return true;
}

bool update_sensors(void *)
{
  update_temp();
  update_amps();
  return true;
}

bool update_state(void *)
{
  update_switch_state();
  return true;
}

void update_amps()
{
  // Update amps
  double Irms = emon1.calcIrms(1480); // Calculate Irms only
  Serial.print(Irms * 240.0);         // Apparent power
  Serial.print(" ");
  Serial.println(Irms); // Irms

  Endpoint end_point = zha.GetEndpoint(SW_AMP_ENDPOINT);
  Cluster cluster = end_point.GetCluster(ELECTRICAL_MEASUREMENT);
  attribute *attr = cluster.GetAttr(RMS_CURRENT);
  uint16_t cor_t = (uint16_t)(Irms * 240.0);

  attr->SetValue(cor_t);
  zha.sendAttributeRpt(cluster.id, attr, end_point.id, 1);
}

void update_switch_state()
{
  // Update our current switch state
  Endpoint end_point = zha.GetEndpoint(SW_AMP_ENDPOINT);
  Cluster cluster = end_point.GetCluster(ON_OFF_CLUSTER_ID);
  attribute *attr = cluster.GetAttr(CURRENT_STATE);
  uint8_t val = digitalRead(SSR_PIN);
  attr->SetValue(val);
  Serial.print(F("Cur St "));
  Serial.println(attr->GetIntValue());
  zha.sendAttributeRpt(cluster.id, attr, end_point.id, 1);
}

void update_temp()
{
  // In Temp
  int16_t res_temp;
  bool r_success = read_temp(inputThermometer, &res_temp);

  if (r_success && zha.dev_status == READY)
  {
    Endpoint in_end_point = zha.GetEndpoint(IN_TEMP_ENDPOINT);
    Cluster in_cluster = in_end_point.GetCluster(TEMP_CLUSTER_ID);
    attribute *in_attr = in_cluster.GetAttr(CURRENT_STATE);
    in_attr->SetValue(res_temp);
    zha.sendAttributeRpt(in_cluster.id, in_attr, in_end_point.id, 1);
  }

  // Out Temp
  r_success = read_temp(outputThermometer, &res_temp);

  if (r_success && zha.dev_status == READY)
  {
    Endpoint out_end_point = zha.GetEndpoint(OUT_TEMP_ENDPOINT);
    Cluster out_cluster = out_end_point.GetCluster(TEMP_CLUSTER_ID);
    attribute *out_attr = out_cluster.GetAttr(CURRENT_STATE);
    out_attr->SetValue(res_temp);
    zha.sendAttributeRpt(out_cluster.id, out_attr, out_end_point.id, 1);
  }
}

void SetAttr(uint8_t ep_id, uint16_t cluster_id, uint16_t attr_id, uint8_t value, uint8_t rqst_seq_id)
{
  Endpoint end_point = zha.GetEndpoint(ep_id);
  Cluster cluster = end_point.GetCluster(cluster_id);
  attribute *attr = cluster.GetAttr(attr_id);

  Serial.print("Clstr: ");
  Serial.println(cluster_id, HEX);

  if (cluster_id == ON_OFF_CLUSTER_ID)
  {
    if (value == 0x00)
    {
      Serial.print(F("Turn Off: "));
      Serial.println(end_point.id);
      digitalWrite(SSR_PIN, LOW);
    }
    else if (value == 0x01)
    {
      Serial.print(F("Turn On: "));
      Serial.println(end_point.id);
      digitalWrite(SSR_PIN, HIGH);
    }
    zha.sendAttributeCmdRsp(cluster_id, attr, ep_id, 1, value, zha.cmd_seq_id);
  }
}

void loop()
{
  zha.loop();

  if (zha.dev_status == READY)
  {
    if (init_status_sent)
    {
      if ((loop_time - last_msg_time) > 1000)
      {
        last_msg_time = millis();
      }
      // Nothing to do here, the timers take care of the updates
    }

    if (!init_status_sent)
    {
      Serial.println(F("Snd Init States"));
      init_status_sent = 1;
      update_switch_state();
    }
  }
  else if ((loop_time - last_msg_time) > 1000)
  {
    Serial.print(F("Not Started "));
    Serial.print(start_fails);
    Serial.print(F(" of "));
    Serial.println(START_LOOPS);

    last_msg_time = millis();
    if (start_fails > 15)
    {
      // Sometimes we don't get a response from dev ann, try a transmit and see if we are good
      update_switch_state();
    }
    if (start_fails > START_LOOPS)
    {
      resetFunc();
    }
    start_fails++;
  }
  sensor_timer.tick();
  state_timer.tick();
  wdt_reset();
  loop_time = millis();
}

void zhaWriteAttr(ZBExplicitRxResponse &erx)
{

  Serial.println(F("Write Cmd"));
  // No writable attributes
}

void zhaClstrCmd(ZBExplicitRxResponse &erx)
{
  Serial.println(F("Clstr Cmd"));
  if (erx.getDstEndpoint() == SW_AMP_ENDPOINT)
  {
    Serial.println(F("Door Ep"));
    if (erx.getClusterId() == ON_OFF_CLUSTER_ID)
    {
      Serial.println(F("ON/OFF Cl"));
      for (uint8_t i = erx.getDataOffset(); i < (erx.getDataLength() + erx.getDataOffset() + 3); i++)
      {
        Serial.print(erx.getFrameData()[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();

      if (erx.getFrameData()[erx.getDataOffset() + 2] == 0x00)
      {
        Serial.println(F("Cmd Off"));
        SetAttr(erx.getDstEndpoint(), erx.getClusterId(), CURRENT_STATE, erx.getFrameData()[erx.getDataOffset() + 2], erx.getFrameData()[erx.getDataOffset() + 1]);
      }
      else if (erx.getFrameData()[erx.getDataOffset() + 2] == 0x01)
      {
        Serial.println(F("Cmd On"));
        SetAttr(erx.getDstEndpoint(), erx.getClusterId(), CURRENT_STATE, erx.getFrameData()[erx.getDataOffset() + 2], erx.getFrameData()[erx.getDataOffset() + 1]);
      }
      else
      {
        Serial.print(F("Cmd Id: "));
        Serial.println(erx.getFrameData()[erx.getDataOffset() + 2], HEX);
      }
    }
  }
  else if (erx.getDstEndpoint() == IN_TEMP_ENDPOINT || erx.getDstEndpoint() == OUT_TEMP_ENDPOINT)
  {
    Serial.println(F("Temp Ep"));
  }
  else
  {
    Serial.println(F("Inv Ep"));
  }

  if (erx.getClusterId() == BASIC_CLUSTER_ID)
  {
    Serial.println(F("Basic Clstr"));
  }
}