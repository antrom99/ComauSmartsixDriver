/* -------------------------------------------------------------------
    *
    * This module has been developed by the Automatic Control Group
    * of the University of Salerno, Italy.
    *
    * Title:   TestMode0Debug_print_joint_values.cpp
    * Author:  Fabrizio Di Domenico, Giovanni Mignone
    * Org.:    UNISA
    * Date:    Sep 05, 2019
    *
    * The aim of this test is verifying the communication between the PC 
    * and the C4G controller, both in transmission and in reception, 
    * since Mode 0’ requires the PC to send back the received packets. 
    * The test will last until a Mode 504 request has been scheduled on 
    * C4G via PDL2.
    * 
    * -------------------------------------------------------------------
*/

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#include <unistd.h>
#include <c4g_open_driver/C4gOpen.hpp>

#define ARM 1

using namespace std;

int32_t main(int32_t argc, char *argv[])
{
    int32_t port;

    // Get port from user's environment variable
    const char* env_var_port_number = std::getenv("C4G_PORT_NUMBER");

    if (env_var_port_number == NULL)
    {
        cout << "C4G_PORT_NUMBER was not found as an environment variable.\n";
        exit(1);
    }

    port = atoi(env_var_port_number);

    cout << "TestMode0Debug_print_joint_values started...\n";
    cout.flush();
    cout << "\n    After C4G starts:";
    cout.flush();
    cout << "\n       1. Drive On the robot";
    cout.flush();
    cout << "\n       2. Set mode 10 (0 Debug) via PDL2\n";
    cout.flush();
    cout << "\n    To stop the test:";
    cout.flush();
    cout << "\n       1. Drive Off the robot";
    cout.flush();
    cout << "\n       2. Set mode 504 via PDL2\n\n";
    cout.flush();

    C4gOpen c4gOpen(port);

    int32_t count = 0;

    cout << "\n[---     Waiting Initialization     ---]\n\n     Now you can boot the C4G Controller     \n" << endl;

    if (c4gOpen.start()) // Wait until the communication starts.
    {
        cout << "\nStart in mode: " << c4gOpen.getMode(ARM);

        while (!c4gOpen.errorOccurred() && c4gOpen.getMode(ARM) != C4G_OPEN_EXIT) // Check communication errors.
        {
            if (c4gOpen.receive()) // Receive the communication packet.
            {
                count = count + 1;

                if (count == 100) // Delay prints every 100 packets.
                {

                    cout << "\nMode: " << c4gOpen.getMode(ARM);
                    cout << "\nNumber of axes in open: " << c4gOpen.getNumberOfOpenAxes();
                    cout << "\nDrive on: " << c4gOpen.isInDriveOn(ARM) << endl;

                    for (int32_t i = 0; i < 7; i++)
                    {

                        cout << "\n-------------------------";
                        cout << "\nAxis: " << i << " Calibration constant: " << c4gOpen.getCalibrationConstant(ARM, i + 1);
                        cout << fixed << setprecision(4) << "\nSpeed: " << c4gOpen.getActualVelocity(ARM, i + 1) << " Position: " << c4gOpen.getActualPosition(ARM, i + 1);
                        cout << fixed << setprecision(4) << "\nTarget speed: " << c4gOpen.getTargetVelocity(ARM, i + 1) << " Target position: " << c4gOpen.getTargetPosition(ARM, i + 1);
                        cout << "\n-------------------------" << endl;
                    }
                    cout << "\n \n \n \n " << endl;
                    count = 0;
                }

                c4gOpen.send(); // Send the packet.
            }
        }
    }

    cout << "\nExit from mode: " << c4gOpen.getMode(ARM) << endl;
    c4gOpen.stop(); // Stop the communication.
    cout << "\nEnd program " << endl;

    return 0;
}