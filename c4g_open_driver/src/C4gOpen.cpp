/*
    C4gOpen.cpp

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

/**
    @file C4gOpen.cpp
    @brief Implementation of methods of #C4gOpen class.

    This file contains the implementation of all methods, both private and public ones,
    of the #C4gOpen class to allow a PC to communicate with Comau C4Gopen.
*/

#include <c4g_open_driver/C4gOpen.hpp>
#include <string.h>
#include <unistd.h>
#include <iostream>

// Default constructor.
/**
    This method constructs an instance of the #C4gOpen class, initializing some of its members.
*/
C4gOpen::C4gOpen(int32_t port): 
                    socketID(-1),
                    portNumber(port),
                    communicationTimeout(COMMUNICATION_TIMEOUT),
                    numberOfOpenAxes(0),
                    sampleTime(-1),
                    lastError(NO_ERROR)
{
    c4gSocketLength = sizeof(sockaddr_in);
    startUpPacket = malloc(MAX_SIZE_OF_PACKETS);

    for (int32_t i = 0; i < MAX_NUM_ARMS * MAX_NUM_AXES_PER_ARM; i++)
        logicalToOpenMap[i] = -1;

    for (int32_t i = 0; i < MAX_NUM_ARMS; i++)
    {
        exitFromOpenDone[i] = false;
        canChangeMode[i] = true;
    }

    resetFlags();

    resetInitPacket();
    resetCommPacketRx();
    resetCommPacketTx();
    for (int32_t i = 0; i < MAX_NUM_OPEN_AXES; i++)
        commPacketTx.axisData[i].mode = -1;
}

// Destructor.
C4gOpen::~C4gOpen()
{
    free(startUpPacket);
}

// Set fields of the initialization packet to their default value.
/**
    This method is called in the #C4gOpen class constructor.
*/
void C4gOpen::resetInitPacket()
{
    initPacket.seqNumber = 0;
    initPacket.numberOfFieldsPerPacket = 0;
    initPacket.numberOfFieldsPerAxis = 0;
    initPacket.numberOfAxesInOpenMode = 0;

    initPacket.typeOfHeaderField1 = 0;
    initPacket.typeOfHeaderField2 = 0;
    initPacket.typeOfHeaderField3 = 0;
    initPacket.typeOfAxisDataField1 = 0;
    initPacket.typeOfAxisDataField2 = 0;
    initPacket.typeOfAxisDataField3 = 0;
    initPacket.typeOfAxisDataField4 = 0;
    initPacket.typeOfAxisDataField5 = 0;
    initPacket.typeOfAxisDataField6 = 0;
    initPacket.typeOfAxisDataField7 = 0;
    initPacket.typeOfAxisDataField8 = 0;
    initPacket.typeOfAxisDataField9 = 0;

    initPacket.sampleTime = 0;

    initPacket.majorNumber = 0;
    initPacket.minorNumber = 0;
    initPacket.buildNumber = 0;

    for (int32_t i = 0; i < MAX_NUM_AXES_PER_ARM; i++)
    {
        initPacket.arm1OpenMode[i] = -1;
        initPacket.arm2OpenMode[i] = -1;
        initPacket.arm3OpenMode[i] = -1;
        initPacket.arm4OpenMode[i] = -1;

        initPacket.arm1OpenAxesMap[i] = -1;
        initPacket.arm2OpenAxesMap[i] = -1;
        initPacket.arm3OpenAxesMap[i] = -1;
        initPacket.arm4OpenAxesMap[i] = -1;

        initPacket.arm1CalibrationConstant[i] = NAN;
        initPacket.arm2CalibrationConstant[i] = NAN;
        initPacket.arm3CalibrationConstant[i] = NAN;
        initPacket.arm4CalibrationConstant[i] = NAN;

        initPacket.arm1CurrentLimit[i] = NAN;
        initPacket.arm2CurrentLimit[i] = NAN;
        initPacket.arm3CurrentLimit[i] = NAN;
        initPacket.arm4CurrentLimit[i] = NAN;

        initPacket.arm1TxRate[i] = NAN;
        initPacket.arm2TxRate[i] = NAN;
        initPacket.arm3TxRate[i] = NAN;
        initPacket.arm4TxRate[i] = NAN;

        initPacket.arm1KinInflCoeff[i] = NAN;
        initPacket.arm2KinInflCoeff[i] = NAN;
        initPacket.arm3KinInflCoeff[i] = NAN;
        initPacket.arm4KinInflCoeff[i] = NAN;

        initPacket.arm1FollowingError[i] = NAN;
        initPacket.arm2FollowingError[i] = NAN;
        initPacket.arm3FollowingError[i] = NAN;
        initPacket.arm4FollowingError[i] = NAN;
    }
}

// Set fields of the incoming communication packet to their default value.
/**
    This method is called in the #C4gOpen class constructor and before receiving a new communication packet from C4G.
*/
void C4gOpen::resetCommPacketRx()
{
    commPacketRx.header.seqNumber = 0;
    commPacketRx.header.status = 0;
    commPacketRx.header.functionality = 0;

    for (int32_t i = 0; i < MAX_NUM_OPEN_AXES; i++)
    {
        commPacketRx.axisData[i].mode = -1;
        commPacketRx.axisData[i].targetPosition = 0.0;
        commPacketRx.axisData[i].targetVelocity = 0.0;
        commPacketRx.axisData[i].actualPosition = 0.0;
        commPacketRx.axisData[i].actualVelocity = 0.0;
        commPacketRx.axisData[i].targetCurrent = 0.0;
        commPacketRx.axisData[i].extra1 = 0.0;
        commPacketRx.axisData[i].extra2 = 0.0;
        commPacketRx.axisData[i].extra3 = 0.0;
    }
}

