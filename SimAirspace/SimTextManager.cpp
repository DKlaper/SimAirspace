#include "SimTextManager.h"


#define TEXTSENT -99
#define IDLE -1 

std::list<Message*> messageQueue;
long long curTime = IDLE; // when was the current first element displayed
long long redispTime = IDLE; // when should it be displayed again because it was aborted
Message* currentInsideMessage = NULL; // we only want a single inside message;

SimTextManager::SimTextManager()
{
}

void SimTextManager::setHandle(HANDLE hsim) {
	hSimConnect = hsim;
}


SimTextManager::~SimTextManager()
{
}

void SimTextManager::reset() // reset everything to initial after simload
{
	for (Message* msg : messageQueue)
	{
		delete msg;
	}
	messageQueue.clear();
	curTime = IDLE;
	redispTime = IDLE;
	currentInsideMessage = NULL;
	currentSimConnectTextResult = SIMCONNECT_TEXT_RESULT_QUEUED;
}

Message* getMessageFromAirspace(AirspaceDef def, MessageType tp) {
	Message* newmsg = new Message();
	strcpy_s(newmsg->name, def.name);
	newmsg->type = def.type;
	if (tp == INSIDE)
	{
		newmsg->timeoutSecs = 0.0; // by default show as long as possible but on first other message disappear
	}
	else {
		newmsg->timeoutSecs = 10.0;
	}
	newmsg->textType = SIMCONNECT_TEXT_TYPE_PRINT_BLUE;
	newmsg->msgType = tp;
	return newmsg;
}

struct MessageCompare {
	Message* a;

	MessageCompare(Message *ap)
	{
		a = ap;
	}
	bool operator () (Message* b)
	{
		bool res= (b->type == a->type && strcmp(a->name, b->name) == 0);
		return res;
	}
};
// message compare that also deletes the values
// very sideeffecty
struct RemoverMessageCompare {
	Message* a;

	RemoverMessageCompare(Message *ap)
	{
		a = ap;
	}
	bool operator () (Message* b)
	{
		bool res = (b->type == a->type && strcmp(a->name, b->name) == 0);
		if (res)
		{
			if (b == currentInsideMessage)
			{
				currentInsideMessage = NULL;
			}
			delete b;
		}
		return res;
	}
};

bool addConfigToMessage(Message*  msg) { 
	if (msg->msgType == ENTERED)
	{
		msg->textType = SIMCONNECT_TEXT_TYPE_PRINT_GREEN;
	}
	else if (msg->msgType == LEFT)
	{
		msg->textType = SIMCONNECT_TEXT_TYPE_PRINT_RED;
	}
	return true; 
}

char* getText(Message* msg)
{
	char* text = new char[512];
	strcpy_s(text, 512, msg->name);
	return text;
}

void SimTextManager::displayNext()
{
	if (curTime != IDLE) // is something even displayed right now, else the first element is the new element
	{
		if (messageQueue.empty())
		{
			curTime = IDLE;
			redispTime = IDLE;
			return;
		}
		// delete first
		if (messageQueue.front() == currentInsideMessage)
		{
			currentInsideMessage = NULL;
		}
		delete messageQueue.front();
		messageQueue.pop_front();
		// since we removed the displayed one even if it was just displayed we are not displaying it anymore
		currentSimConnectTextResult = SIMCONNECT_TEXT_RESULT_QUEUED;
	}
	if (messageQueue.empty())
	{
		curTime = IDLE;
		redispTime = IDLE;
		return;
	}
	Message* newMsg = messageQueue.front();
	char* text = getText(newMsg);
	HRESULT res = SimConnect_Text(hSimConnect, newMsg->textType, newMsg->timeoutSecs, TEXTREQUESTID, 512, text);
	if (SUCCEEDED(res))
	{ // note this only works because it's single threaded so there's no race condition
		curTime = TEXTSENT;
		redispTime = TEXTSENT;
	}
	delete text;
}

void SimTextManager::redisplayCurrent()
{
	if (messageQueue.empty())
	{
		curTime = IDLE;
		redispTime = IDLE;
		return;
	}
	Message* msg = messageQueue.front();
	char* text = getText(msg);
	HRESULT res = SimConnect_Text(hSimConnect, msg->textType, msg->timeoutSecs, TEXTREQUESTID, 512, text);
	if (SUCCEEDED(res))
	{ // note this only works because it's single threaded so there's no race condition
		curTime = TEXTSENT;
		redispTime = TEXTSENT;
	}
	delete text;
}

