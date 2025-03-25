/* -------------------------------------------------------------------
    *
    * This module has been developed by the Automatic Control Group
    * of the University of Salerno, Italy.
    *
    * Title:   TestMode4_movement_test.cpp
    * Author:  Fabrizio Di Domenico, Giovanni Mignone
    * Org.:    UNISA
    * Date:    Sep 05, 2019
    *
    * The aim of this test is controlling the robot in Mode 4, that is 
    * supplying every open-enabled axis with absolute target position and
    * target velocity according to a sinusoidal movement, whose amplitude 
    * and frequency are chosen by the user, starting from the current 
    * position of the robot. The test will last until a Mode 504 request 
    * has been scheduled on C4G via PDL2.
    * 
    * -------------------------------------------------------------------
*/

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <c4g_open_driver/C4gOpen.hpp>

#define ARM      1
#define N_JOINTS 7

using namespace std;

int32_t main(int32_t argc, char *argv[])
{

    double txRate[N_JOINTS];
    double calibration_constants[N_JOINTS];
    bool sinAxisEnabled[N_JOINTS];
    bool checksend;
    double frequencyHz;
    double amplitude;
    double amplitudeGearRotations[N_JOINTS];
    double sampleTime;
    int32_t port;

    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << "freqHz amplitude [1][2][3][4][5][6][7]\n\n";
        cout << "Example: " << argv[0] << " 0.25 5 1357\n";
        cout << "         means a sin contribute of 5 degrees at 0.25 Hz for axes 1, 3 and 5\n";
        cout << "         and a sin contribute of 5 centimeters at 0.25 Hz for axis 7.\n";

        exit(1);
    }

    // Get port from user's environment variable
    const char* env_var_port_number = std::getenv("C4G_PORT_NUMBER");

    if (env_var_port_number == NULL) 
    {
        cout << "C4G_PORT_NUMBER was not found as an environment variable.\n";
        exit(1);
    }

    port = atoi(env_var_port_number);

    // Parse command line parameters
    frequencyHz = atof(argv[1]);
    amplitude = atof(argv[2]);

    for (int32_t i = 0; i < N_JOINTS; i++)
        sinAxisEnabled[i] = false;

    for (uint32_t i = 0; i < strlen(argv[3]); i++)
    {
        if (argv[3][i] >= '1' && argv[3][i] <= '7')
            sinAxisEnabled[(argv[3][i] - '0') - 1] = true;
    }

    C4gOpen c4gOpen(port);

    cout << "\nTestMode4 started...\n";
    cout.flush();
    cout << "\n    After C4G starts:";
    cout.flush();
    cout << "\n       1. Drive On the robot";
    cout.flush();
    cout << "\n       2. Set mode 4 via PDL2\n";
    cout.flush();
    cout << "\n    To stop the test:";
    cout.flush();
    cout << "\n       1. Drive Off the robot";
    cout.flush();
    cout << "\n       2. Set mode 504 via PDL2\n\n";
    cout.flush();

    if (c4gOpen.start())
    {
        float initialPositions[N_JOINTS];
        float actualPositions[N_JOINTS];
        float previousPositions[N_JOINTS];
        float deltaPositions[N_JOINTS];

        // Sample time is returned in milliseconds, we convert it in seconds
        sampleTime = (double)c4gOpen.getSampleTime() / 1000.0;

        float k54 = c4gOpen.getKinInflCoeff54(ARM);
        float k65 = c4gOpen.getKinInflCoeff65(ARM);
        float k64 = c4gOpen.getKinInflCoeff64(ARM);

        for (int32_t i = 0; i < N_JOINTS; i++)
        {
            txRate[i] = c4gOpen.getTxRate(ARM, i + 1);
            calibration_constants[i] = c4gOpen.getCalibrationConstant(ARM, i + 1);
        }

        for (int32_t i = 0; i < N_JOINTS; i++)
        {
            if (sinAxisEnabled[i])
            {
                if (i < 4)
                    amplitudeGearRotations[i] = txRate[i] * amplitude * M_PI / 180 / (2 * M_PI) + calibration_constants[i];
                else if (i == 4)
                    amplitudeGearRotations[i] = txRate[i] * (amplitude * M_PI / 180 - k54 * ((amplitudeGearRotations[i - 1] - calibration_constants[i - 1]) * 2 * M_PI / txRate[i - 1])) / (2 * M_PI) + calibration_constants[i];
                else if (i == 5)
                    amplitudeGearRotations[i] = txRate[i] * (amplitude * M_PI / 180 - k65 * ((amplitudeGearRotations[i - 1] + txRate[i - 1] * k54 * ((amplitudeGearRotations[i - 2] - calibration_constants[i - 2]) * 2 * M_PI / txRate[i - 2]) / (2 * M_PI) - calibration_constants[i - 1]) * 2 * M_PI / txRate[i - 1]) + (k65 * k54 - k64) * ((amplitudeGearRotations[i - 2] - calibration_constants[i - 2]) * 2 * M_PI / txRate[i - 2])) / (2 * M_PI) + calibration_constants[i];
                else if (i == 6)
                    amplitudeGearRotations[i] = (amplitude * 10 + calibration_constants[i]) / txRate[i];
            }
        }

        bool keepGoingOn = true;
        int32_t counter = 0;

        // The switch to Open Mode 4 is done via PDL2 when the C4G starts.
        // Since we get to this point, before the axes have been switched,
        // we need to wait for all of them to be in the correct Mode before sending commands.
        c4gOpen.waitForOpenMode4(ARM);
        cout << "\nReady in mode: " << c4gOpen.getMode(ARM);
        cout.flush();

        while (keepGoingOn)
        {
            if (c4gOpen.receive())
            {

                int32_t mode = c4gOpen.getMode(ARM);

                if (mode == C4G_OPEN_EXIT)
                    keepGoingOn = false;
                else
                {
                    if (mode == C4G_OPEN_DRIVING_ON || mode == C4G_OPEN_MODE_0 || mode == C4G_OPEN_MODE_4)
                    {

                        if (mode == C4G_OPEN_MODE_4)
                        {
                            if (counter == 0)
                            {
                                // If we have just started sending commands, let's initialize the data structures we use
                                for (int32_t i = 0; i < c4gOpen.getNumberOfOpenAxes(); i++)
                                {
                                    if (sinAxisEnabled[i])
                                    {
                                        initialPositions[i] = c4gOpen.getActualPosition(ARM, i + 1);
                                        previousPositions[i] = initialPositions[i];
                                        deltaPositions[i] = 0.0;
                                    }
                                }
                            }

                            for (int32_t i = 0; i < c4gOpen.getNumberOfOpenAxes(); i++)
                            {
                                if (sinAxisEnabled[i])
                                {
                                    if (c4gOpen.isInDriveOn(ARM))
                                    {
                                        double omegat = 2 * M_PI * frequencyHz * ((double)counter * sampleTime);
                                        actualPositions[i] = initialPositions[i] + (float)(amplitudeGearRotations[i] * cos(omegat) - amplitudeGearRotations[i]);

                                        // Since the period is fixed for the C4G, velocity is determined only on the basis of the position difference
                                        deltaPositions[i] = actualPositions[i] - previousPositions[i];

                                        previousPositions[i] = actualPositions[i];
                                    }

                                    c4gOpen.setTargetPosition(ARM, i + 1, actualPositions[i]);
                                    c4gOpen.setTargetVelocity(ARM, i + 1, deltaPositions[i]);
                                }
                            }

                            if (c4gOpen.isInDriveOn(ARM))
                                counter++;
                        }

                        checksend = c4gOpen.send();

                        if (!checksend)
                            keepGoingOn = false;

                        if (c4gOpen.errorOccurred())
                        {
                            cout << "\nError: " << c4gOpen.getLastError();
                            cout.flush();

                            c4gOpen.resetError();
                            counter = 0;
                        }
                    }
                    else
                    {
                        c4gOpen.setMode(ARM, C4G_OPEN_DRIVE_OFF);
                        c4gOpen.send();
                        keepGoingOn = false;
                    }
                }
            }
            else
                keepGoingOn = false;
        }
    }

    c4gOpen.stop();

    cout << "\nTestMode4_movement_test terminated.\n\n";
    cout.flush();

    return 0;
}