// Set fields of the outgoing communication packet to their default value.
/**
    This method is called in the #C4gOpen class constructor and after having sent a new communication packet to C4G.
*/
void C4gOpen::resetCommPacketTx()
{
    commPacketTx.header.seqNumber = 0;
    commPacketTx.header.status = 0;
    commPacketTx.header.functionality = 0;

    for (int32_t i = 0; i < MAX_NUM_OPEN_AXES; i++)
    {
        commPacketTx.axisData[i].targetPosition = 0.0;
        commPacketTx.axisData[i].targetVelocity = 0.0;
        commPacketTx.axisData[i].measure = 0.0;
        commPacketTx.axisData[i].ffwVelocity = 0.0;
        commPacketTx.axisData[i].ffwCurrent = 0.0;
        commPacketTx.axisData[i].extra1 = 0.0;
        commPacketTx.axisData[i].extra2 = 0.0;
        commPacketTx.axisData[i].extra3 = 0.0;
    }
}

// Set all the flags to their default value.
/**
    This method is called in the #C4gOpen class constructor and after having sent a new communication packet to C4G.
*/
void C4gOpen::resetFlags()
{
    for (int32_t i = 0; i < MAX_NUM_ARMS; i++)
        setModeDone[i] = false;

    for (int32_t i = 0; i < MAX_NUM_OPEN_AXES; i++)
    {
        setTargetPositionDone[i] = false;
        setTargetVelocityDone[i] = false;
        setMeasureDone[i] = false;
        setFeedForwardVelocityDone[i] = false;
        setFeedForwardCurrentDone[i] = false;
        setDeltaCurrentDone[i] = false;
        followingErrorOvercome[i] = false;
    }
}

// Initialize a #C4gOpen instance. This function modifies the current thread priority 
// in such a way it becomes a real time thread: this goal is achieved by using 
// the pthread Linux library and assigning high execution priority to the thread 
// (from pthread documentation at https://man7.org/linux/man-pages/man3/pthread_setschedparam.3.html,
// any priority between 1 and 99 makes the thread real-time: the value used is 80).
/**
    @return True if all the initialization procedure goes well, false otherwise.
*/
bool C4gOpen::start()
{
    bool retValue = true;
    int32_t ret;

    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
        printf("mlockall failed: %m\n");
        exit(-2);
    }

    // We'll operate on the currently running thread.
    pthread_t this_thread = pthread_self();

    // struct sched_param is used to store the scheduling priority
    struct sched_param params;

    params.sched_priority = 80;

    // Attempt to set thread real-time priority to the SCHED_FIFO policy
    ret = pthread_setschedparam(this_thread, SCHED_FIFO, &params);
    if (ret != 0) {
        // Print the error
        std::cout << "Unsuccessful in setting thread realtime priority" << std::endl;
        return false;     
    }

    if (initSocket())
    {
        retValue = waitInitPacket();
    }
    else
    {
        lastError = SOCKET_INIT_ERROR;
        retValue = false;
    }
    return retValue;
}

// Initialize a socket.
/**
    This method creates an UDP socket and binds it to port #portNumber, on which the server running on PC
    listens to C4G client requests.

    @return True if the socket is successfully created and bound, false otherwise and #lastError is appropriately set:
            <ul>			
            <li>SOCKET_OPEN_ERROR if the socket has not been successfully created;</li>
            <li>SOCKET_BINDING_ERROR if the socket has not been successfully bound.</li>
            </ul>
*/
bool C4gOpen::initSocket()
{
    bool retValue = true;
    socketID = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketID >= 0)
    {
        memset(&socketStruct, 0, sizeof(socketStruct));
        socketStruct.sin_family = AF_INET;
        socketStruct.sin_addr.s_addr = htonl(INADDR_ANY);
        socketStruct.sin_port = htons(portNumber);

        // This priority was 7 in the original driver, but setting a priority outside the 
        // range 0 to 6 requires the CAP_NET_ADMIN capability or root privileges.
        // In the case of ACG robots, there is a dedicated connection with the robots,
        // so any priority should work fine, in principle.
        int32_t priority = 6;
        if (setsockopt(socketID, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(int32_t)) == -1) {
            retValue = false;
        }
        int32_t tos_local = 0x28;
        if (setsockopt(socketID, IPPROTO_IP, IP_TOS, &tos_local, sizeof(tos_local)) == -1) {
            retValue = false;
        }

        if (bind(socketID, (struct sockaddr *)&socketStruct, sizeof(struct sockaddr_in)) < 0)
        {
            lastError = SOCKET_BINDING_ERROR;
            retValue = false;
        }
    }
    else
    {
        lastError = SOCKET_OPEN_ERROR;
        retValue = false;
    }
    return retValue;
}

// Wait for the arrival of the initialization packet.
/**
    This method handles the reception of the initialization packet from C4G.

    If C4G has been restarted, this method receives 9 synchronization packets, then it receives
    the inizialization packet (with sequence number equal to 10) and calls #readInitPacket(), eventually
    it receives other 2 synchronization packets before starting the normal communication.

    If C4G client has been restarted (special mode 506), this method receives immediately the
    inizialization packet (with sequence number equal to 10) and calls #readInitPacket().

    When a timeout set to #WAIT_INIT_PCK_TIMEOUT expires, this method returns false.

    @return True if the initialization packet is successfully received, false otherwise and #lastError is appropriately set:
            <ul>
            <li>INIT_PACKET_UNEXPECTED_SEQ_NUM if the initialization packet has an unexpected sequence number;</li>
            <li>INIT_PACKET_RECEIVE_ERROR if the initialization packet has not been successfully received.</li>
            </ul>
*/
bool C4gOpen::waitInitPacket()
{
    bool retValue = true;
    setCommunicationTimeout(WAIT_INIT_PCK_TIMEOUT);
    int32_t received = receivePacket(startUpPacket, sizeof(InitPacket));
    if (received >= 0)
    {
        if (received == sizeof(InitPacket)) // Client restart (mode 506)
        {
            if (((InitPacket *)startUpPacket)->seqNumber == 10)
            {
                if (!readInitPacket())
                {
                    lastError = INIT_PACKET_RECEIVE_ERROR;
                    retValue = false;
                }
            }
            else
            {
                lastError = INIT_PACKET_UNEXPECTED_SEQ_NUM;
                retValue = false;
            }
        }
        else // C4G restart

        {
            sendPacket(startUpPacket, sizeof(commPacketTx));
            for (int32_t i = 0; i < 8; i++)
            {
                receivePacket(startUpPacket, MAX_SIZE_OF_PACKETS);
                sendPacket(startUpPacket, sizeof(commPacketTx));
            }

            if (receivePacket(startUpPacket, sizeof(InitPacket)) == sizeof(InitPacket))
            {
                if (((InitPacket *)startUpPacket)->seqNumber == 10)
                {
                    if (!readInitPacket())
                    {
                        lastError = INIT_PACKET_RECEIVE_ERROR;
                        retValue = false;
                    }
                }
                else
                {
                    lastError = INIT_PACKET_UNEXPECTED_SEQ_NUM;
                    retValue = false;
                }
            }
            else
                retValue = false;

            if (retValue)
            {
                for (int32_t i = 0; i < 2; i++)
                {
                    receivePacket(startUpPacket, MAX_SIZE_OF_PACKETS);
                    sendPacket(startUpPacket, sizeof(commPacketTx));
                }
            }
        }
    }
    else
        retValue = false;

    setCommunicationTimeout(COMMUNICATION_TIMEOUT);
    return retValue;
}

