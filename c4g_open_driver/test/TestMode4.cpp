/*
    TestMode4.cpp

    Copyright (C) 2007-2008 Sintesi S.C.p.A.

    Developers:
        Sabino   COLONNA (2006-, s.colonna@sintesi-scpa.com)
        Giovanni IACCA   (2006-, g.iacca@sintesi-scpa.com  )
        Giovanni TOTARO  (2006-, g.totaro@sintesi-scpa.com )


    This file is part of libC4gOpen.

    libC4gOpen is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation; either version 2.1 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <c4g_open_driver/C4gOpen.hpp>

#define PI       3.14159265358979
#define ARM      1
#define N_JOINTS 6

using namespace std;

const char disclaimer[] = "\n*********************************************************************\n"
                          "                              TestMode4                                \n\n"
                          "   C4G Open Library - Copyright (C) 2007 Sintesi S.C.p.A.             \n\n"
                          "  Developers:                                                          \n"
                          "           Sabino   COLONNA (2006-, s.colonna@sintesi-scpa.com)    \n"
                          "           Giovanni IACCA   (2006-, g.iacca@sintesi-scpa.com  )    \n"
                          "           Giovanni TOTARO  (2006-, g.totaro@sintesi-scpa.com )    \n"
                          "\n*********************************************************************\n";

int32_t main(int32_t argc, char *argv[])
{

    double txRate[N_JOINTS];
    double calibration_constants[N_JOINTS];

    bool sinAxisEnabled[N_JOINTS];
    double frequencyHz;
    double amplitude;
    double amplitudeGearRotations[N_JOINTS];
        
    double sampleTime;
    
    cout << disclaimer << "\n";	

    if (argc != 4)
    {
        cout << "Usage: " << argv[0] << " freqHz amplitude [1][2][3][4][5][6]\n\n";
        cout << "Example: " << argv[0] << " 0.25 5 135\n";
        cout << "         means a sin contribute of 5 degree at 0.25 Hz for axes 1, 3 and 5\n";
        
        exit(1);
    }

    // Parse command line parameters
    frequencyHz = atof(argv[1]);
    amplitude = atof(argv[2]);
    for (int32_t i = 0; i < N_JOINTS; i++) sinAxisEnabled[i] = false;
    
    for (uint32_t i = 0; i < strlen(argv[3]); i++)
    {
        if (argv[3][i] >= '1' && argv[3][i] <= '6') sinAxisEnabled[(argv[3][i]-'0')-1] = true;
    }

    C4gOpen c4gOpen;

    cout << "\nTestMode4 started...\n"; cout.flush();
    cout << "\n    After C4G starts:"; cout.flush();
    cout << "\n       1. Drive On the robot"; cout.flush();
    cout << "\n       2. Set mode 4 via PDL2\n"; cout.flush();
    cout << "\n    To stop the test:"; cout.flush();
    cout << "\n       1. Drive Off the robot"; cout.flush();
    cout << "\n       2. Set mode 504 via PDL2\n\n"; cout.flush();

    if (c4gOpen.start())
    {
        float initialPositions[N_JOINTS];
        float actualPositions[N_JOINTS];
        float previousPositions[N_JOINTS];
        float deltaPositions[N_JOINTS];

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
            }
        }

        bool keepGoingOn = true;
        int32_t counter = 0;					

        c4gOpen.waitForOpenMode4(ARM);
        cout << "\nReady in mode: " << c4gOpen.getMode(ARM);
        cout.flush();
        
        while (keepGoingOn)
        {
            if (c4gOpen.receive())
            {
                int32_t mode = c4gOpen.getMode(ARM);

                if (mode == C4G_OPEN_EXIT) keepGoingOn = false;
                else
                {
                    if (mode == C4G_OPEN_DRIVING_ON || mode == C4G_OPEN_MODE_0 || mode == C4G_OPEN_MODE_4)
                    {
                        if (mode == C4G_OPEN_MODE_4)
                        {
                            if (counter == 0)
                            {
                                for (int32_t i = 0; i < N_JOINTS; i++)
                                {
                                    if (sinAxisEnabled[i])
                                    {
                                        initialPositions[i] = c4gOpen.getActualPosition(ARM, i+1);
                                        previousPositions[i] = initialPositions[i];
                                        deltaPositions[i] = 0.0;
                                    }
                                }
                            }

                            for (int32_t i = 0; i < N_JOINTS; i++)
                            {
                                if (sinAxisEnabled[i])
                                {
                                    if (c4gOpen.isInDriveOn(ARM))
                                    {
                                        double omegat = 2 * PI * frequencyHz * ((double)counter * sampleTime);
                                        actualPositions[i] = initialPositions[i] + (float)(amplitudeGearRotations[i] * cos(omegat) - amplitudeGearRotations[i]);
                                        
                                        deltaPositions[i] = actualPositions[i] - previousPositions[i];
                                        
                                        previousPositions[i] = actualPositions[i];
                                    }

                                    c4gOpen.setTargetPosition(ARM, i+1, actualPositions[i]);
                                    c4gOpen.setTargetVelocity(ARM, i+1, deltaPositions[i]);
                                }
                            }
                            
                            if (c4gOpen.isInDriveOn(ARM)) counter++;
                        }

                        if (!c4gOpen.send()) keepGoingOn = false;
                        
                        if (c4gOpen.errorOccurred())
                        {
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
            else keepGoingOn = false;
        }
    }
    
    c4gOpen.stop();
    
    cout << "\nTestMode4 terminated.\n\n"; cout.flush();

    return 0;
}