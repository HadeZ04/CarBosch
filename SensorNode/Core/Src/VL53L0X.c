// Most of the functionality of this library is based on the VL53L0X API
// provided by ST (STSW-IMG005), and some of the explanatory comments are quoted
// or paraphrased from the API source code, API user manual (UM2039), and the
// VL53L0X datasheet.

#include <stdint.h>
#include "stm32f1xx_hal.h" // Change it for your requirements.
#include "string.h"
#include "VL53L0X.h"

//---------------------------------------------------------
// Local variables within this file (private)
//---------------------------------------------------------
uint8_t g_i2cAddr = ADDRESS_DEFAULT;
uint16_t g_ioTimeout = 0;  // no timeout
uint8_t g_isTimeout = 0;
uint16_t g_timeoutStartMs;
uint8_t g_stopVariable; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
uint32_t g_measTimBudUs;

#define I2C_TIMEOUT 100 // I2C timeout in ms
#define I2C_READ 1
#define I2C_WRITE 0
//I2C_HandleTypeDef VL53L0X_I2C_Handler; // I2C handler
uint8_t msgBuffer[4];
HAL_StatusTypeDef i2cStat;

//---------------------------------------------------------
// Locally used functions (private)
//---------------------------------------------------------
bool getSpadInfo(uint8_t *count, bool *type_is_aperture, VL53L0X_t *vl53l0x);
void getSequenceStepEnables(SequenceStepEnables * enables, VL53L0X_t *vl53l0x);
void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts, VL53L0X_t *vl53l0x);
bool performSingleRefCalibration(uint8_t vhv_init_byte, VL53L0X_t *vl53l0x);
static uint16_t decodeTimeout(uint16_t value);
static uint16_t encodeTimeout(uint16_t timeout_mclks);
static uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
static uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);

//---------------------------------------------------------
// I2C communication Functions
//---------------------------------------------------------
// Write an 8-bit register
void writeReg(uint8_t reg, uint8_t value, VL53L0X_t *vl53l0x) {

  uint8_t msg = value; // Assign the value to the buffer.
  i2cStat = HAL_I2C_Mem_Write(vl53l0x->vl53l0x_i2c, (vl53l0x->Address | I2C_WRITE), reg, 1, &msg, 1, I2C_TIMEOUT);
}

// Write a 16-bit register
void writeReg16Bit(uint8_t reg, uint16_t value, VL53L0X_t *vl53l0x){

  uint8_t msg[2];
  msg[0] = (value >> 8) & 0xff;
  msg[1] = value & 0xff;
  i2cStat = HAL_I2C_Mem_Write(vl53l0x->vl53l0x_i2c, (vl53l0x->Address | I2C_WRITE), reg, 1, (uint8_t*)msg, 2, I2C_TIMEOUT);
}

// Write a 32-bit register
void writeReg32Bit(uint8_t reg, uint32_t value, VL53L0X_t *vl53l0x){

	uint8_t msg[4];
	msg[0] = (value >> 24) & 0xff;
	msg[1] = (value >> 16) & 0xff;
	msg[2] = (value >> 8) & 0xff;
	msg[3] = value & 0xff;
  i2cStat = HAL_I2C_Mem_Write(vl53l0x->vl53l0x_i2c, (vl53l0x->Address | I2C_WRITE), reg, 1, (uint8_t*)msg, 4, I2C_TIMEOUT);
}

// Read an 8-bit register
uint8_t readReg(uint8_t reg, VL53L0X_t *vl53l0x) {
  uint8_t value;

  i2cStat = HAL_I2C_Mem_Read(vl53l0x->vl53l0x_i2c, (vl53l0x->Address | I2C_READ), reg, 1, msgBuffer, 1, I2C_TIMEOUT);
  value = msgBuffer[0];

  return value;
}

// Read a 16-bit register
uint16_t readReg16Bit(uint8_t reg, VL53L0X_t *vl53l0x) {
  uint16_t value;
  uint8_t msg[2];
  i2cStat = HAL_I2C_Mem_Read(vl53l0x->vl53l0x_i2c, (vl53l0x->Address | I2C_READ), reg, 1, msg, 2, I2C_TIMEOUT);
  value = (uint16_t)((msg[0] << 8) | msg[1]);

  return value;
}

