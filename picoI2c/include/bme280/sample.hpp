#pragma once

struct Bme280Sample {
    double temperature_c {0.0};
    double pressure_pa {0.0};
    double humidity_percent {0.0};
};
