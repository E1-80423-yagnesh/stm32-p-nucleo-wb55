#include "main.h"
#include "adc.h"
#include "gsm.h"

//#define DEBUG
#define MIN_FAST_REFRESH_CYCLES 2
#define STABILIZATION_CHECKS 3  // Require 3 consecutive stable readings before switching back

static uint8_t fast_refresh_cycles_remaining = 0;
static uint8_t stable_reading_count = 0;  // Counter for consecutive stable readings
static uint16_t rms_counter = 0;
static uint16_t fast_check_counter = 0;
static float prev_adc_AI_f[10] = {0};
static float prev_adc_average_m[3] = {0};
static float prev_adc_average_F[3] = {0};

uint32_t adc_sum[NUM_SLOW_CHANNELS] = {0};
uint16_t adc_average[NUM_SLOW_CHANNELS] = {0};
float adc_average_F[NUM_SLOW_CHANNELS] = {0};
uint16_t adc_average_m[6] = {0};
uint16_t adc_buffer[ADC_CHANNELS];

void ADC_Process_Conversion_Complete(void)
{
    // Map ADC buffer values to logical channels A0-A15
    adc_values_all[0] = adc_buffer[2];   // A0
    adc_values_all[1] = adc_buffer[9];   // A1
    adc_values_all[2] = adc_buffer[8];   // A2
    adc_values_all[3] = adc_buffer[15];  // A3
    adc_values_all[4] = adc_buffer[14];  // A4
    adc_values_all[5] = adc_buffer[7];   // A5
    adc_values_all[6] = adc_buffer[6];   // A6
    adc_values_all[7] = adc_buffer[5];   // A7
    adc_values_all[8] = adc_buffer[4];   // A8
    adc_values_all[9] = adc_buffer[3];   // A9
    adc_values_all[10] = adc_buffer[10]; // A10
    adc_values_all[11] = adc_buffer[13]; // A11
    adc_values_all[12] = adc_buffer[11]; // A12
    adc_values_all[13] = adc_buffer[0];  // A13
    adc_values_all[14] = adc_buffer[12]; // A14
    adc_values_all[15] = adc_buffer[1];  // A15
}

void ADC_Process_Timer_Interrupt(void)
{
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)adc_buffer, ADC_CHANNELS);

    // Accumulate samples for channels 10-15 (A10-A15)
    for (uint8_t ch = 0; ch < NUM_SLOW_CHANNELS; ch++)
    {
        adc_sum[ch] += adc_values_all[10 + ch];
    }

    // Increment counters
    rms_counter++;
    fast_check_counter++;

    // Check A0-A9 every 20ms (40 samples at 500μs)
    if (fast_check_counter >= FAST_CHECK_COUNT)
    {
        check_A0_A9_flag = 1;
        fast_check_counter = 0;
    }

    // Calculate averages for A10-A15 every 80ms (160 samples at 500μs)
    if (rms_counter >= RMS_SAMPLE_COUNT)
    {
        // Calculate averages for channels 10-15
        for (uint8_t ch = 0; ch < NUM_SLOW_CHANNELS; ch++)
        {
            adc_average[ch] = adc_sum[ch] / RMS_SAMPLE_COUNT;
            adc_sum[ch] = 0;
        }

        adc_average_m[0] = (float)(adc_average[0]-3)*(calib.V_fact);
        adc_average_m[2] = (float)(adc_average[2]-3)*(calib.V_fact);
        adc_average_m[4] = (float)(adc_average[4]-3)*(calib.V_fact);

        adc_average_F[1] = (float)adc_average[1]*(calib.I_fact);
        adc_average_F[3] = (float)adc_average[3]*(calib.I_fact);
        adc_average_F[5] = (float)adc_average[5]*(calib.I_fact);

        rms_counter = 0;
        average_ready = 1;
    }
}

// Enhanced threshold check with hysteresis
uint8_t Check_Percentage_Threshold_With_Hysteresis(float current_value, float previous_value,
                                                    float threshold_percent, uint8_t use_hysteresis)
{
    float min_absolute_threshold = 0.5f;

    // Apply hysteresis: use 70% of threshold for exit condition
    float effective_threshold = threshold_percent;
    if (use_hysteresis)
    {
        effective_threshold = threshold_percent * 0.7f;  // 30% hysteresis band
    }

    float percentage_threshold = fabsf(previous_value) * (effective_threshold / 100.0f);
    float threshold_amount = (percentage_threshold > min_absolute_threshold) ?
                            percentage_threshold : min_absolute_threshold;

    float upper_limit = previous_value + threshold_amount;
    float lower_limit = previous_value - threshold_amount;

    return (current_value > upper_limit || current_value < lower_limit) ? 1 : 0;
}

// Helper function - backward compatible
uint8_t Check_Percentage_Threshold(float current_value, float previous_value, float threshold_percent)
{
    return Check_Percentage_Threshold_With_Hysteresis(current_value, previous_value,
                                                       threshold_percent, 0);
}

