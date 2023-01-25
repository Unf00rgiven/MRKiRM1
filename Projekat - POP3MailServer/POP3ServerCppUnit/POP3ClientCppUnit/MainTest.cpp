#include "fsm.h"
#include "MainTest.h"
#include "const.h"

CPPUNIT_TEST_SUITE_REGISTRATION( MainTests );
uint8 * msg;
uint16 msgcode;


void MainTests::setUp()
{
    pSys = new FSMSystem(4, 4);

    POP3ServerFSM = new SrAuto();

   
    uint8 buffClassNo = 4;
    uint32 buffsCount[8] = { 100, 50, 50, 50 };
    uint32 buffsLength[8] = { 1025, 1025, 1025, 1025};

    pSys->InitKernel(buffClassNo, buffsCount, buffsLength, 1);

    lf = new LogFile("log.log", "log.ini");
    LogAutomateNew::SetLogInterface(lf);	

   
    pSys->Add(POP3ServerFSM, SR_AUTOMATE_TYPE_ID, 1, true);

}

void MainTests::tearDown()
{
}

void MainTests::UserTest()
{

}
