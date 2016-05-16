// PianoMidi.cpp: implementation of the CPianoMidi class.
//
//////////////////////////////////////////////////////////////////////

#include "PianoMidi.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPianoMidi::CPianoMidi()
{
	create();
}

CPianoMidi::~CPianoMidi()
{
	destroy();
}

bool CPianoMidi::create()
{

	Pm_CountDevices();

    int i = 1;
    TIME_START;

  
    Pm_OpenInput(&midi, 
                 i,
                 DRIVER_INFO, 
                 INPUT_BUFFER_SIZE, 
                 TIME_PROC, 
                 TIME_INFO);

    Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK);

    while (Pm_Poll(midi))
        Pm_Read(midi, buffer, 1);

	return true;
}

bool CPianoMidi::destroy()
{
	Pm_Close(midi);
	return true;
//  CLIST
//	 CARRAY
}


//MIDI_EVENT CPianoMidi::getOneEvent()
//{

//}

int CPianoMidi::getAllEvents()
{
    /* now start paying attention to messages */
    //i = 0; /* count messages as they arrive */
	numNotes = 0;
    while (1) {
        status = Pm_Poll(midi);
        if (status == TRUE) {
            length = Pm_Read(midi,buffer, 1);
            if (length > 0) {
                //printf("Got message %d: time %ld, %2lx %2lx %2lx\n",
                  //     i,
                    //   buffer[0].timestamp,
                      // Pm_MessageStatus(buffer[0].message),
                       //Pm_MessageData1(buffer[0].message),
                       //Pm_MessageData2(buffer[0].message));
				
				noteArray[numNotes] = Pm_MessageData1(buffer[0].message);
	            //i++;
				numNotes++;
				if (numNotes == 20)
					return 0;
            } 
			else 
                return 0;
            
        }
		else
			return 0;
        
    }
}

void CPianoMidi::clearAllEvents()
{
	/* now start paying attention to messages */
    //i = 0; /* count messages as they arrive */
	numNotes = 0;
    while (1) {
        status = Pm_Poll(midi);
        if (status == TRUE) {
            length = Pm_Read(midi,buffer, 1);
            if (length > 0) {
            } 
			else 
                return;
        }
		else
			return;
        
    }
}