void Check_Fast_Channels_A0_A9(void)
{
    // Calculate current values for A0-A9
    for (uint8_t ch = 0; ch < 10; ch++){
        adc_AI_f[ch] = ((float)(adc_values_all[ch]-4)) * calib.A0_A9_fact;
    }

    exceeded = 0;

    // Use hysteresis only when in fast mode trying to exit
    uint8_t use_hysteresis = (current_refresh_rate == 1) ? 1 : 0;

    // Check channels A0–A9 for percentage-based threshold
    for (int i = 0; i < 10; i++)
    {
        if (Check_Percentage_Threshold_With_Hysteresis(adc_AI_f[i], prev_adc_AI_f[i],
                                                        (float)systemConfig.threshold, use_hysteresis))
        {
            exceeded = 1;
#ifdef DEBUG
            char debug_buf[64];
            sprintf(debug_buf, "A%d threshold exceeded: %.3f (prev: %.3f)\r\n",
                    i, adc_AI_f[i], prev_adc_AI_f[i]);
            GSM_Debug_Print(debug_buf);
#endif
            break;
        }
    }

    // Update previous values for A0-A9
    for (int i = 0; i < 10; i++)
        prev_adc_AI_f[i] = adc_AI_f[i];
}

void Check_Slow_Channels_A10_A15(void)
{
    exceeded1 = 0;

    // Use hysteresis only when in fast mode trying to exit
    uint8_t use_hysteresis = (current_refresh_rate == 1) ? 1 : 0;

    // Check A10, A12, A14 (indices 0,2,4)
    int m_indices[] = {0, 2, 4};
    for (int i = 0; i < 3; i++)
    {
        if (Check_Percentage_Threshold_With_Hysteresis((float)adc_average_m[m_indices[i]],
                                                        prev_adc_average_m[i],
                                                        (float)systemConfig.threshold, use_hysteresis))
        {
            exceeded1 = 1;
#ifdef DEBUG
            char debug_buf[64];
            sprintf(debug_buf, "A%d threshold exceeded: %d (prev: %.2f)\r\n",
                    10 + m_indices[i], adc_average_m[m_indices[i]], prev_adc_average_m[i]);
            GSM_Debug_Print(debug_buf);
#endif
            break;
        }
    }

    // Check A11, A13, A15 (indices 1,3,5) if not already exceeded
    if (!exceeded1)
    {
        int f_indices[] = {1, 3, 5};
        for (int i = 0; i < 3; i++)
        {
            if (Check_Percentage_Threshold_With_Hysteresis(adc_average_F[f_indices[i]],
                                                            prev_adc_average_F[i],
                                                            (float)systemConfig.threshold, use_hysteresis))
            {
                exceeded1 = 1;
#ifdef DEBUG
                char debug_buf[64];
                sprintf(debug_buf, "A%d threshold exceeded: %.2f (prev: %.2f)\r\n",
                        11 + f_indices[i], adc_average_F[f_indices[i]], prev_adc_average_F[i]);
                GSM_Debug_Print(debug_buf);
#endif
                break;
            }
        }
    }

    // Update previous values for A10-A15
    int m_indices_update[] = {0, 2, 4};
    int f_indices_update[] = {1, 3, 5};

    for (int i = 0; i < 3; i++)
    {
        prev_adc_average_m[i] = (float)adc_average_m[m_indices_update[i]];
        prev_adc_average_F[i] = adc_average_F[f_indices_update[i]];
    }
}

void Update_Refresh_Rate_From_Flags(void)
{
    // If threshold exceeded, switch to fast mode
    if ((exceeded || exceeded1))
    {
        // Reset stable counter whenever threshold is exceeded
        stable_reading_count = 0;

        if (current_refresh_rate == 0)
        {
            // Switching from slow to fast mode
            current_refresh_rate = 1;
            refresh_rate_locked = 1;
            fast_cycle_completed = 0;
            fast_refresh_cycles_remaining = MIN_FAST_REFRESH_CYCLES;

#ifdef DEBUG
            GSM_Debug_Print("Switching to FAST refresh mode\r\n");
#endif

            // Trigger faster send if timer is too far ahead
            if (timer_counter > systemConfig.refreshTime2)
            {
                timer_counter = systemConfig.refreshTime2;
            }
        }
        else
        {
            // Already in fast mode, extend the duration
            fast_refresh_cycles_remaining = MIN_FAST_REFRESH_CYCLES;
        }
    }
    // Check if we can switch back to slow mode
    else if ((current_refresh_rate == 1) && (!exceeded && !exceeded1))
    {
        // Increment stable reading counter
        stable_reading_count++;

        // Only decrement cycle counter when data is actually sent
        if (fast_cycle_completed)
        {
            if (fast_refresh_cycles_remaining > 0)
            {
                fast_refresh_cycles_remaining--;
            }
            fast_cycle_completed = 0;
        }

        // Switch back only after:
        // 1. Minimum fast cycles completed
        // 2. Multiple consecutive stable readings (no threshold exceeded)
        if ((fast_refresh_cycles_remaining == 0) &&
            (stable_reading_count >= STABILIZATION_CHECKS))
        {
            current_refresh_rate = 0;
            refresh_rate_locked = 0;
            stable_reading_count = 0;  // Reset for next transition

            // Reset timer to ensure full slow refresh cycle
            timer_counter = 0;

#ifdef DEBUG
            GSM_Debug_Print("Switching to SLOW refresh mode (stable)\r\n");
#endif
        }
#ifdef DEBUG
        else if (fast_refresh_cycles_remaining == 0)
        {
            char debug_buf[64];
            sprintf(debug_buf, "Stabilizing... (%d/%d)\r\n",
                    stable_reading_count, STABILIZATION_CHECKS);
            GSM_Debug_Print(debug_buf);
        }
#endif
    }
}
