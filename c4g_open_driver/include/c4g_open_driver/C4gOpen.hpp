/*
    C4gOpen.hpp

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
    @file C4gOpen.hpp
    @brief Structs and class to allow a PC to communicate with Comau C4Gopen.
    
    This file contains declaration of:<br>
    <ul>
    <li>InitPacket, a struct that represents the inizialization packet;</li>
    <li>Header, a struct that represents the header of a communication packet;</li>
    <li>AxisDataRx, a struct that represents the axis data fields of an incoming communication packet;</li>
    <li>AxisDataTx, a struct that represents the axis data fields of an outgoing communication packet;</li>
    <li>CommPacketRx, a struct that represents an incoming communication packet;</li>
    <li>CommPacketTx, a struct that represents an outgoing communication packet;</li>
    <li>C4gOpen, a class that contains members and methods to allow a PC to communicate with Comau C4Gopen.</li>
    </ul>
*/

#ifndef _C4GOPEN_HPP_
#define _C4GOPEN_HPP_

#include <sys/mman.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <c4g_open_driver/C4gOpenConstants.hpp>
#include <pthread.h>
#include <sched.h>

#define MAX_SIZE_OF_PACKETS   1500           //< Maximum size of a packet exchanged between C4G and PC.
#define WAIT_INIT_PCK_TIMEOUT 300000000000LL //< Socket timeout while waiting for the arrival of initialization packet.
#define COMMUNICATION_TIMEOUT 1000000000LL   //< Socket timeout during normal communication between C4G and PC.