// Read a just received initialization packet.
/**
    This method reads a just received initialization packet, extracting informations to be
    passed to #mapIndices() in order to map open axes to logical ones.

    @return True if the initialization packet is successfully read and replied, false otherwise.
*/
bool C4gOpen::readInitPacket()
{
    bool retValue = true;

    initPacket = *(InitPacket *)startUpPacket;

    numberOfOpenAxes = initPacket.numberOfAxesInOpenMode;
    sampleTime = initPacket.sampleTime;

    mapIndices(1, initPacket.arm1OpenAxesMap);
    mapIndices(2, initPacket.arm2OpenAxesMap);
    mapIndices(3, initPacket.arm3OpenAxesMap);
    mapIndices(4, initPacket.arm4OpenAxesMap);

    if (sendPacket(startUpPacket, sizeof(InitPacket)) != sizeof(InitPacket))
        retValue = false;

    return retValue;
}

// Receive a packet from C4G.
/**
    This method receives a packet coming from C4G, by calling recvfrom().

    @param packet Pointer to the receiving buffer.
    @param bytesToBeReceived Number of bytes to be received.

    @return The number of received bytes.
*/
int32_t C4gOpen::receivePacket(void *packet, int32_t bytesToBeReceived)
{
    return recvfrom(socketID, packet, bytesToBeReceived, 0, (sockaddr *)&c4gSocketStruct, &c4gSocketLength);
}

// Send a packet to C4G.
/**
    This method updates the packet sequence number, then sends the packet to C4G, by calling sendto().

    @param packet Pointer to the buffer containing the packet to be sent.
    @param bytesToSend Number of bytes to be sent.

    @return The number of sent bytes.
*/
int32_t C4gOpen::sendPacket(void *packet, int32_t bytesToSend)
{
    *(int32_t *)packet += 1;
    if (*(int32_t *)packet > 0x7FFF)
        *(int32_t *)packet = 1;

    return sendto(socketID, packet, bytesToSend, 0, (sockaddr *)&c4gSocketStruct, sizeof(c4gSocketStruct));
}

// Receive a communication packet from C4G.
/**
    This method receives a communication packet coming from C4G after having set fields of the incoming communication packet to their default value
    by calling #resetCommPacketRx().

    @return True if the number of received bytes equals the size of an Rx communication packet (with #numberOfOpenAxes fields), false otherwise
            and #lastError is set to SOCKET_RECEIVE_ERROR.
*/
bool C4gOpen::receive()
{
    bool retValue = true;
    int32_t bytesToBeReceived = sizeof(Header) + numberOfOpenAxes * sizeof(AxisDataRx);

    resetCommPacketRx();

    if (!errorOccurred())
    {
        if (receivePacket((CommPacketRx *)&commPacketRx, bytesToBeReceived) != bytesToBeReceived)
        {
            lastError = SOCKET_RECEIVE_ERROR;
            retValue = false;
        }
    }
    else
        retValue = false;

    return retValue;
}

// Send a communication packet to C4G.
/**
    This method sends a communication packet to C4G, then set fields of the outgoing communication packet to their default value by calling #resetCommPacketTx().
    If any error has occurred, this method schedules a DRIVE OFF request, by setting the #AxisDataTx::mode field to C4G_OPEN_DRIVE_OFF.
    If a following error threshold has been overcome (see #checkFollowingError()), the #AxisDataTx::mode field is set to C4G_OPEN_FOLLOWING_ERROR.

    @return True if the packet has been successfully sent, that is number of sent bytes equals the size of a Tx communication packet
            (with #numberOfOpenAxes fields), false otherwise and #lastError is set to SOCKET_SEND_ERROR.
*/
bool C4gOpen::send()
{
    bool retValue = true;
    int32_t bytesToSend = sizeof(Header) + numberOfOpenAxes * sizeof(AxisDataTx);

    commPacketTx.header.seqNumber = commPacketRx.header.seqNumber;

    for (int32_t i = 0; i < MAX_NUM_ARMS; i++)
    {
        if (exitFromOpenDone[i])
        {
            canChangeMode[i] = true;
            exitFromOpenDone[i] = false;
        }

        for (int32_t j = 0; j < MAX_NUM_AXES_PER_ARM; j++)
        {
            int32_t openIndex = getOpenIndex(i + 1, j + 1);
            if (openIndex != -1)
            {
                if (!setModeDone[i])
                    commPacketTx.axisData[openIndex].mode = commPacketRx.axisData[openIndex].mode;

                if (commPacketTx.axisData[openIndex].mode == C4G_OPEN_MODE_0_DEBUG)
                {
                    commPacketTx.axisData[openIndex].targetPosition = commPacketRx.axisData[openIndex].targetPosition;
                    commPacketTx.axisData[openIndex].targetVelocity = commPacketRx.axisData[openIndex].targetVelocity;
                    commPacketTx.axisData[openIndex].measure = commPacketRx.axisData[openIndex].actualPosition;
                    commPacketTx.axisData[openIndex].ffwVelocity = commPacketRx.axisData[openIndex].actualVelocity;
                    commPacketTx.axisData[openIndex].ffwCurrent = commPacketRx.axisData[openIndex].targetCurrent;
                }
            }
        }
    }

    checkOperations();

    if (errorOccurred())
    {
        bool followingErrorFound = false;

        for (int32_t i = 0; i < numberOfOpenAxes && !followingErrorFound; i++)
        {
            if (followingErrorOvercome[i])
            {
                commPacketTx.axisData[i].mode = C4G_OPEN_FOLLOWING_ERROR;
                followingErrorFound = true;
            }
        }

        if (!followingErrorFound)
        {
            for (int32_t i = 0; i < numberOfOpenAxes; i++)
                commPacketTx.axisData[i].mode = C4G_OPEN_DRIVE_OFF;
        }
    }

    if (sendPacket((CommPacketTx *)&commPacketTx, bytesToSend) == bytesToSend)
    {
        resetCommPacketTx();
        resetFlags();
    }
    else
    {
        lastError = SOCKET_SEND_ERROR;
        retValue = false;
    }

    return retValue;
}

