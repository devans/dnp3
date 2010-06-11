// 
// Licensed to Green Energy Corp (www.greenenergycorp.com) under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Green Enery Corp licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
// 
#include "MockAsyncAppLayer.h"

#include <boost/foreach.hpp>
#include <APL/Logger.h>
#include <APL/ToHex.h>


namespace apl { namespace dnp {

MockAsyncAppLayer::MockAsyncAppLayer(Logger* apLogger) :
Loggable(apLogger),
mNumCancel(0)
{
	
}

MockAsyncAppLayer::~MockAsyncAppLayer()
{
	
}

void MockAsyncAppLayer::SendResponse(APDU& arAPDU)
{
	LOG_BLOCK(LEV_COMM, "=> " << toHex(arAPDU.GetBuffer(), arAPDU.Size(), true));
	LOG_BLOCK(LEV_INTERPRET, "=> " << arAPDU.ToString());
	mFragments.push_back(arAPDU);
}

void MockAsyncAppLayer::SendUnsolicited(APDU& arAPDU)
{
	LOG_BLOCK(LEV_COMM, "=> " << toHex(arAPDU.GetBuffer(), arAPDU.Size(), true));
	LOG_BLOCK(LEV_INTERPRET, "=> " << arAPDU.ToString());
	mFragments.push_back(arAPDU);
}

void MockAsyncAppLayer::SendRequest(APDU& arAPDU)
{
	LOG_BLOCK(LEV_COMM, "=> " << toHex(arAPDU.GetBuffer(), arAPDU.Size(), true));
	LOG_BLOCK(LEV_INTERPRET, "=> " << arAPDU.ToString());
	mFragments.push_back(arAPDU);
}

APDU MockAsyncAppLayer::Read()
{
	if(mFragments.size() == 0) throw InvalidStateException(LOCATION, "no more fragments");
	APDU frag = mFragments.front();
	frag.Interpret();
	mFragments.pop_front();
	return frag;
}

APDU MockAsyncAppLayer::Peek()
{
	if(mFragments.size() == 0) throw InvalidStateException(LOCATION, "no more fragments");
	APDU frag = mFragments.front();
	frag.Interpret();
	return frag;
}

void MockAsyncAppLayer::Pop()
{
	if(mFragments.size() == 0) throw InvalidStateException(LOCATION, "no more fragments");
	mFragments.pop_front();	
}

FunctionCodes MockAsyncAppLayer::ReadFunction()
{
	if(mFragments.size() == 0) throw InvalidStateException(LOCATION, "No more fragments");
	else
	{
		FunctionCodes func = mFragments.front().GetFunction();
		mFragments.pop_front();
		return func;
	}
}

void MockAsyncAppLayer::CancelResponse()
{
	++mNumCancel;
}

}}
