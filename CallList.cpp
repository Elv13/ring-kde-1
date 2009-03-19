#include "CallList.h"

CallList::CallList()
{
	callIdCpt = 0;
	calls = new QVector<Call *>();
}


Call * CallList::operator[](const QListWidgetItem * item)
{
	for(int i = 0 ; i < size() ; i++)
	{
		if ((*calls)[i]->getItem() == item)
		{
			return (*calls)[i];
		}
	}
	return NULL;
}

Call * CallList::operator[](const QString & callId)
{
	for(int i = 0 ; i < size() ; i++)
	{
		if ((*calls)[i]->getCallId() == callId)
		{
			return (*calls)[i];
		}
	}
	return NULL;
}

Call * CallList::operator[](int ind)
{
	return (*calls)[ind];
}

QString CallList::getAndIncCallId()
{
	QString res = QString::number(callIdCpt++);
	
	return res;
}

int CallList::size()
{
	return calls->size();
}

QListWidgetItem * CallList::addDialingCall()
{
	Call * call = Call::buildDialingCall(getAndIncCallId());
	calls->append(call);
	return call->getItem();
}

QListWidgetItem * CallList::addIncomingCall(const QString & callId, const QString & from, const QString & account)
{
	Call * call = Call::buildIncomingCall(callId, from, account);
	calls->append(call);
	return call->getItem();
}