void SimTextManager::updateView()
{
	long long systime = GetTickCount64();
	switch (currentSimConnectTextResult)
	{
	case SIMCONNECT_TEXT_RESULT_TIMEOUT:
		// if less then half redisplay.
		if (curTime > 0 && systime - curTime < messageQueue.front()->timeoutSecs * 1000 / 2)
		{
			redispTime = systime + 5000; // redisplay 5 seconds later
		}
		else { // we just display them when they would have been displayed
			// it could be that we never got the displayed message for it if there was no change.
			if (curTime == TEXTSENT)
			{
				// since it was removed it was definitely sent at some point
				curTime = IDLE;
			}
		}
		break;
	case SIMCONNECT_TEXT_RESULT_DISPLAYED:
		curTime = systime; // start counting against the timeout
		break;

	default: // do nothing 
		break;
	}
	if (redispTime < 0 && ( curTime == IDLE || !messageQueue.empty() && (curTime > 0 && systime - curTime > messageQueue.front()->timeoutSecs * 1000)))
	{
		displayNext();
		return;
	}
	else if (redispTime > 0 && redispTime < systime) // time to redisplay
	{
		redisplayCurrent();
	}
	currentSimConnectTextResult = SIMCONNECT_TEXT_RESULT_QUEUED;
}

void SimTextManager::process(std::set<AirspaceDef, airspaceCompare>* currentAirspaces, std::vector<AirspaceDef>* enteredAsp, std::vector<AirspaceDef>* leftAsp)
{
	// first remove all left and add new left also consider case where left is currently displayed
	for (AirspaceDef left : *leftAsp)
	{
		Message* curLeft = getMessageFromAirspace(left, LEFT);
		MessageCompare cmp = MessageCompare(curLeft);
		while (!messageQueue.empty() && cmp(messageQueue.front()))
		{
			displayNext();
		}

		// removes 
		messageQueue.remove_if(RemoverMessageCompare(curLeft));

		if (addConfigToMessage(curLeft))
		{
			messageQueue.push_back(curLeft);
		}
		else {
			delete curLeft;
		}
	}
	// then display all the ones we entered
	for (AirspaceDef entered : *enteredAsp)
	{
		// make message
		Message* curEnter = getMessageFromAirspace(entered, ENTERED);

		if (addConfigToMessage(curEnter))
		{
			messageQueue.push_back(curEnter);
		}
		else {
			delete curEnter;
		}
	}

	// inside is special we only want the one MOST IMPORTANT inside message to prevail and only have one inside message in the queue at any time
	// the importance is taken from hardcoded importance values that already correspond to the set order!

	// step 1 find current best airspace
	Message* newBest = NULL;
	for (AirspaceDef inside : *currentAirspaces) // this is already sorted according to importance
	{
		Message* curInside = getMessageFromAirspace(inside, INSIDE);
		if (addConfigToMessage(curInside))
		{
			newBest = curInside;
			break; // first match wins
		}
		else {
			delete curInside;
		}
	}

	// step 2 remove current inside message if it's necessary // otherwise just leave it
	if (currentInsideMessage != NULL && (newBest == NULL || (newBest->type == currentInsideMessage->type && strcmp(newBest->name, currentInsideMessage->name) == 0) ) )
	{
		if (currentInsideMessage == messageQueue.front())
		{
			displayNext();// autohandles it
		}
		else {
			delete currentInsideMessage;
			messageQueue.remove(currentInsideMessage);
		}

		// step 2b add the new one if the old one was removed
		if (newBest != NULL) {
			currentInsideMessage = newBest;
			messageQueue.push_back(currentInsideMessage);
		}
	}
	else if (currentInsideMessage == NULL && newBest != NULL) // special case we don't have a current but a new one so we want to display that
	{
		currentInsideMessage = newBest;
		messageQueue.push_back(currentInsideMessage);
	}
	else if (newBest != NULL)
	{
		// prevent memory leak when msg not used
		delete newBest;
	}


	updateView();
}