// Read a 32-bit register
uint32_t readReg32Bit(uint8_t reg, VL53L0X_t *vl53l0x) {
  uint32_t value;
  i2cStat = HAL_I2C_Mem_Read(vl53l0x->vl53l0x_i2c, (vl53l0x->Address | I2C_READ), reg, 1, msgBuffer, 4, I2C_TIMEOUT);
  value = (uint32_t)((msgBuffer[0] << 24) | (msgBuffer[1] << 16) | (msgBuffer[2] << 8) | msgBuffer[3]);

  return value;
}

// Write an arbitrary number of bytes from the given array to the sensor,
// starting at the given register
void writeMulti(uint8_t reg, uint8_t const *src, uint8_t count, VL53L0X_t *vl53l0x){
	uint8_t msg[count];
	for (uint8_t i = 0; i < count; i++)
	{
		msg[i] = src[i];
	}
  i2cStat = HAL_I2C_Mem_Write(vl53l0x->vl53l0x_i2c, vl53l0x->Address | I2C_WRITE, reg, 1, (uint8_t*)msg, count, I2C_TIMEOUT);
}

// Read an arbitrary number of bytes from the sensor, starting at the given
// register, into the given array
void readMulti(uint8_t reg, uint8_t * dst, uint8_t count, VL53L0X_t *vl53l0x) {
	i2cStat = HAL_I2C_Mem_Read(vl53l0x->vl53l0x_i2c, vl53l0x->Address | I2C_READ, reg, 1, dst, count, I2C_TIMEOUT);
}


// Public Methods //////////////////////////////////////////////////////////////

bool setAddress(uint8_t new_addr, VL53L0X_t *vl53l0x) {
    writeReg(I2C_SLAVE_DEVICE_ADDRESS, (new_addr >> 1) & 0x7F, vl53l0x);
    if (i2cStat != HAL_OK)
    {
      return false;
    }
    vl53l0x->Address = new_addr;
    g_i2cAddr = new_addr;
    return true;
}

uint8_t getAddress(VL53L0X_t *vl53l0x) {
  return vl53l0x->Address;
}