// Finalize a #C4gOpen instance.
/**
    This method calls #closeSocket() to close a previously opened socket.
*/
void C4gOpen::stop()
{
    closeSocket();
}

// Close a previously opened socket.
void C4gOpen::closeSocket()
{
    if (socketID != -1)
        close(socketID);
}

// Get the communication timeout.
/**
    @return The socket timeout during normal communication between C4G and PC in nanoseconds.
*/
int64_t C4gOpen::getCommunicationTimeout()
{
    return communicationTimeout;
}

// Set the communication timeout.
/**
    @param timeout The socket timeout during normal communication between C4G and PC in nanoseconds.
*/
void C4gOpen::setCommunicationTimeout(int64_t timeout)
{
    communicationTimeout = timeout;
    struct timeval tv;
    tv.tv_sec = communicationTimeout;
    tv.tv_usec = 0;
    if (setsockopt(socketID, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        lastError = SOCKET_SET_RCV_TIMEOUT_ERROR;
}

// Check if an error occurred.
/**
    @return True if an error occurred, false otherwise.
*/
bool C4gOpen::errorOccurred()
{
    if (lastError != NO_ERROR)
        return true;
    else
        return false;
}

// Get the value of the last occurred error.
/**
    @return The value of the last occurred error.
*/
ErrorCodes C4gOpen::getLastError()
{
    return lastError;
}

// Reset the last occurred error.
/**
    This method clears the last occurred error, allowing to keep the communication going on when, e.g.,
    "Drive Off" command is sent to C4G or a "following error overcome" is detected by the PC.
*/
void C4gOpen::resetError()
{
    lastError = NO_ERROR;
}

// Map open axes to logical ones.
/**
    This method reads the \em armOpenAxesMap field of the initialization packet passed as argument
    and initializes #logicalToOpenMap array in order to create a mapping between open axes and logical ones
    that is used when the fields of an outgoing communication packet are written (by calling #getOpenIndex()).

    @param arm Number of the arm.
    @param armOpenAxesMap Open/Logical axes map of the specified arm.
*/
void C4gOpen::mapIndices(int32_t arm, int32_t armOpenAxesMap[MAX_NUM_AXES_PER_ARM])
{
    for (int32_t i = 0; i < MAX_NUM_AXES_PER_ARM; i++)
    {
        if (armOpenAxesMap[i] != 0xFF)
            logicalToOpenMap[(arm - 1) * MAX_NUM_AXES_PER_ARM + i] = armOpenAxesMap[i];
    }
}

// Get open index of an axis.
/**
    This method retrieves the open index of an axis (i.e. the index used inside the communication packet)
    from the logical one (i.e. the index used by an user), by means of #logicalToOpenMap array.

    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Open index of the specified axis.
*/
int32_t C4gOpen::getOpenIndex(int32_t arm, int32_t axis)
{
    return logicalToOpenMap[(arm - 1) * MAX_NUM_AXES_PER_ARM + (axis - 1)];
}

// Check validity of an arm.
/**
    This method ckecks if the specified arm is a valid one, i.e. if the arm is in the range [1-4] and is open.

    @param arm Number of the arm.
    @return True if the specified arm is valid, false otherwise and #lastError is appropriately set:
            <ul>
            <li>INVALID_ARM_NUMBER if the arm is not in the range [1-4];</li>
            <li>ARM_NOT_IN_OPEN_MODE if the arm is not open.</li>
            </ul>
*/
bool C4gOpen::checkArm(int32_t arm)
{
    bool retValue = true;
    bool isOpenArm = false;

    if (arm >= 1 && arm <= MAX_NUM_ARMS)
    {
        for (int32_t i = 0; i < MAX_NUM_AXES_PER_ARM && !isOpenArm; i++)
        {
            if (getOpenIndex(arm, i + 1) != -1)
                isOpenArm = true;
        }
        if (!isOpenArm)
        {
            lastError = ARM_NOT_IN_OPEN_MODE;
            retValue = false;
        }
    }
    else
    {
        lastError = INVALID_ARM_NUMBER;
        retValue = false;
    }

    return retValue;
}

// Check validity of an axis.
/**
    This method ckecks if the specified arm is a valid one, by calling #checkArm(),
    and if the specified axis is a valid one, i.e. if the axis is in the range [1-10] and is open.

    @param arm Number of the arm which the axis belongs to.
    @param axis Number of the axis.
    @return True if the specified axis is valid, false otherwise and #lastError is appropriately set:
            <ul>
            <li>INVALID_AXIS_NUMBER if the axis is not in the range [1-10];</li>
            <li>AXIS_NOT_IN_OPEN_MODE if the axis is not open.</li>
            </ul>
*/
bool C4gOpen::checkAxis(int32_t arm, int32_t axis)
{
    bool retValue = true;

    if (checkArm(arm))
    {
        if (axis >= 1 && axis <= MAX_NUM_AXES_PER_ARM)
        {
            if (getOpenIndex(arm, axis) == -1)
            {
                lastError = AXIS_NOT_IN_OPEN_MODE;
                retValue = false;
            }
        }
        else
        {
            lastError = INVALID_AXIS_NUMBER;
            retValue = false;
        }
    }
    else
        retValue = false;

    return retValue;
}

// Get the number of axes in open mode.
/**
    @return Number of axes in open mode.
*/
int32_t C4gOpen::getNumberOfOpenAxes()
{
    return numberOfOpenAxes;
}

// Get the communication cycle time in milliseconds.
/**
    @return Communication cycle time in milliseconds.
*/
int32_t C4gOpen::getSampleTime()
{
    return sampleTime;
}

// Get the calibration constant of an axis in gear rotations.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Calibration constant of the specified axis in gear rotations.
*/
float C4gOpen::getCalibrationConstant(int32_t arm, int32_t axis)
{
    float calibrationConstant = NAN;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            switch (arm)
            {
            case 1:
                calibrationConstant = initPacket.arm1CalibrationConstant[axis - 1];
                break;
            case 2:
                calibrationConstant = initPacket.arm2CalibrationConstant[axis - 1];
                break;
            case 3:
                calibrationConstant = initPacket.arm3CalibrationConstant[axis - 1];
                break;
            case 4:
                calibrationConstant = initPacket.arm4CalibrationConstant[axis - 1];
                break;
            }

            return calibrationConstant;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Get the current limit of an axis in ampere.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Current limit of the specified axis in ampere.
*/
float C4gOpen::getCurrentLimit(int32_t arm, int32_t axis)
{
    float currentLimit = NAN;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            switch (arm)
            {
            case 1:
                currentLimit = initPacket.arm1CurrentLimit[axis - 1];
                break;
            case 2:
                currentLimit = initPacket.arm2CurrentLimit[axis - 1];
                break;
            case 3:
                currentLimit = initPacket.arm3CurrentLimit[axis - 1];
                break;
            case 4:
                currentLimit = initPacket.arm4CurrentLimit[axis - 1];
                break;
            }

            return currentLimit;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Get the transmission rate of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Transmission rate of the specified axis.
*/
float C4gOpen::getTxRate(int32_t arm, int32_t axis)
{
    float txRate = NAN;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            switch (arm)
            {
            case 1:
                txRate = initPacket.arm1TxRate[axis - 1];
                break;
            case 2:
                txRate = initPacket.arm2TxRate[axis - 1];
                break;
            case 3:
                txRate = initPacket.arm3TxRate[axis - 1];
                break;
            case 4:
                txRate = initPacket.arm4TxRate[axis - 1];
                break;
            }

            return txRate;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Get the kinematic influence coefficient of axis 4 on axis 5.
/**
    @param arm Number of the arm.
    @return Kinematic influence coefficient of axis 4 on axis 5.
*/
float C4gOpen::getKinInflCoeff54(int32_t arm)
{
    float kinInflCoeff54 = NAN;

    if (!errorOccurred())
    {
        if (checkArm(arm))
        {
            switch (arm)
            {
            case 1:
                kinInflCoeff54 = initPacket.arm1KinInflCoeff[3];
                break;

            case 2:
                kinInflCoeff54 = initPacket.arm2KinInflCoeff[3];
                break;

            case 3:
                kinInflCoeff54 = initPacket.arm3KinInflCoeff[3];
                break;

            case 4:
                kinInflCoeff54 = initPacket.arm4KinInflCoeff[3];
                break;
            }

            return kinInflCoeff54;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Get the kinematic influence coefficient of axis 4 on axis 6.
/**
    @param arm Number of the arm.
    @return Kinematic influence coefficient of axis 4 on axis 6.
*/
float C4gOpen::getKinInflCoeff64(int32_t arm)
{
    float kinInflCoeff64 = NAN;

    if (!errorOccurred())
    {
        if (checkArm(arm))
        {
            switch (arm)
            {
            case 1:
                kinInflCoeff64 = initPacket.arm1KinInflCoeff[4];
                break;

            case 2:
                kinInflCoeff64 = initPacket.arm2KinInflCoeff[4];
                break;

            case 3:
                kinInflCoeff64 = initPacket.arm3KinInflCoeff[4];
                break;

            case 4:
                kinInflCoeff64 = initPacket.arm4KinInflCoeff[4];
                break;
            }

            return kinInflCoeff64;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Get the kinematic influence coefficient of axis 6 on axis 5.
/**
    @param arm Number of the arm.
    @return Kinematic influence coefficient of axis 6 on axis 5.
*/
float C4gOpen::getKinInflCoeff65(int32_t arm)
{
    float kinInflCoeff65 = NAN;

    if (!errorOccurred())
    {
        if (checkArm(arm))
        {
            switch (arm)
            {
            case 1:
                kinInflCoeff65 = initPacket.arm1KinInflCoeff[5];
                break;

            case 2:
                kinInflCoeff65 = initPacket.arm2KinInflCoeff[5];
                break;

            case 3:
                kinInflCoeff65 = initPacket.arm3KinInflCoeff[5];
                break;

            case 4:
                kinInflCoeff65 = initPacket.arm4KinInflCoeff[5];
                break;
            }

            return kinInflCoeff65;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Get the following error threshold of an axis in gear rotations.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Following error threshold of the specified axis in gear rotations.
*/
float C4gOpen::getFollowingErrorThreshold(int32_t arm, int32_t axis)
{
    float followingErrorThreshold = NAN;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            switch (arm)
            {
            case 1:
                followingErrorThreshold = initPacket.arm1FollowingError[axis - 1];
                break;
            case 2:
                followingErrorThreshold = initPacket.arm2FollowingError[axis - 1];
                break;
            case 3:
                followingErrorThreshold = initPacket.arm3FollowingError[axis - 1];
                break;
            case 4:
                followingErrorThreshold = initPacket.arm4FollowingError[axis - 1];
                break;
            }

            return followingErrorThreshold;
        }
        else
            return NAN;
    }
    else
        return NAN;
}

// Check if an arm is in "Drive On".
/**
    @param arm Number of the arm.
    @return	True if the specified arm is in "Drive On", false otherwise.
*/
bool C4gOpen::isInDriveOn(int32_t arm)
{
    bool retValue = true;
    int32_t *armStatus = &commPacketRx.header.status;

    if (!errorOccurred())
    {
        if (checkArm(arm))
        {
            if (*((char *)armStatus + (arm - 1)) != DRIVE_ON)
                retValue = false;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Order an arm to exit from open mode.
/**
    @param arm Number of the arm.
    @return True if the specified arm is valid, false otherwise.
*/
bool C4gOpen::exitFromOpen(int32_t arm)
{
    bool retValue = true;
    int32_t *armStatus = &commPacketTx.header.status;

    if (!errorOccurred())
    {
        if (checkArm(arm))
        {
            *((char *)armStatus + (arm - 1)) = EXIT_FROM_OPEN;
            exitFromOpenDone[arm - 1] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get open mode functionality.
/**
    This method gets the functionality provided for a certain open mode on the specified axis.

    @param arm Number of the arm.
    @return The open mode functionality, -1 if an error has occurred or the specified arm and/or axis are not valid.
*/
int32_t C4gOpen::getFunctionality(int32_t arm)
{
    int32_t *functionality = &commPacketRx.header.functionality;

    if (!errorOccurred())
    {
        if (checkArm(arm))
            return *((char *)functionality + (arm - 1));
        else
            return -1;
    }
    else
        return -1;
}

// Get the open mode of an arm.
/**
    @param arm Number of the arm.
    @return The open mode of the specified arm.
*/
int32_t C4gOpen::getMode(int32_t arm)
{
    int32_t mode = -1;

    if (!errorOccurred())
    {
        if (checkArm(arm))
        {
            for (int32_t i = 0; i < MAX_NUM_AXES_PER_ARM && mode == -1; i++)
            {
                int32_t openIndex = getOpenIndex(arm, i + 1);
                if (openIndex != -1)
                    mode = commPacketRx.axisData[openIndex].mode;
            }
        }
    }

    return mode;
}

// Set the open mode of an arm.
/**
    This method sets the open mode of an arm, if and only if:
    <ul>
    <li>an #exitFromOpen() has called in the previous step;</li>
    <li>or the actual mode is 0;</li>
    <li>or the specified mode is equal to the actual one.</li>
    </ul>

    @param arm Number of the arm.
    @param mode Open mode to set.

    @return	True if the mode is successfully set, false otherwise and #lastError is appropriately set:
            <ul>
            <li>INVALID_OPEN_MODE if the specified mode is not a valid open mode (i.e. it is not included in #validOpenModes);</li>
            <li>SET_MODE_NOT_ALLOWED if the operation is not permitted (i.e. the above conditions are not satisfied).</li>
            </ul>
*/
bool C4gOpen::setMode(int32_t arm, int32_t mode)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (canChangeMode[arm - 1] || getMode(arm) == 0 || mode == getMode(arm))
        {
            if (checkArm(arm))
            {
                if (checkMode(mode))
                {
                    for (int32_t i = 0; i < MAX_NUM_AXES_PER_ARM; i++)
                    {
                        int32_t openIndex = getOpenIndex(arm, i + 1);
                        if (openIndex != -1)
                            commPacketTx.axisData[openIndex].mode = mode;
                    }

                    if (canChangeMode[arm - 1])
                        canChangeMode[arm - 1] = false;

                    setModeDone[arm - 1] = true;
                }
                else
                {
                    lastError = INVALID_OPEN_MODE;
                    retValue = false;
                }
            }
            else
                retValue = false;
        }
        else
        {
            lastError = SET_MODE_NOT_ALLOWED;
            retValue = false;
        }
    }
    else
        retValue = false;

    return retValue;
}

// Check if a given mode is valid.
/**
    This method checks if the specified open mode is a valid open mode, i.e. it is included in #validOpenModes. 

    @param mode The mode to be set.
    @return True if the specified mode is a valid open mode, false otherwise.
*/
bool C4gOpen::checkMode(int32_t mode)
{
    bool retValue = false;
    uint32_t numberOfValidOpenModes = sizeof(validOpenModes) / sizeof(int32_t);

    for (uint32_t i = 0; i < numberOfValidOpenModes && !retValue; i++)
        if (mode == validOpenModes[i])
            retValue = true;

    return retValue;
}

// Get the target position of an axis in gear rotations.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Target position of the specified axis in gear rotations,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getTargetPosition(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].targetPosition;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set the target position of an axis in gear rotations.
/**
    This method sets the target position of the specified axis, checking if the following error threshold has been overcome.
    
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param targetPosition Target position of the specified axis in gear rotations.
    @return True if the target position has been successfully set, false if:
            <ul>
            <li>the specified arm and/or axis are not valid;</li>
            <li>the following error threshold is overcome.</li>
            </ul>
*/
bool C4gOpen::setTargetPosition(int32_t arm, int32_t axis, float targetPosition)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            checkFollowingError(arm, axis, targetPosition);
            commPacketTx.axisData[getOpenIndex(arm, axis)].targetPosition = targetPosition;
            setTargetPositionDone[getOpenIndex(arm, axis)] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get the target velocity of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Target velocity of the specified axis,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getTargetVelocity(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].targetVelocity;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set the target velocity of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param targetVelocity Target velocity of the specified axis.
    @return True if the target velocity has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setTargetVelocity(int32_t arm, int32_t axis, float targetVelocity)
{

    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            // Add check on the targetVelocity
            commPacketTx.axisData[getOpenIndex(arm, axis)].targetVelocity = targetVelocity;
            setTargetVelocityDone[getOpenIndex(arm, axis)] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get the actual position of an axis in gear rotations.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Actual position of the specified axis in gear rotations,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getActualPosition(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].actualPosition;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set the measure given by an external sensor.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param measure Measure given by the external sensor.
    @return True if the measure has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setMeasure(int32_t arm, int32_t axis, float measure)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            // Add check on the measure
            commPacketTx.axisData[getOpenIndex(arm, axis)].measure = measure;
            setMeasureDone[getOpenIndex(arm, axis)] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get the actual velocity of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Actual velocity of the specified axis,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getActualVelocity(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].actualVelocity;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set the feedforward velocity of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param ffwVelocity Feedforward velocity of the specified axis.
    @return True if the feedforward velocity has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setFeedForwardVelocity(int32_t arm, int32_t axis, float ffwVelocity)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            // Add check on the ffwVelocity
            commPacketTx.axisData[getOpenIndex(arm, axis)].ffwVelocity = ffwVelocity;
            setFeedForwardVelocityDone[getOpenIndex(arm, axis)] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get the target current of an axis in ampere.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Target current of the specified axis in ampere,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getTargetCurrent(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].targetCurrent;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set the feedforward current of an axis.
/**
    This method sets the feedforward current of the specified axis, checking if the given value overcomes current limit of the specified axis:
    if so this method sets the feedforward current to the current limit of the specified axis.

    @param arm Number of the arm.
    @param axis Number of the axis.
    @param ffwCurrent Feedforward current of the specified axis.
    @return True if the feedforward current has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setFeedForwardCurrent(int32_t arm, int32_t axis, float ffwCurrent)
{
    bool retValue = true;
    float currentLimit = getCurrentLimit(arm, axis);

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            if (ffwCurrent > currentLimit)
                commPacketTx.axisData[getOpenIndex(arm, axis)].ffwCurrent = currentLimit;
            else if (ffwCurrent < -currentLimit)
                commPacketTx.axisData[getOpenIndex(arm, axis)].ffwCurrent = -currentLimit;
            else
                commPacketTx.axisData[getOpenIndex(arm, axis)].ffwCurrent = ffwCurrent;
            setFeedForwardCurrentDone[getOpenIndex(arm, axis)] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Set delta current of an axis.
/**
    This method sets delta current of the specified axis, checking if the given value added to the actual current value
    overcomes current limit of the specified axis: if so this method sets the feedforward current to the current limit of the specified axis.

    @param arm Number of the arm.
    @param axis Number of the axis.
    @param deltaCurrent Delta current of the specified axis.
    @return True if delta current has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setDeltaCurrent(int32_t arm, int32_t axis, float deltaCurrent)
{
    bool retValue = true;
    float actualCurrent = getTargetCurrent(arm, axis);
    float currentLimit = getCurrentLimit(arm, axis);

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            if (actualCurrent + deltaCurrent > currentLimit)
                commPacketTx.axisData[getOpenIndex(arm, axis)].ffwCurrent = currentLimit - actualCurrent;
            else if (actualCurrent + deltaCurrent < -currentLimit)
                commPacketTx.axisData[getOpenIndex(arm, axis)].ffwCurrent = -currentLimit - actualCurrent;
            else
                commPacketTx.axisData[getOpenIndex(arm, axis)].ffwCurrent = deltaCurrent;
            setDeltaCurrentDone[getOpenIndex(arm, axis)] = true;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get the dynamic model of an axis in ampere.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Dynamic model of the specified axis in ampere,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getDynamicModel(int32_t arm, int32_t axis)
{
    return getExtra1(arm, axis);
}

// Get the diagonal inertia of an axis in kg*m^2.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Diagonal inertia of the specified axis in kg*m^2,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getDiagonalInertia(int32_t arm, int32_t axis)
{
    return getExtra2(arm, axis);
}

// Get extra info 1 of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Extra info 1 of the specified axis,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getExtra1(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].extra1;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set extra info 1 of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param extra1 Extra info 1 of the specified axis.
    @return True if extra info 1 has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setExtra1(int32_t arm, int32_t axis, float extra1)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            // Add check on the extra1
            commPacketTx.axisData[getOpenIndex(arm, axis)].extra1 = extra1;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get extra info 2 of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Extra info 2 of the specified axis,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getExtra2(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].extra2;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set extra info 2 of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param extra2 Extra info 2 of the specified axis.
    @return True if extra info 2 has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setExtra2(int32_t arm, int32_t axis, float extra2)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            // Add check on the extra2
            commPacketTx.axisData[getOpenIndex(arm, axis)].extra2 = extra2;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Get extra info 3 of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @return Extra info 3 of the specified axis,
            NAN if an error has occurred or the specified arm and/or axis are not valid.
*/
float C4gOpen::getExtra3(int32_t arm, int32_t axis)
{
    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
            return commPacketRx.axisData[getOpenIndex(arm, axis)].extra3;
        else
            return NAN;
    }
    else
        return NAN;
}

// Set extra info 3 of an axis.
/**
    @param arm Number of the arm.
    @param axis Number of the axis.
    @param extra3 Extra info 3 of the specified axis.
    @return True if extra info 3 has been successfully set, false if the specified arm and/or axis are not valid.
*/
bool C4gOpen::setExtra3(int32_t arm, int32_t axis, float extra3)
{
    bool retValue = true;

    if (!errorOccurred())
    {
        if (checkAxis(arm, axis))
        {
            // Add check on the extra3
            commPacketTx.axisData[getOpenIndex(arm, axis)].extra3 = extra3;
        }
        else
            retValue = false;
    }
    else
        retValue = false;

    return retValue;
}

// Check following error threshold overcoming.
/**
    This method checks if the following error threshold of the specified axis has been overcome,
    i.e. the absolute value of the difference between target position and actual position
    of the specified axis overcomes the threshold.

    If the following error threshold has been overcome, #send() will set the #AxisDataTx::mode field to C4G_OPEN_FOLLOWING_ERROR.

    @param arm Number of the arm.
    @param axis Number of the axis.
    @param targetPosition Target position set for the specified axis.
    @return True if the following error threshold has not been overcome, false otherwise and
            #lastError is set to FOLLOWING_ERROR_OVERCOME.
*/
bool C4gOpen::checkFollowingError(int32_t arm, int32_t axis, float targetPosition)
{
    bool retValue = true;

    float actualPosition = getActualPosition(arm, axis);
    float followingErrorThreshold = getFollowingErrorThreshold(arm, axis);

    if (fabs(actualPosition - targetPosition) > followingErrorThreshold)
    {
        followingErrorOvercome[getOpenIndex(arm, axis)] = true;
        lastError = FOLLOWING_ERROR_OVERCOME;
        retValue = false;
    }

    return retValue;
}

// Check the validity of operations.
/**
    This method is called before sending a communication packet to C4G in order to check if all the operations
    expected by the actual open mode have been performed.

    For example, if the actual mode is C4G_OPEN_MODE_5, a \em setTargetPosition and a \em setTargetVelocity are expected,
    otherwise #lastError is appropriately set to SET_TARGET_POSITION_MISSING or SET_TARGET_VELOCITY_MISSING.
    If an unexpected operation has been performed, #lastError is set to OPERATION_NOT_ALLOWED.

    @return True if all the expected operations have been performed, false otherwise and #lastError is appropriately set.
*/
bool C4gOpen::checkOperations()
{
    bool retValue = true;

    for (int32_t i = 0; i < numberOfOpenAxes && retValue; i++)
    {
        switch (commPacketTx.axisData[i].mode)
        {
        case C4G_OPEN_MODE_0:
            if (setTargetPositionDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setTargetVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_0_DEBUG:
            // Set operations are automatically performed by send() method.
            break;

        case C4G_OPEN_MODE_1:
        case C4G_OPEN_MODE_1_STANDBY:
            if (!setTargetPositionDone[i])
            {
                lastError = SET_TARGET_POSITION_MISSING;
                retValue = false;
            }
            if (!setTargetVelocityDone[i])
            {
                lastError = SET_TARGET_VELOCITY_MISSING;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (!setFeedForwardVelocityDone[i])
            {
                lastError = SET_FFW_VELOCITY_MISSING;
                retValue = false;
            }
            if (!setFeedForwardCurrentDone[i])
            {
                lastError = SET_FFW_CURRENT_MISSING;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_2:
        case C4G_OPEN_MODE_2_STANDBY:
            if (setTargetPositionDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setTargetVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (!setFeedForwardVelocityDone[i])
            {
                lastError = SET_FFW_VELOCITY_MISSING;
                retValue = false;
            }
            if (!setFeedForwardCurrentDone[i])
            {
                lastError = SET_FFW_CURRENT_MISSING;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_4:
        case C4G_OPEN_MODE_4_STANDBY:
            if (!setTargetPositionDone[i])
            {
                lastError = SET_TARGET_POSITION_MISSING;
                retValue = false;
            }
            if (!setTargetVelocityDone[i])
            {
                lastError = SET_TARGET_VELOCITY_MISSING;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_5:
        case C4G_OPEN_MODE_5_STANDBY:
            if (!setTargetPositionDone[i])
            {
                lastError = SET_TARGET_POSITION_MISSING;
                retValue = false;
            }
            if (!setTargetVelocityDone[i])
            {
                lastError = SET_TARGET_VELOCITY_MISSING;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_7:
        case C4G_OPEN_MODE_7_STANDBY:
            if (!setTargetPositionDone[i])
            {
                lastError = SET_TARGET_POSITION_MISSING;
                retValue = false;
            }
            if (setTargetVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_8:
            if (setTargetPositionDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setTargetVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (!setFeedForwardVelocityDone[i])
            {
                lastError = SET_FFW_VELOCITY_MISSING;
                retValue = false;
            }
            if (setFeedForwardCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setDeltaCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            break;

        case C4G_OPEN_MODE_9:
            if (setTargetPositionDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setTargetVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setMeasureDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardVelocityDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (setFeedForwardCurrentDone[i])
            {
                lastError = OPERATION_NOT_ALLOWED;
                retValue = false;
            }
            if (!setDeltaCurrentDone[i])
            {
                lastError = SET_DELTA_CURRENT_MISSING;
                retValue = false;
            }
            break;

        default:
            break;
        }
    }

    return retValue;
}

/**
 * @brief Wait until all the open axes are set in mode 4
 * 
 * @param arm robotic arm index the wait function refers to
 * 
 * @return true when all the open mode axes are in mode 4, false otherwise
 * */

bool C4gOpen::waitForOpenMode4(int32_t arm)
{

    bool ready = false;
    int32_t numAxesMod4 = 0;
    int32_t numAxesinOpen = getNumberOfOpenAxes();

    while (!ready)
    {

        if (receive())
        {
            // Receive a comm packet from C4G

            for (int32_t i = 0; i < numAxesinOpen; i++)
            {

                // Check how many axes are in open mode 4

                if (commPacketRx.axisData[i].mode == C4G_OPEN_MODE_4)
                {
                    setTargetPosition(arm, i + 1, getActualPosition(arm, i + 1));
                    setTargetVelocity(arm, i + 1, 0);
                    numAxesMod4 = numAxesMod4 + 1;
                }
            }

            if (numAxesMod4 == numAxesinOpen)
            {
                ready = true;
            }
            else
            {
                numAxesMod4 = 0;
            }

            send();

        }
        else
        {
            ready = true;
            // Stop the communication socket
            stop();
        }
    }

    return ready;
}