// Structure representing the initialization packet.
/**
    This packet is sent by C4G to PC before beginning the actual communication, after the synchronization phase.
    It contains C4Gopen System informations and robot arms data.
*/
struct InitPacket // Size = 1200 bytes
{
    int32_t seqNumber;                                   //< Sequence number of the initialization packet.
    int32_t numberOfFieldsPerPacket;                     //< Number of header fields of a communication packet.
    int32_t numberOfFieldsPerAxis;                       //< Number of axis data fields of a communication packet.
    int32_t numberOfAxesInOpenMode;                      //< Number of 'open mode' axes.
    int32_t arm1OpenMode[MAX_NUM_AXES_PER_ARM];          //< Array of axes mode of Arm 1.
    int32_t arm2OpenMode[MAX_NUM_AXES_PER_ARM];          //< Array of axes mode of Arm 2.
    int32_t arm3OpenMode[MAX_NUM_AXES_PER_ARM];          //< Array of axes mode of Arm 3.
    int32_t arm4OpenMode[MAX_NUM_AXES_PER_ARM];          //< Array of axes mode of Arm 4.
    int32_t typeOfHeaderField1;                          //< Type of 1st header field of a communication packet.
    int32_t typeOfHeaderField2;                          //< Type of 2nd header field of a communication packet.
    int32_t typeOfHeaderField3;                          //< Type of 3rd header field of a communication packet.
    int32_t typeOfAxisDataField1;                        //< Type of 1st axis data field of a communication packet.
    int32_t typeOfAxisDataField2;                        //< Type of 2nd axis data field of a communication packet.
    int32_t typeOfAxisDataField3;                        //< Type of 3rd axis data field of a communication packet.
    int32_t typeOfAxisDataField4;                        //< Type of 4th axis data field of a communication packet.
    int32_t typeOfAxisDataField5;                        //< Type of 5th axis data field of a communication packet.
    int32_t typeOfAxisDataField6;                        //< Type of 6th axis data field of a communication packet.
    int32_t typeOfAxisDataField7;                        //< Type of 7th axis data field of a communication packet.
    int32_t typeOfAxisDataField8;                        //< Type of 8th axis data field of a communication packet.
    int32_t typeOfAxisDataField9;                        //< Type of 9th axis data field of a communication packet.
    int32_t sampleTime;                                  //< Communication cycle time [ms].
    int32_t arm1OpenAxesMap[MAX_NUM_AXES_PER_ARM];       //< Open/Logical axes map of Arm 1.
    int32_t arm2OpenAxesMap[MAX_NUM_AXES_PER_ARM];       //< Open/Logical axes map of Arm 2.
    int32_t arm3OpenAxesMap[MAX_NUM_AXES_PER_ARM];       //< Open/Logical axes map of Arm 3.
    int32_t arm4OpenAxesMap[MAX_NUM_AXES_PER_ARM];       //< Open/Logical axes map of Arm 4.
    float arm1CalibrationConstant[MAX_NUM_AXES_PER_ARM]; //< Calibration constants of axes of Arm 1 [gear rotations].
    float arm2CalibrationConstant[MAX_NUM_AXES_PER_ARM]; //< Calibration constants of axes of Arm 2 [gear rotations].
    float arm3CalibrationConstant[MAX_NUM_AXES_PER_ARM]; //< Calibration constants of axes of Arm 3 [gear rotations].
    float arm4CalibrationConstant[MAX_NUM_AXES_PER_ARM]; //< Calibration constants of axes of Arm 4 [gear rotations].
    float arm1CurrentLimit[MAX_NUM_AXES_PER_ARM];        //< Current limits of axes of Arm 1 [A].
    float arm2CurrentLimit[MAX_NUM_AXES_PER_ARM];        //< Current limits of axes of Arm 2 [A].
    float arm3CurrentLimit[MAX_NUM_AXES_PER_ARM];        //< Current limits of axes of Arm 3 [A].
    float arm4CurrentLimit[MAX_NUM_AXES_PER_ARM];        //< Current limits of axes of Arm 4 [A].
    int32_t majorNumber;                                 //< C4G system software major number.
    int32_t minorNumber;                                 //< C4G system software minor number.
    int32_t buildNumber;                                 //< C4G system software build number.
    float arm1KinInflCoeff[MAX_NUM_AXES_PER_ARM];        //< Kinematic Influence Coefficients of axes of Arm 1.
    float arm2KinInflCoeff[MAX_NUM_AXES_PER_ARM];        //< Kinematic influence coefficients of axes of Arm 2.
    float arm3KinInflCoeff[MAX_NUM_AXES_PER_ARM];        //< Kinematic influence coefficients of axes of Arm 3.
    float arm4KinInflCoeff[MAX_NUM_AXES_PER_ARM];        //< Kinematic influence coefficients of axes of Arm 4.
    float arm1TxRate[MAX_NUM_AXES_PER_ARM];              //< Transmission rates of axes of Arm 1.
    float arm2TxRate[MAX_NUM_AXES_PER_ARM];              //< Transmission rates of axes of Arm 2.
    float arm3TxRate[MAX_NUM_AXES_PER_ARM];              //< Transmission rates of axes of Arm 3.
    float arm4TxRate[MAX_NUM_AXES_PER_ARM];              //< Transmission rates of axes of Arm 4.
    float arm1FollowingError[MAX_NUM_AXES_PER_ARM];      //< Following error thresholds of axes of Arm 1 [gear rotations].
    float arm2FollowingError[MAX_NUM_AXES_PER_ARM];      //< Following error thresholds of axes of Arm 2 [gear rotations].
    float arm3FollowingError[MAX_NUM_AXES_PER_ARM];      //< Following error thresholds of axes of Arm 3 [gear rotations].
    float arm4FollowingError[MAX_NUM_AXES_PER_ARM];      //< Following error thresholds of axes of Arm 4 [gear rotations].
};

// Structure representing the header of a communication packet.
struct Header
{
    int32_t seqNumber;     //< Sequence number of the communication packet.
    int32_t status;        //< Arm status (#DRIVE_ON or #EXIT_FROM_OPEN).
    int32_t functionality; //< Open mode functionality exchanged between C4G and PC.
};

// Structure representing the axis data fields of an incoming communication packet.
/**
    Each field is always present, but its meaning and value depend on the specific open mode.
*/
struct AxisDataRx
{
    int32_t mode;         //< Axis mode.
    float targetPosition; //< Target position computed by C4G.
                          /**< The value of this field is significant depending on the specific open mode. */
    float targetVelocity; //< Target velocity computed by C4G.
                          /**< The value of this field is significant depending on the specific open mode. */
    float actualPosition; //< Position given by the encoder.
                          /**< The value of this field is always significant, apart from the specific open mode. */
    float actualVelocity; //< Velocity given by the encoder.
                          /**< The value of this field is always significant, apart from the specific open mode. */
    float targetCurrent;  //< Target current computed by C4G.
                          /**< The value of this field is significant depending on the specific open mode. */
    float extra1;         //< First extra information.
    float extra2;         //< Second extra information.
    float extra3;         //< Third extra information.
};