// Initialize sensor using sequence based on VL53L0X_DataInit(),
// VL53L0X_StaticInit(), and VL53L0X_PerformRefCalibration().
// This function does not perform reference SPAD calibration
// (VL53L0X_PerformRefSpadManagement()), since the API user manual says that it
// is performed by ST on the bare modules; it seems like that should work well
// enough unless a cover glass is added.
// If io_2v8 (optional) is true or not given, the sensor is configured for 2V8
// mode.
uint8_t initVL53L0X(uint8_t io_2v8, VL53L0X_t *vl53l0x, I2C_HandleTypeDef *handler){
  // VL53L0X_DataInit() begin

  //Handler
	vl53l0x->vl53l0x_i2c = handler;

	if (!setAddress(vl53l0x->Address, vl53l0x))
	{
		return false;
	}

  // Reset the message buffer.
  msgBuffer[0] = 0;
  msgBuffer[1] = 0;
  msgBuffer[2] = 0;
  msgBuffer[3] = 0;

	//check model ID reg
	if (readReg(IDENTIFICATION_MODEL_ID, vl53l0x) != 0xEE)
	{
		return false;
	}
  // sensor uses 1V8 mode for I/O by default; switch to 2V8 mode if necessary
  if (io_2v8)
  {
    writeReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
      readReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, vl53l0x) | 0x01, vl53l0x); // set bit 0
  }

  // "Set I2C standard mode"
  writeReg(0x88, 0x00, vl53l0x);

  writeReg(0x80, 0x01, vl53l0x);
  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x00, vl53l0x);
  g_stopVariable = readReg(0x91, vl53l0x);
  writeReg(0x00, 0x01, vl53l0x);
  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x80, 0x00, vl53l0x);

  // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
  writeReg(MSRC_CONFIG_CONTROL, readReg(MSRC_CONFIG_CONTROL, vl53l0x) | 0x12, vl53l0x);

  // set final range signal rate limit to 0.25 MCPS (million counts per second)
  setSignalRateLimit(0.25, vl53l0x);

  writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF, vl53l0x);

  // VL53L0X_DataInit() end

  // VL53L0X_StaticInit() begin

  uint8_t spad_count;
  bool spad_type_is_aperture;
  if (!getSpadInfo(&spad_count, &spad_type_is_aperture, vl53l0x)) { return false; }

  // The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
  // the API, but the same data seems to be more easily readable from
  // GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
  uint8_t ref_spad_map[6];
  readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6, vl53l0x);

  // -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00, vl53l0x);
  writeReg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C, vl53l0x);
  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4, vl53l0x);

  uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
  uint8_t spads_enabled = 0;

  for (uint8_t i = 0; i < 48; i++)
  {
    if (i < first_spad_to_enable || spads_enabled == spad_count)
    {
      // This bit is lower than the first one that should be enabled, or
      // (reference_spad_count) bits have already been enabled, so zero this bit
      ref_spad_map[i / 8] &= ~(1 << (i % 8));
    }
    else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
    {
      spads_enabled++;
    }
  }

  writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6, vl53l0x);

  // -- VL53L0X_set_reference_spads() end

  // -- VL53L0X_load_tuning_settings() begin
  // DefaultTuningSettings from vl53l0x_tuning.h

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x00, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x09, 0x00, vl53l0x);
  writeReg(0x10, 0x00, vl53l0x);
  writeReg(0x11, 0x00, vl53l0x);

  writeReg(0x24, 0x01, vl53l0x);
  writeReg(0x25, 0xFF, vl53l0x);
  writeReg(0x75, 0x00, vl53l0x);

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x4E, 0x2C, vl53l0x);
  writeReg(0x48, 0x00, vl53l0x);
  writeReg(0x30, 0x20, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x30, 0x09, vl53l0x);
  writeReg(0x54, 0x00, vl53l0x);
  writeReg(0x31, 0x04, vl53l0x);
  writeReg(0x32, 0x03, vl53l0x);
  writeReg(0x40, 0x83, vl53l0x);
  writeReg(0x46, 0x25, vl53l0x);
  writeReg(0x60, 0x00, vl53l0x);
  writeReg(0x27, 0x00, vl53l0x);
  writeReg(0x50, 0x06, vl53l0x);
  writeReg(0x51, 0x00, vl53l0x);
  writeReg(0x52, 0x96, vl53l0x);
  writeReg(0x56, 0x08, vl53l0x);
  writeReg(0x57, 0x30, vl53l0x);
  writeReg(0x61, 0x00, vl53l0x);
  writeReg(0x62, 0x00, vl53l0x);
  writeReg(0x64, 0x00, vl53l0x);
  writeReg(0x65, 0x00, vl53l0x);
  writeReg(0x66, 0xA0, vl53l0x);

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x22, 0x32, vl53l0x);
  writeReg(0x47, 0x14, vl53l0x);
  writeReg(0x49, 0xFF, vl53l0x);
  writeReg(0x4A, 0x00, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x7A, 0x0A, vl53l0x);
  writeReg(0x7B, 0x00, vl53l0x);
  writeReg(0x78, 0x21, vl53l0x);

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x23, 0x34, vl53l0x);
  writeReg(0x42, 0x00, vl53l0x);
  writeReg(0x44, 0xFF, vl53l0x);
  writeReg(0x45, 0x26, vl53l0x);
  writeReg(0x46, 0x05, vl53l0x);
  writeReg(0x40, 0x40, vl53l0x);
  writeReg(0x0E, 0x06, vl53l0x);
  writeReg(0x20, 0x1A, vl53l0x);
  writeReg(0x43, 0x40, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x34, 0x03, vl53l0x);
  writeReg(0x35, 0x44, vl53l0x);

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x31, 0x04, vl53l0x);
  writeReg(0x4B, 0x09, vl53l0x);
  writeReg(0x4C, 0x05, vl53l0x);
  writeReg(0x4D, 0x04, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x44, 0x00, vl53l0x);
  writeReg(0x45, 0x20, vl53l0x);
  writeReg(0x47, 0x08, vl53l0x);
  writeReg(0x48, 0x28, vl53l0x);
  writeReg(0x67, 0x00, vl53l0x);
  writeReg(0x70, 0x04, vl53l0x);
  writeReg(0x71, 0x01, vl53l0x);
  writeReg(0x72, 0xFE, vl53l0x);
  writeReg(0x76, 0x00, vl53l0x);
  writeReg(0x77, 0x00, vl53l0x);

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x0D, 0x01, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x80, 0x01, vl53l0x);
  writeReg(0x01, 0xF8, vl53l0x);

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x8E, 0x01, vl53l0x);
  writeReg(0x00, 0x01, vl53l0x);
  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x80, 0x00, vl53l0x);

  // -- VL53L0X_load_tuning_settings() end

  // "Set interrupt config to new sample ready"
  // -- VL53L0X_SetGpioConfig() begin

  writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04, vl53l0x);
  writeReg(GPIO_HV_MUX_ACTIVE_HIGH, readReg(GPIO_HV_MUX_ACTIVE_HIGH, vl53l0x) & ~0x10, vl53l0x); // active low
  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01, vl53l0x);

  // -- VL53L0X_SetGpioConfig() end

  g_measTimBudUs = getMeasurementTimingBudget(vl53l0x);

  // "Disable MSRC and TCC by default"
  // MSRC = Minimum Signal Rate Check
  // TCC = Target CentreCheck
  // -- VL53L0X_SetSequenceStepEnable() begin

  writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8, vl53l0x);

  // -- VL53L0X_SetSequenceStepEnable() end

  // "Recalculate timing budget"
  setMeasurementTimingBudget(g_measTimBudUs, vl53l0x);

  // VL53L0X_StaticInit() end

  // VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

  // -- VL53L0X_perform_vhv_calibration() begin

  writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01, vl53l0x);
  if (!performSingleRefCalibration(0x40, vl53l0x)) { return false; }

  // -- VL53L0X_perform_vhv_calibration() end

  // -- VL53L0X_perform_phase_calibration() begin

  writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02, vl53l0x);
  if (!performSingleRefCalibration(0x00, vl53l0x)) { return false; }

  // -- VL53L0X_perform_phase_calibration() end

  // "restore the previous Sequence Config"
  writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8, vl53l0x);

  // VL53L0X_PerformRefCalibration() end


  return true;
}

