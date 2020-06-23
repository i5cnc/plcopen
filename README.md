# PLCOpen Motion - Uranus

Uranus is a C++ library that implements the part 1 & 2 of PLCopen motion control standard.

# Currently included function blocks
Function Block            |   Description
------------------------- |   -----------------------------------
MC_Power | Controls the power stage (On or Off).
MC_Home | Commands the axis to perform the «search home» sequence.
MC_Stop | Commands a controlled motion stop and transfers the axis to the state ‘Stopping’.
MC_Halt | Commands a controlled motion stop and transfers the axis to the state ‘StandStill’.
MC_MoveAbsolute | Commands a controlled motion to a specified absolute position.
MC_MoveRelative | Commands a controlled motion of a specified distance relative to the set position at the time of the execution.
MC_MoveAdditive | Commands a controlled motion of a specified relative distance additional to the most recent commanded position.
MC_MoveVelocity | Commands a never ending controlled motion at a specified velocity.
MC_ReadStatus | Returns in detail the status of the state diagram of the selected axis.
MC_ReadMotionState | Returns in detail the status of the axis with respect to the motion currently in progress.
MC_ReadAxisError | Reads information concerning an axis, like modes, inputs directly related to the axis, and certain status information.
MC_EmergencyStop | Commands the axis to stop immediately and transfers the axis to the state ‘ErrorStop’.
MC_Reset | Makes the transition from the state ‘ErrorStop’ to ‘Standstill’ or ‘Disabled’ by resetting all internal axis-related errors.
MC_ReadActualPosition | Returns the actual position.
MC_ReadCommandPosition | Returns the command position.
MC_ReadActualVelocity | Returns the actual velocity.
MC_ReadCommandVelocity | Returns the command velocity.

# Build & install commands

    mkdir build
    cd build
    cmake ..
    make
    sudo make install