// Structure representing the axis data fields of an outgoing communication packet.
/**
    Each field is always present, but its meaning and value depend on the specific open mode.
*/
struct AxisDataTx
{
    int32_t mode;         //< Axis mode.
    float targetPosition; //< Target position computed by PC.
                          /**< The value of this field is significant depending on the specific open mode. */
    float targetVelocity; //< Target velocity computed by PC.
                          /**< The value of this field is significant depending on the specific open mode. */
    float measure;        //< Measure given from an external sensor.
                          /**< The value of this field is significant depending on the specific open mode. */
    float ffwVelocity;    //< Feedforward velocity computed by PC.
                          /**< The value of this field is significant depending on the specific open mode. */
    float ffwCurrent;     //< Feedforward current computed by PC.
                          /**< The value of this field is significant depending on the specific open mode. */
    float extra1;         //< First extra information.
    float extra2;         //< Second extra information.
    float extra3;         //< Third extra information.
};

// Structure representing an incoming communication packet.
/**
    An incoming communication has always the header field and
    as many axis data fields as the number of axes in open mode.
*/
struct CommPacketRx
{
    Header header;                          //< Header of the incoming communication packet.
    AxisDataRx axisData[MAX_NUM_OPEN_AXES]; //< Data fields of the incoming communication packet.
};

// Structure representing an outgoing communication packet.
/**
    An outgoing communication has always the header field and
    as many axis data fields as the number of axes in open mode.
*/
struct CommPacketTx
{
    Header header;                          //< Header of the outgoing communication packet.
    AxisDataTx axisData[MAX_NUM_OPEN_AXES]; //< Data fields of the outgoing communication packet.
};

// Class containing members and methods to allow a PC to communicate with Comau C4Gopen.
class C4gOpen
{
    private:
        int32_t socketID;             //< Socket identifier.
        sockaddr_in socketStruct;     //< Server IPv4 structure.
        sockaddr_in c4gSocketStruct;  //< Client IPv4 structure.
        uint32_t c4gSocketLength;     //< Size of the buffer associated with client socket.
        int32_t portNumber;           //< C4G-PC UDP socket port number.
        int64_t communicationTimeout; //< Socket timeout during normal communication between C4G and PC.

        void *startUpPacket;          //< Pointer to a packet exchanged during the initialization/synchronization phase.

        int32_t logicalToOpenMap[MAX_NUM_ARMS * MAX_NUM_AXES_PER_ARM]; //< Open/Logical axes map.
                                                                       /**< This array is filled when #mapIndices() method is called. */

        InitPacket initPacket;     //< Instance of an initialization packet.
        CommPacketRx commPacketRx; //< Instance of an incoming communication packet.
        CommPacketTx commPacketTx; //< Instance of an outgoing communication packet.

        int32_t numberOfOpenAxes;  //< Number of axes in open mode.
                                   /**< This value is read from the #InitPacket::numberOfAxesInOpenMode field of the initialization packet. */
        int32_t sampleTime;        //< Communication cycle time.
                                   /**< This value is read from #InitPacket::sampleTime field of the initialization packet. */

        ErrorCodes lastError;      //< Code of the last occurred error.