// Set the return signal rate limit check value in units of MCPS (mega counts
// per second). "This represents the amplitude of the signal reflected from the
// target and detected by the device"; setting this limit presumably determines
// the minimum measurement necessary for the sensor to report a valid reading.
// Setting a lower limit increases the potential range of the sensor but also
// seems to increase the likelihood of getting an inaccurate reading because of
// unwanted reflections from objects other than the intended target.
// Defaults to 0.25 MCPS as initialized by the ST API and this library.
bool setSignalRateLimit(float limit_Mcps, VL53L0X_t *vl53l0x)
{
  if (limit_Mcps < 0 || limit_Mcps > 511.99) { return false; }

  // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
  writeReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit_Mcps * (1 << 7), vl53l0x);
  return true;
}

// Get the return signal rate limit check value in MCPS
float getSignalRateLimit(VL53L0X_t *vl53l0x)
{
  return (float)readReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, vl53l0x) / (1 << 7);
}

// Set the measurement timing budget in microseconds, which is the time allowed
// for one measurement; the ST API and this library take care of splitting the
// timing budget among the sub-steps in the ranging sequence. A longer timing
// budget allows for more accurate measurements. Increasing the budget by a
// factor of N decreases the range measurement standard deviation by a factor of
// sqrt(N). Defaults to about 33 milliseconds; the minimum is 20 ms.
// based on VL53L0X_set_measurement_timing_budget_micro_seconds()
bool setMeasurementTimingBudget(uint32_t budget_us, VL53L0X_t *vl53l0x)
{
  SequenceStepEnables enables;
  SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  uint32_t const MinTimingBudget = 20000;

  if (budget_us < MinTimingBudget) { return false; }

  uint32_t used_budget_us = StartOverhead + EndOverhead;

  getSequenceStepEnables(&enables, vl53l0x);
  getSequenceStepTimeouts(&enables, &timeouts, vl53l0x);

  if (enables.tcc)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    used_budget_us += FinalRangeOverhead;

    // "Note that the final range timeout is determined by the timing
    // budget and the sum of all other timeouts within the sequence.
    // If there is no room for the final range timeout, then an error
    // will be set. Otherwise the remaining time will be applied to
    // the final range."

    if (used_budget_us > budget_us)
    {
      // "Requested timeout too big."
      return false;
    }

    uint32_t final_range_timeout_us = budget_us - used_budget_us;

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

    // "For the final range timeout, the pre-range timeout
    //  must be added. To do this both final and pre-range
    //  timeouts must be expressed in macro periods MClks
    //  because they have different vcsel periods."

    uint16_t final_range_timeout_mclks =
      timeoutMicrosecondsToMclks(final_range_timeout_us,
                                 timeouts.final_range_vcsel_period_pclks);

    if (enables.pre_range)
    {
      final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
      encodeTimeout(final_range_timeout_mclks), vl53l0x);

    // set_sequence_step_timeout() end

    g_measTimBudUs = budget_us; // store for internal reuse
  }
  return true;
}

