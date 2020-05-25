//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "ExampleService.h"
#include "artery/traci/VehicleController.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <inet/common/ModuleAccess.h>
#include <inet/common/geometry/common/CanvasProjection.h>
#include <inet/features.h>

#include "artery/application/DenService.h"
// PHILIPPOS
#include "artery/application/ProofObject.h"
#include "artery/application/ProofObject.cc"


#include <vector>

using namespace omnetpp;
using namespace vanetza;

namespace artery
{

static const simsignal_t scSignalCamReceived = cComponent::registerSignal("CamReceived");

Define_Module(ExampleService)

ExampleService::ExampleService()
{
}

ExampleService::~ExampleService()
{
	cancelAndDelete(m_self_msg);
}


// Executed on reception of a message
void ExampleService::indicate(const btp::DataIndication& ind, cPacket* packet, const NetworkInterface& net)
{
	Enter_Method("indicate");
	auto& ldm = getFacilities().get_const<LocalDynamicMap>();

    static const vanetza::ItsAid example_its_aid = 16480;

    auto& mco = getFacilities().get_const<MultiChannelPolicy>();
    auto& networks = getFacilities().get_const<NetworkInterfaceTable>();

    //Vehicle ID from VehicleDataProvider
    const auto egoStationID = getFacilities().get_const<VehicleDataProvider>().station_id();

    auto vehicle_ID = std::to_string(egoStationID);

    // Request message was received
    if (strcmp(packet->getName(), "REQUEST") == 0) {

        std::vector<std::string> endorsed_neighbors;
        std::string str(packet->_getEncapMsg()->_getEncapMsg()->getName());

        // Check necessary because boost::split adds "something" to endorsed_neighbor even when str is empty
        // So the for loop in the next if clause is executed always at least one time.
        if(!str.empty()){
            boost::split(endorsed_neighbors, str, boost::is_any_of(","));
        }

        // Receiver in the endorsed list or not
        if (std::find(endorsed_neighbors.begin(), endorsed_neighbors.end(), vehicle_ID) != endorsed_neighbors.end())
        {
            uint32_t current_StationID = std::stoi(packet->_getEncapMsg()->getName());
            std::vector<uint32_t> neighbor_list;

            // Lambda function to pass to the LDM
            LocalDynamicMap::CamPredicate cam_exist = [&](const LocalDynamicMap::Cam& cam) {
                bool result = false;
                if(cam->header.stationID == current_StationID){
                    result = true;
                }
                return result;
            };

            // Requester in the receiver's LDM or not
            if(ldm.count(cam_exist) >= 1){

                cPacket* packet3 = new cPacket(std::to_string(current_StationID).c_str());

                // Endorse the remaining neighbors that the receivers also "sees"
                for (auto const& value: endorsed_neighbors)
                {
                    std::stringstream id(value);
                    id >> current_StationID;
                    if(ldm.count(cam_exist) >= 1){
                        // Endorsing the found neighbors from the the location proof request
                        neighbor_list.push_back(current_StationID);
                    }
                }

                // Find a channel to transmit the response message
                for (auto channel : mco.allChannels(example_its_aid)) {
                    auto network = networks.select(channel);
                    if (network) {
                        btp::DataRequestB req;
                        // use same port number as configured for listening on this channel
                        req.destination_port = host_cast(getPortNumber(channel));
                        req.gn.transport_type = geonet::TransportType::SHB;
                        req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP3));
                        req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;
                        req.gn.its_aid = example_its_aid;

                        cPacket* packet = new cPacket("RESPONSE");
                        cPacket* packet2 = new cPacket(vehicle_ID.c_str());

                        std::stringstream neighbor_list_string;

                        if(!neighbor_list.empty()){
                            std::copy(neighbor_list.begin(), neighbor_list.end()-1, std::ostream_iterator<unsigned long>(neighbor_list_string, ","));
                            neighbor_list_string << neighbor_list.back();
                        }

                        cPacket* packet4 = new cPacket(neighbor_list_string.str().c_str());

                        packet3->encapsulate(packet4);
                        packet2->encapsulate(packet3);
                        packet->encapsulate(packet2);

                        request(req, packet, network.get());
                        loc_proof_send++;
                    }

                    else
                    {
                        EV_ERROR << "No network interface available for channel " << channel << "\n";
                    }
                }
            }
        }
    }

    // Only accept responses when the timer (proof_req) has not yet run out
    if (strcmp(packet->getName(), "RESPONSE") == 0 && proof_req == true) {
        if(strcmp(packet->_getEncapMsg()->_getEncapMsg()->getName(), vehicle_ID.c_str()) == 0){
            response_list.erase(std::remove(response_list.begin(), response_list.end(), std::stoi(packet->_getEncapMsg()->getName())), response_list.end());

            if(response_list.empty()){
                EV_INFO << "VEHICLE: " << vehicle_ID << " MANAGED TO GENERATE A FULL LOCATION PROOF AT: " << omnetpp::simTime() << " .\n";
                proof_req = false;
                full_proofs++;
                proof_size.record(endorsed_list.size() - response_list.size());
                proof_requested.record(endorsed_list.size());
            }
        }
    }

    delete(packet);
}

