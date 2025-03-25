/*
    TestMode0Debug.cpp

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

#define ARM 1

using namespace std;

const char disclaimer[] = "\n*********************************************************************\n"
                          "                           TestMode0Debug                              \n\n"
                          "   C4G Open Library - Copyright (C) 2007 Sintesi S.C.p.A.             \n\n"
                          "  Developers:                                                          \n"
                          "           Sabino   COLONNA (2006-, s.colonna@sintesi-scpa.com)    \n"
                          "           Giovanni IACCA   (2006-, g.iacca@sintesi-scpa.com  )    \n"
                          "           Giovanni TOTARO  (2006-, g.totaro@sintesi-scpa.com )    \n"
                          "\n*********************************************************************\n";

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

    cout << disclaimer << "\n";

    cout << "\nTestMode0Debug started...\n";
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

    if (c4gOpen.start())
    {
        bool keepGoingOn = true;

        while (keepGoingOn)
        {
            if (c4gOpen.receive())
            {
                int32_t mode = c4gOpen.getMode(ARM);

                if (mode == C4G_OPEN_EXIT)
                    keepGoingOn = false;
                else
                {

                    if (mode == C4G_OPEN_DRIVING_ON || mode == C4G_OPEN_MODE_0 || mode == C4G_OPEN_MODE_0_DEBUG)
                    {
                        if (!c4gOpen.send())
                            keepGoingOn = false;
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
    if (c4gOpen.errorOccurred())
    {

        cout.flush();
        c4gOpen.resetError();
    }

    c4gOpen.stop();

    return 0;
}