// Get the measurement timing budget in microseconds
// based on VL53L0X_get_measurement_timing_budget_micro_seconds()
// in us
uint32_t getMeasurementTimingBudget(VL53L0X_t *vl53l0x)
{
  SequenceStepEnables enables;
  SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  // "Start and end overhead times always present"
  uint32_t budget_us = StartOverhead + EndOverhead;

  getSequenceStepEnables(&enables, vl53l0x);
  getSequenceStepTimeouts(&enables, &timeouts, vl53l0x);

  if (enables.tcc)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    budget_us += (timeouts.final_range_us + FinalRangeOverhead);
  }

  g_measTimBudUs = budget_us; // store for internal reuse
  return budget_us;
}

// Set the VCSEL (vertical cavity surface emitting laser) pulse period for the
// given period type (pre-range or final range) to the given value in PCLKs.
// Longer periods seem to increase the potential range of the sensor.
// Valid values are (even numbers only):
//  pre:  12 to 18 (initialized default: 14)
//  final: 8 to 14 (initialized default: 10)
// based on VL53L0X_set_vcsel_pulse_period()
bool setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks, VL53L0X_t *vl53l0x)
{
  uint8_t vcsel_period_reg = encodeVcselPeriod(period_pclks);

  SequenceStepEnables enables;
  SequenceStepTimeouts timeouts;

  getSequenceStepEnables(&enables, vl53l0x);
  getSequenceStepTimeouts(&enables, &timeouts, vl53l0x);

  // "Apply specific settings for the requested clock period"
  // "Re-calculate and apply timeouts, in macro periods"

  // "When the VCSEL period for the pre or final range is changed,
  // the corresponding timeout must be read from the device using
  // the current VCSEL period, then the new VCSEL period can be
  // applied. The timeout then must be written back to the device
  // using the new VCSEL period.
  //
  // For the MSRC timeout, the same applies - this timeout being
  // dependant on the pre-range vcsel period."


  if (type == VcselPeriodPreRange)
  {
    // "Set phase check limits"
    switch (period_pclks)
    {
      case 12:
        writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18, vl53l0x);
        break;

      case 14:
        writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30, vl53l0x);
        break;

      case 16:
        writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40, vl53l0x);
        break;

      case 18:
        writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50, vl53l0x);
        break;

      default:
        // invalid period
        return false;
    }
    writeReg(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08, vl53l0x);

    // apply new VCSEL period
    writeReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg, vl53l0x);

    // update timeouts

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)

    uint16_t new_pre_range_timeout_mclks =
      timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);

    writeReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
      encodeTimeout(new_pre_range_timeout_mclks), vl53l0x);

    // set_sequence_step_timeout() end

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)

    uint16_t new_msrc_timeout_mclks =
      timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);

    writeReg(MSRC_CONFIG_TIMEOUT_MACROP,
      (new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1), vl53l0x);

    // set_sequence_step_timeout() end
  }
  else if (type == VcselPeriodFinalRange)
  {
    switch (period_pclks)
    {
      case 8:
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10, vl53l0x);
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08, vl53l0x);
        writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02, vl53l0x);
        writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C, vl53l0x);
        writeReg(0xFF, 0x01, vl53l0x);
        writeReg(ALGO_PHASECAL_LIM, 0x30, vl53l0x);
        writeReg(0xFF, 0x00, vl53l0x);
        break;

      case 10:
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28, vl53l0x);
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08, vl53l0x);
        writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03, vl53l0x);
        writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09, vl53l0x);
        writeReg(0xFF, 0x01, vl53l0x);
        writeReg(ALGO_PHASECAL_LIM, 0x20, vl53l0x);
        writeReg(0xFF, 0x00, vl53l0x);
        break;

      case 12:
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38, vl53l0x);
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08, vl53l0x);
        writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03, vl53l0x);
        writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08, vl53l0x);
        writeReg(0xFF, 0x01, vl53l0x);
        writeReg(ALGO_PHASECAL_LIM, 0x20, vl53l0x);
        writeReg(0xFF, 0x00, vl53l0x);
        break;

      case 14:
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48, vl53l0x);
        writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08, vl53l0x);
        writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03, vl53l0x);
        writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07, vl53l0x);
        writeReg(0xFF, 0x01, vl53l0x);
        writeReg(ALGO_PHASECAL_LIM, 0x20, vl53l0x);
        writeReg(0xFF, 0x00, vl53l0x);
        break;

      default:
        // invalid period
        return false;
    }

    // apply new VCSEL period
    writeReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg, vl53l0x);

    // update timeouts

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

    // "For the final range timeout, the pre-range timeout
    //  must be added. To do this both final and pre-range
    //  timeouts must be expressed in macro periods MClks
    //  because they have different vcsel periods."

    uint16_t new_final_range_timeout_mclks =
      timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);

    if (enables.pre_range)
    {
      new_final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
      encodeTimeout(new_final_range_timeout_mclks), vl53l0x);

    // set_sequence_step_timeout end
  }
  else
  {
    // invalid type
    return false;
  }

  // "Finally, the timing budget must be re-applied"

  setMeasurementTimingBudget(g_measTimBudUs, vl53l0x);

  // "Perform the phase calibration. This is needed after changing on vcsel period."
  // VL53L0X_perform_phase_calibration() begin

  uint8_t sequence_config = readReg(SYSTEM_SEQUENCE_CONFIG, vl53l0x);
  writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02, vl53l0x);
  performSingleRefCalibration(0x0, vl53l0x);
  writeReg(SYSTEM_SEQUENCE_CONFIG, sequence_config, vl53l0x);

  // VL53L0X_perform_phase_calibration() end

  return true;
}

