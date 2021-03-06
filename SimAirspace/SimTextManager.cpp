#include "SimTextManager.h"


#define TEXTSENT -99
#define IDLE -1 

std::list<Message*> messageQueue;
long long curTime = IDLE; // when was the current first element displayed
long long redispTime = IDLE; // when should it be displayed again because it was aborted
Message* currentInsideMessage = NULL; // we only want a single inside message;
const size_t TEXTSIZE = 512;

/*Because c is ugly*/

char *str_replace(char *orig, char *rep, char *with) {
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	int len_rep;  // length of rep
	int len_with; // length of with
	int len_front; // distance between rep and end of last rep
	int count;    // number of replacements

	if (!orig)
		return NULL;
	if (!rep)
		rep = "";
	len_rep = strlen(rep);
	if (!with)
		with = "";
	len_with = strlen(with);

	ins = orig;
	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	tmp = result = new char[strlen(orig) + (len_with - len_rep) * count + 1];

	if (!result)
		return NULL;

	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

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

void fixTFRName(AirspaceDef& def)
{ // it can always only be one but we need to check all
	// ugly copying should have done str str map but that also looks ugly
	// also note this null handling only fails if the new call failed and then we have other problems
	char *nextRes, *prevRes;
	nextRes = str_replace(def.name, "Section_91_143", "SPACE OPERATIONS TFR");
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_91_141", "VIP TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_91_137_a_1", "HAZARDS PROTECTION TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_91_145", "AIR SHOW / SPORTS TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_91_137_a_2", "HAZARDS RELIEF TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_91_137_a_3", "HAZARDS CONGESTION TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_91_138", "HAWAII DISASTER TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;
	nextRes = str_replace(prevRes, "Section_99_7", "NATIONAL SECURITY TFR");
	if (prevRes != NULL)
	{
		// not used anymore
		delete prevRes;
	}
	prevRes = nextRes;

	if (nextRes != NULL)
	{
		strcpy_s(def.name, 150, nextRes);
		delete nextRes;
	}
}

Message* getMessageFromAirspace(AirspaceDef def, MessageType tp) {
	Message* newmsg = new Message();
	if (def.type == TFR)
	{ // if TFR we need to fix the name now
		fixTFRName(def);
	}
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
	msg->timeoutSecs = AirspaceDef::conf.standardTimeout;
	if (airspaceImportance[msg->type] < AirspaceDef::conf.displayImportance)
	{
		return false;
	}
	if (msg->msgType == ENTERED)
	{
		msg->textType = SIMCONNECT_TEXT_TYPE_PRINT_GREEN;
	}
	else if (msg->msgType == LEFT)
	{
		msg->textType = SIMCONNECT_TEXT_TYPE_PRINT_YELLOW;
	}
	else {
		msg->textType = SIMCONNECT_TEXT_TYPE_PRINT_BLUE;
		if (airspaceImportance[msg->type] > AirspaceDef::conf.insideImportance)
		{
			msg->timeoutSecs = 0.0; // inside always overwrite timeout
		}
		else {
			return false; // do not display inside message
		}
 }
	if (airspaceImportance[msg->type] > AirspaceDef::conf.forbiddenImportance)
	{
		msg->textType = SIMCONNECT_TEXT_TYPE_PRINT_RED;
	}
	return true; 
}


char* getText(Message* msg)
{
	char* text = new char[TEXTSIZE];
	switch (msg->msgType)
	{
		case ENTERED:
			sprintf(text, "You have just entered %s. This is %s airspace. Please comply with regulatory requirements.", msg->name, airspaceTypes[msg->type].c_str());
			break;
		case INSIDE:
			if (airspaceImportance[msg->type] > AirspaceDef::conf.forbiddenImportance)
			{
				sprintf(text, "You are inside %s. This is %s airspace. Leave this airspace immediately! Flying is forbidden here!", msg->name, airspaceTypes[msg->type].c_str());
			}
			else {
				sprintf(text, "You are inside %s. This is %s airspace. Please be considerate, observe restrictions and minimum altitudes in this airspace.", msg->name, airspaceTypes[msg->type].c_str());
			}
			break;
		case LEFT:
			sprintf(text, "You just left %s. This is %s airspace. Report leaving if necessary.", msg->name, airspaceTypes[msg->type].c_str());
			break;
		default:
			break;
	}
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
	HRESULT res = SimConnect_Text(hSimConnect, newMsg->textType, newMsg->timeoutSecs, TEXTREQUESTID, TEXTSIZE, text);
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
	HRESULT res = SimConnect_Text(hSimConnect, msg->textType, msg->timeoutSecs, TEXTREQUESTID, TEXTSIZE, text);
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
