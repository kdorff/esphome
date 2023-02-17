#define NUM_FIR_TAPS 10

//
// This code implements a Moving Average Finite
// Impulse Response to smooth input values.
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

void initFIR() {
    for (int i = 0; i < NUM_FIR_TAPS; i++) {
        taps[i] = 0;
    }
    tapPosition = 0;
    numSamplesObserved = 0;
}

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