// Get the VCSEL pulse period in PCLKs for the given period type.
// based on VL53L0X_get_vcsel_pulse_period()
uint8_t getVcselPulsePeriod(vcselPeriodType type, VL53L0X_t *vl53l0x)
{
  if (type == VcselPeriodPreRange)
  {
    return decodeVcselPeriod(readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, vl53l0x));
  }
  else if (type == VcselPeriodFinalRange)
  {
    return decodeVcselPeriod(readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vl53l0x));
  }
  else { return 255; }
}

// Start continuous ranging measurements. If period_ms (optional) is 0 or not
// given, continuous back-to-back mode is used (the sensor takes measurements as
// often as possible); otherwise, continuous timed mode is used, with the given
// inter-measurement period in milliseconds determining how often the sensor
// takes a measurement.
// based on VL53L0X_StartMeasurement()
void startContinuous(uint32_t period_ms, VL53L0X_t *vl53l0x)
{
  writeReg(0x80, 0x01, vl53l0x);
  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x00, vl53l0x);
  writeReg(0x91, g_stopVariable, vl53l0x);
  writeReg(0x00, 0x01, vl53l0x);
  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x80, 0x00, vl53l0x);

  if (period_ms != 0)
  {
    // continuous timed mode

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

    uint16_t osc_calibrate_val = readReg16Bit(OSC_CALIBRATE_VAL, vl53l0x);

    if (osc_calibrate_val != 0)
    {
      period_ms *= osc_calibrate_val;
    }

    writeReg32Bit(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms, vl53l0x);

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() end

    writeReg(SYSRANGE_START, 0x04, vl53l0x); // VL53L0X_REG_SYSRANGE_MODE_TIMED
  }
  else
  {
    // continuous back-to-back mode
    writeReg(SYSRANGE_START, 0x02, vl53l0x); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
  }
}

// Stop continuous measurements
// based on VL53L0X_StopMeasurement()
void stopContinuous(VL53L0X_t *vl53l0x)
{
  writeReg(SYSRANGE_START, 0x01, vl53l0x); // VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT

  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x00, vl53l0x);
  writeReg(0x91, 0x00, vl53l0x);
  writeReg(0x00, 0x01, vl53l0x);
  writeReg(0xFF, 0x00, vl53l0x);
}

