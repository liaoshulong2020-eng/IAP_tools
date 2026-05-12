#include <math.h> // For sin() and cos() functions
#include "main.h"
#include "soft_spll_app.h"
// Define constants for PLL
SPLL spll;

// Initialize SPLL object
void SPLL_Init(SPLL *spll) {
    spll->phase_angle = 0.0;
    spll->omega = TWO_PI * NOMINAL_FREQUENCY / SAMPLING_FREQUENCY;
    spll->phase_increment = TWO_PI / SAMPLES_PER_CYCLE;
    spll->sin_output = 0.0;
    spll->cos_output = 1.0;  // Start at cosine(0) = 1
}


// Update SPLL with new sample
void SPLL_Update(SPLL *spll, float grid_voltage) {
    float error = grid_voltage - spll->sin_output;
    spll->omega += 0.001 * error; // Simple PI controller, tune as needed
    spll->phase_angle += spll->omega;

    // Normalize the phase angle to keep it within 0 to 2*PI range
    while (spll->phase_angle >= TWO_PI) {
        spll->phase_angle -= TWO_PI;
    }
    while (spll->phase_angle < 0) {
        spll->phase_angle += TWO_PI;
    }

    spll->sin_output = sin(spll->phase_angle);
    spll->cos_output = cos(spll->phase_angle);
}

    
    


