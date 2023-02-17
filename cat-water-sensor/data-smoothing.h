#define NUM_FIR_TAPS 10
#define RECENT_MIN_WINDOW 1

//
// This code implements a Moving Average Finite
// Impulse Response to smooth input values as well as
// a offering a recentMin functionality to provide
// additional integer smoothing at the finer-level.
//
// https://en.wikipedia.org/wiki/Finite_impulse_response
//
// The the NUM_FIR_TAPS to an appropriate window size,
// but not too big or the average will be slow
// to respond to actual major changes in data.
//
// Call initFIR() once and then call
// int observeSample(int) to provide the current reading
// and receive the a corrected, averaged reading.
//

int taps[NUM_FIR_TAPS];
int tapPosition;
long numSamplesObserved;

// The recent minimum value. Provide additional smoothing at the small level.
int recentMin = -1;

/**
 * Initialize.
 */
void initFIR() {
    for (int i = 0; i < NUM_FIR_TAPS; i++) {
        taps[i] = 0;
    }
    tapPosition = 0;
    numSamplesObserved = 0;
}

/**
 * Observe a sample with the FIR and return a smoothed value.
 */
int observeSample(int newReading) {
    taps[tapPosition++] = newReading;
    numSamplesObserved++;
    if (tapPosition == NUM_FIR_TAPS) {
        // taps[] is a cyclic window. Move back to the start.
        tapPosition = 0;
    }
    double result = 0;
    int numTapsToUse = numSamplesObserved < NUM_FIR_TAPS ? numSamplesObserved : NUM_FIR_TAPS;
    double tapFactor = (1 / (double) numTapsToUse);
    for (int i = 0; i < numTapsToUse; i++) {
        result += taps[i] * (tapFactor);
    }
    return (int) result;
}

/**
 * If the reading is within [recentMin, recentMin+RECENT_MIN_WINDOW] return
 * recentMin. Otherwise use the value as the new recentMin.
 * Provides smoothing at the smaller scale.
 */
int recentMinForSample(int newValue) {
    if (recentMin == -1) {
        // We don't have a recentMin so newValue becomes recentMin.
        ESP_LOGI("recentMin", "recentMin initialized newValue=%d, recentMin=%d", newValue, recentMin);
        recentMin = newValue;
        return recentMin;
    }
    if (newValue >= recentMin && newValue <= (recentMin + RECENT_MIN_WINDOW)) {
        // newValue is in the window. Return recentMin.
        ESP_LOGI("recentMin", "UYsing recentMin newValue=%d, recentMin=%d", newValue, recentMin);
        return recentMin;
    }
    // Not in the window, we have a new recentMin.
    ESP_LOGI("recentMin", "New recentMin newValue=%d, recentMin=%d", newValue, recentMin);
    recentMin = newValue;
    return recentMin;
}