// Returns a range reading in millimeters when continuous mode is active
// (readRangeSingleMillimeters() also calls this function after starting a
// single-shot range measurement)
// extraStats provides additional info for this measurment. Set to 0 if not needed.
uint16_t readRangeContinuousMillimeters(VL53L0X_t *vl53l0x) {
//  uint8_t tempBuffer[12];
  uint16_t temp, result;
  startTimeout();
  while ((readReg(RESULT_INTERRUPT_STATUS, vl53l0x) & 0x07) == 0) {
    if (checkTimeoutExpired())
    {
      g_isTimeout = true;
      return 65535;
    }
  }
  temp = readReg16Bit(RESULT_RANGE_STATUS + 10, vl53l0x);

  switch(i2cStat)
  {
  	  case HAL_ERROR:
  		  result = 404;
  		  break;
  	  case HAL_BUSY:
  		  result = 405;
		  break;
  	  case HAL_TIMEOUT:
  		  result = 406;
  		  break;
  	  default:
  		  result = temp;
  }


  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01, vl53l0x);

  return result;
}

// Performs a single-shot range measurement and returns the reading in
// millimeters
// based on VL53L0X_PerformSingleRangingMeasurement()
// extraStats provides additional info for this measurment. Set to 0 if not needed.
uint16_t readRangeSingleMillimeters(VL53L0X_t *vl53l0x) {
  writeReg(0x80, 0x01, vl53l0x);
  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x00, vl53l0x);
  writeReg(0x91, g_stopVariable, vl53l0x);
  writeReg(0x00, 0x01, vl53l0x);
  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x80, 0x00, vl53l0x);
  writeReg(SYSRANGE_START, 0x01, vl53l0x);
  // "Wait until start bit has been cleared"
  startTimeout();
  while (readReg(SYSRANGE_START, vl53l0x) & 0x01){
    if (checkTimeoutExpired()){
      g_isTimeout = true;
      return 65535;
    }
  }
  return readRangeContinuousMillimeters(vl53l0x);
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool timeoutOccurred()
{
  bool tmp = g_isTimeout;
  g_isTimeout = false;
  return tmp;
}

void setTimeout(uint16_t timeout){
  g_ioTimeout = timeout;
}

uint16_t getTimeout(void){
  return g_ioTimeout;
}

// Private Methods /////////////////////////////////////////////////////////////

// Get reference SPAD (single photon avalanche diode) count and type
// based on VL53L0X_get_info_from_device(),
// but only gets reference SPAD count and type
bool getSpadInfo(uint8_t * count, bool * type_is_aperture,VL53L0X_t *vl53l0x)
{
  uint8_t tmp;

  writeReg(0x80, 0x01, vl53l0x);
  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x00, vl53l0x);

  writeReg(0xFF, 0x06, vl53l0x);
  writeReg(0x83, readReg(0x83, vl53l0x) | 0x04, vl53l0x);
  writeReg(0xFF, 0x07, vl53l0x);
  writeReg(0x81, 0x01, vl53l0x);

  writeReg(0x80, 0x01, vl53l0x);

  writeReg(0x94, 0x6b, vl53l0x);
  writeReg(0x83, 0x00, vl53l0x);
  startTimeout();
  while (readReg(0x83, vl53l0x) == 0x00)
  {
    if (checkTimeoutExpired()) { return false; }
  }
  writeReg(0x83, 0x01, vl53l0x);
  tmp = readReg(0x92, vl53l0x);

  *count = tmp & 0x7f;
  *type_is_aperture = (tmp >> 7) & 0x01;

  writeReg(0x81, 0x00, vl53l0x);
  writeReg(0xFF, 0x06, vl53l0x);
  writeReg(0x83, readReg(0x83, vl53l0x)  & ~0x04, vl53l0x);
  writeReg(0xFF, 0x01, vl53l0x);
  writeReg(0x00, 0x01, vl53l0x);

  writeReg(0xFF, 0x00, vl53l0x);
  writeReg(0x80, 0x00, vl53l0x);

  return true;
}

