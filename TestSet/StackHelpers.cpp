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
#include "StackHelpers.h"

#include <DNP3/SlaveStackConfig.h>
#include <DNP3/MasterStackConfig.h>
#include <APL/IPhysicalLayerAsync.h>

#include <APLXML/PhysicalLayerXMLFactory.h>
#include <DNP3XML/XmlToConfig.h>
#include <APLXML/XMLConversion.h>

#include <Terminal/PhysicalLayerIOStreamAsync.h>

using namespace APLXML_Base;

namespace apl { namespace dnp {

IPhysicalLayerAsync* FGetTerminalPhys(Logger* apLogger, boost::asio::io_service* apSrv, bool aRemote, apl::uint_16_t aRemotePort)
{
	if (aRemote) return PhysicalLayerFactory::FGetTCPServerAsync(aRemotePort, apSrv, apLogger);
	else return new PhysicalLayerIOStreamAsync(apLogger, apSrv);
}

StackBase::StackBase(const APLXML_Base::PhysicalLayerList_t& arList, FilterLevel aLevel, const std::string& arLogFile, bool aRemote, apl::uint_16_t aRemotePort) :
	log(),
	logToFile(&log, arLogFile),
	pTermLogger(log.GetLogger(aLevel, "terminal")),	
	mService(),
	mTermThread(mService.Get()),
	mTimerSrc(mService.Get()),
	pTermPhys(FGetTerminalPhys(pTermLogger, mTermThread.GetService(), aRemote, aRemotePort)),
	lte(&log),	    
	trm(pTermLogger, pTermPhys.get(), &mTimerSrc, "Test Set Terminal (DNP)", true),
	mgr(log.GetLogger(aLevel, "mgr"))
{
	trm.AddExtension(&lte);			
	XmlToConfig::Configure(arList, aLevel, mgr);
}

void StackBase::Run() {
	mgr.Start();
	trm.Init();
	mTermThread.Run(); //blocking

	mTermThread.Stop();
}


SlaveXMLStack::SlaveXMLStack(APLXML_STS::SlaveTestSet_t* pCfg, FilterLevel aLevel) : 
	StackBase(pCfg->PhysicalLayerList, aLevel, pCfg->LogFile, pCfg->Remote, pCfg->RemotePort),
	pObs(mgr.AddSlave(pCfg->PhysicalLayer, "sts_stack", aLevel, crte.GetCmdAcceptor(), XmlToConfig::GetSlaveConfig(pCfg->Slave, pCfg->DeviceTemplate))),	
	crte(log.GetLogger(LEV_INTERPRET, "Incoming Controls"), pCfg->LinkCommandStatus, pObs),
	dote(pObs)
{
	trm.AddExtension(&dote);
	trm.AddExtension(&crte);
}


MasterXMLStack::MasterXMLStack(APLXML_MTS::MasterTestSet_t* pCfg, FilterLevel aLevel) :
StackBase(pCfg->PhysicalLayerList, aLevel, pCfg->LogFile),
fdo(),
accept(mgr.AddMaster(pCfg->PhysicalLayer, "mts_stack", aLevel, &fdo, XmlToConfig::GetMasterConfig(pCfg->Master))),
cte(accept),
fte(&fdo)
{
	trm.AddExtension(&fte);
	trm.AddExtension(&cte);
}
	
	

}}