        bool followingErrorOvercome[MAX_NUM_OPEN_AXES];     //< Array of flags indicating if a following error has occurred for a certain axis.
        bool setTargetPositionDone[MAX_NUM_OPEN_AXES];      //< Array of flags indicating if target position has been set for a certain axis.
        bool setTargetVelocityDone[MAX_NUM_OPEN_AXES];      //< Array of flags indicating if target velocity has been set for a certain axis.
        bool setMeasureDone[MAX_NUM_OPEN_AXES];             //< Array of flags indicating if measure from an external sensor has been set for a certain axis.
        bool setFeedForwardVelocityDone[MAX_NUM_OPEN_AXES]; //< Array of flags indicating if feedforward velocity has been set for a certain axis.
        bool setFeedForwardCurrentDone[MAX_NUM_OPEN_AXES];  //< Array of flags indicating if feedforward current has been set for a certain axis.
        bool setDeltaCurrentDone[MAX_NUM_OPEN_AXES];        //< Array of flags indicating if delta current has been set for a certain axis.
        bool setModeDone[MAX_NUM_ARMS];                     //< Array of flags indicating if an explicit mode setting has been done for a certain arm.
        bool exitFromOpenDone[MAX_NUM_ARMS];                //< Array of flags indicating if an exit from open mode has been requested for a certain arm.
        bool canChangeMode[MAX_NUM_ARMS];                   //< Array of flags indicating if the open mode of a certain arm can be changed.
                                                            /**< The open mode of an arm can be changed only after having performed #exitFromOpen().*/

        void resetInitPacket();
        void resetCommPacketRx();
        void resetCommPacketTx();
        void resetFlags();

        bool initSocket();
        bool waitInitPacket();
        bool readInitPacket();
        void closeSocket();

        int32_t receivePacket(void *packet, int32_t bytesToBeReceived);
        int32_t sendPacket(void *packet, int32_t bytesToSend);

        void mapIndices(int32_t arm, int32_t armOpenAxesMap[MAX_NUM_AXES_PER_ARM]);
        int32_t getOpenIndex(int32_t arm, int32_t axis);

        bool checkArm(int32_t arm);
        bool checkAxis(int32_t arm, int32_t axis);
        bool checkMode(int32_t mode);
        bool checkFollowingError(int32_t arm, int32_t axis, float targetPosition);
        bool checkOperations();

    public:
        C4gOpen(int32_t port = DEFAULT_PORT_NUMBER);
        ~C4gOpen();

        bool start();
        bool receive();
        bool send();
        void stop();

        int64_t getCommunicationTimeout();
        void setCommunicationTimeout(int64_t timeout);

        bool errorOccurred();
        ErrorCodes getLastError();
        void resetError();

        int32_t getNumberOfOpenAxes();
        int32_t getSampleTime();
        float getCalibrationConstant(int32_t arm, int32_t axis);
        float getCurrentLimit(int32_t arm, int32_t axis);
        float getTxRate(int32_t arm, int32_t axis);
        float getKinInflCoeff54(int32_t arm);
        float getKinInflCoeff64(int32_t arm);
        float getKinInflCoeff65(int32_t arm);
        float getFollowingErrorThreshold(int32_t arm, int32_t axis);

        bool isInDriveOn(int32_t arm);
        int32_t getFunctionality(int32_t arm);
        int32_t getMode(int32_t arm);

        float getTargetPosition(int32_t arm, int32_t axis);
        float getTargetVelocity(int32_t arm, int32_t axis);
        float getActualPosition(int32_t arm, int32_t axis);
        float getActualVelocity(int32_t arm, int32_t axis);
        float getTargetCurrent(int32_t arm, int32_t axis);
        float getDynamicModel(int32_t arm, int32_t axis);
        float getDiagonalInertia(int32_t arm, int32_t axis);
        float getExtra1(int32_t arm, int32_t axis);
        float getExtra2(int32_t arm, int32_t axis);
        float getExtra3(int32_t arm, int32_t axis);

        bool exitFromOpen(int32_t arm);
        bool setMode(int32_t arm, int32_t mode);

        bool setTargetPosition(int32_t arm, int32_t axis, float targetPosition);
        bool setTargetVelocity(int32_t arm, int32_t axis, float targetVelocity);
        bool setMeasure(int32_t arm, int32_t axis, float measure);
        bool setFeedForwardVelocity(int32_t arm, int32_t axis, float ffwVelocity);
        bool setFeedForwardCurrent(int32_t arm, int32_t axis, float ffwCurrent);
        bool setDeltaCurrent(int32_t arm, int32_t axis, float deltaCurrent);
        bool setExtra1(int32_t arm, int32_t axis, float extra1);
        bool setExtra2(int32_t arm, int32_t axis, float extra2);
        bool setExtra3(int32_t arm, int32_t axis, float extra3);

        bool waitForOpenMode4(int32_t arm);
};

#endif // _C4GOPEN_HPP_