// Get sequence step enables
// based on VL53L0X_GetSequenceStepEnables()
void getSequenceStepEnables(SequenceStepEnables * enables, VL53L0X_t *vl53l0x)
{
  uint8_t sequence_config = readReg(SYSTEM_SEQUENCE_CONFIG, vl53l0x);

  enables->tcc          = (sequence_config >> 4) & 0x1;
  enables->dss          = (sequence_config >> 3) & 0x1;
  enables->msrc         = (sequence_config >> 2) & 0x1;
  enables->pre_range    = (sequence_config >> 6) & 0x1;
  enables->final_range  = (sequence_config >> 7) & 0x1;
}

// Get sequence step timeouts
// based on get_sequence_step_timeout(),
// but gets all timeouts instead of just the requested one, and also stores
// intermediate values
void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts, VL53L0X_t *vl53l0x)
{
  timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange, vl53l0x);

  timeouts->msrc_dss_tcc_mclks = readReg(MSRC_CONFIG_TIMEOUT_MACROP, vl53l0x) + 1;
  timeouts->msrc_dss_tcc_us =
    timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->pre_range_mclks =
    decodeTimeout(readReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, vl53l0x));
  timeouts->pre_range_us =
    timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange, vl53l0x);

  timeouts->final_range_mclks =
    decodeTimeout(readReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, vl53l0x));

  if (enables->pre_range)
  {
    timeouts->final_range_mclks -= timeouts->pre_range_mclks;
  }

  timeouts->final_range_us =
    timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                               timeouts->final_range_vcsel_period_pclks);
}

// Decode sequence step timeout in MCLKs from register value
// based on VL53L0X_decode_timeout()
// Note: the original function returned a uint32_t, but the return value is
// always stored in a uint16_t.
uint16_t decodeTimeout(uint16_t reg_val)
{
  // format: "(LSByte * 2^MSByte) + 1"
  return (uint16_t)((reg_val & 0x00FF) <<
         (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
// Note: the original function took a uint16_t, but the argument passed to it
// is always a uint16_t.
uint16_t encodeTimeout(uint16_t timeout_mclks)
{
  // format: "(LSByte * 2^MSByte) + 1"

  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0)
  {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0)
    {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else { return 0; }
}

// Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_us()
uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

  return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_mclks()
uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

  return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}


// based on VL53L0X_perform_single_ref_calibration()
bool performSingleRefCalibration(uint8_t vhv_init_byte, VL53L0X_t *vl53l0x)
{
  writeReg(SYSRANGE_START, 0x01 | vhv_init_byte, vl53l0x); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

  startTimeout();
  while ((readReg(RESULT_INTERRUPT_STATUS, vl53l0x) & 0x07) == 0)
  {
    if (checkTimeoutExpired()) { return false; }
  }

  writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01, vl53l0x);

  writeReg(SYSRANGE_START, 0x00, vl53l0x);

  return true;
}
// void resetSensor(VL53L0X_t *lox1, VL53L0X_t *lox2)
// {
// 	lox1->Address = ADDRESS_DEFAULT;
// 	lox2->Address = ADDRESS_DEFAULT;
// 	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 0);
// 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
// 	HAL_Delay(20);

// 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
// 	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);
// 	HAL_Delay(20);
// }

// void changeAddressSensor1(VL53L0X_t *lox1, I2C_HandleTypeDef *i2cx)
// {
// 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 0);
// 	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);
// 	HAL_Delay(20);
// 	while (!initVL53L0X(1, lox1, i2cx))
// 	{
// 	  HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
// 	  printf("Failed 1!\n");
// 	}
// 	setAddress(0x50, lox1);

// 	while (!initVL53L0X(1, lox1, i2cx))
// 	{
// 	  HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
// //	  printf("Failed 2!\n");
// 	}
// }

// void changeAddressSensor2(VL53L0X_t *lox2, I2C_HandleTypeDef *i2cx)
// {
// 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
// 	//  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 0);
// 	HAL_Delay(20);

// 	while (!initVL53L0X(1, lox2, i2cx))
// 	{
// 	  HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
// //	  printf("Failed 3!\n");
// 	}
// 	setAddress(0x42, lox2);

// 	while (!initVL53L0X(1, lox2, i2cx))
// 	{
// 	  HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 0); // on led
// //	  printf("Failed 4!\n");
// 	}
// }
// void turnOnSensor()
// {
// 	// Sensor
// 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, 1);
// 	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1);
// 	HAL_Delay(20);
// 	HAL_GPIO_WritePin(LEDR_GPIO_Port, LEDR_Pin, 1);
// }