void ExampleService::initialize()
{
    mDecisionLogic = par("DecisionLogic");
    mNeighboringWindow = par("NeighboringWindow");

    proof_req = false;
    loc_proof_send = 0;

    // Add a random delay to the generation of location proofs so that not all vehicles request proofs at the same time
    omnetpp::SimTime random_init_delay { uniform(0,60), omnetpp::SIMTIME_S };
    nextUpdate = omnetpp::simTime() + random_init_delay;

    full_proofs = 0;
    half_proofs = 0;
    failed_proofs = 0;

    proof_size.setName("proof_size");
    proof_requested.setName("proof_requested");

    WATCH(full_proofs);
    WATCH(half_proofs);
    WATCH(failed_proofs);

	ItsG5Service::initialize();
	m_self_msg = new cMessage("Example Service");
	subscribe(scSignalCamReceived);

	scheduleAt(simTime() + 3.0, m_self_msg);
}

void ExampleService::finish()
{
	// you could record some scalars at this point
	ItsG5Service::finish();

    recordScalar("#full_proofs", full_proofs);
    recordScalar("#half_proofs", half_proofs);
    recordScalar("#failed_proofs", failed_proofs);
    recordScalar("#loc_proof_send", loc_proof_send);

}

// Handle behavior when the timer has run out
void ExampleService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");
    //Vehicle ID from VehicleDataProvider
    const auto egoStationID = getFacilities().get_const<VehicleDataProvider>().station_id();
    auto vehicle_ID = std::to_string(egoStationID);

	if (msg == m_self_msg && proof_req == true) {
	    proof_req = false;

	    if(endorsed_list.size() == response_list.size()){
		    EV_INFO << "VEHICLE: " << vehicle_ID << ": NO RESPONSES AT ALL AT: " << omnetpp::simTime() << " .\n";
		    failed_proofs++;
            proof_size.record(endorsed_list.size() - response_list.size());
            proof_requested.record(endorsed_list.size());
	    }
	    else{
		    EV_INFO << "VEHICLE: " << vehicle_ID << ": SOME RESPONSES AT: " << omnetpp::simTime() << " .\n";
		    half_proofs++;
            proof_size.record(endorsed_list.size() - response_list.size());
            proof_requested.record(endorsed_list.size());
	    }
	}
}

void ExampleService::trigger()
{
	Enter_Method("trigger");

	if(omnetpp::simTime() > nextUpdate)
	{
	    static const omnetpp::SimTime location_proof_update_time { 60, omnetpp::SIMTIME_S };
	    nextUpdate = nextUpdate + location_proof_update_time;
	    // use an ITS-AID reserved for testing purposes
	    static const vanetza::ItsAid example_its_aid = 16480;

	    auto& mco = getFacilities().get_const<MultiChannelPolicy>();
	    auto& networks = getFacilities().get_const<NetworkInterfaceTable>();
	    auto& ldm = getFacilities().get_const<LocalDynamicMap>();

	    //Vehicle ID from VehicleDataProvider
	    const auto egoStationID = getFacilities().get_const<VehicleDataProvider>().station_id();

	    auto vehicle_ID = std::to_string(egoStationID);

	    for (auto channel : mco.allChannels(example_its_aid)) {
	        auto network = networks.select(channel);
	        if (network) {
	            btp::DataRequestB req;
	            // use same port number as configured for listening on this channel
	            req.destination_port = host_cast(getPortNumber(channel));
	            req.gn.transport_type = geonet::TransportType::SHB;
	            req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP3));
	            req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;
	            req.gn.its_aid = example_its_aid;

	            cPacket* packet = new cPacket("REQUEST");
	            cPacket* packet2 = new cPacket(vehicle_ID.c_str());

	            endorsed_list = ldm.return_neighbors(mNeighboringWindow, mDecisionLogic);
	            response_list = endorsed_list;

	            std::stringstream endorsed_list_string;

	            // Send requests only if neighbors exist
	            if(!endorsed_list.empty()){
	                std::copy(endorsed_list.begin(), endorsed_list.end()-1, std::ostream_iterator<unsigned long>(endorsed_list_string, ","));
	                endorsed_list_string << endorsed_list.back();

	                cPacket* packet3 = new cPacket(endorsed_list_string.str().c_str());

	                packet2->encapsulate(packet3);
	                packet->encapsulate(packet2);

	                request(req, packet, network.get());
	                loc_proof_send++;

	                m_self_msg = new cMessage("Requested Proof");
	                proof_req = true;
	                // Start timer
	                scheduleAt(simTime() + 3.0, m_self_msg);
	            }

	        } else {
	            EV_ERROR << "No network interface available for channel " << channel << "\n";
	        }
	    }
	}
}

void ExampleService::receiveSignal(cComponent* source, simsignal_t signal, cObject*, cObject*)
{
	Enter_Method("receiveSignal");

	if (signal == scSignalCamReceived) {
		auto& vehicle = getFacilities().get_const<traci::VehicleController>();
	}

}
} // namespace artery
