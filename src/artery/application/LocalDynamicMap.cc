#include "artery/application/LocalDynamicMap.h"
#include "artery/application/Timer.h"
#include <omnetpp/csimulation.h>
#include <cassert>
#include <algorithm>

namespace artery
{

LocalDynamicMap::LocalDynamicMap(const Timer& timer) :
    mTimer(timer)
{
}

void LocalDynamicMap::updateAwareness(const CaObject& obj)
{
    const vanetza::asn1::Cam& msg = obj.asn1();

    static const omnetpp::SimTime lifetime { 1100, omnetpp::SIMTIME_MS };
    auto tai = mTimer.reconstructMilliseconds(msg->cam.generationDeltaTime);
    const omnetpp::SimTime expiry = mTimer.getTimeFor(tai) + lifetime;

    const auto now = omnetpp::simTime();
    if (expiry < now || expiry > now + 2 * lifetime) {
        EV_STATICCONTEXT
        EV_WARN << "Expiry of received CAM is out of bounds";
        return;
    }

    AwarenessEntry entry(obj, expiry);

    //Look if there is already an entry for the stationID received
    // If yes then replace the latest CAM message with the new received
    // If not then create a new map entry with the CAM message
    auto found = mCaMessages.find(msg->header.stationID);
    if (found != mCaMessages.end()) {
        found->second = std::move(entry);
    } else {
        mCaMessages.emplace(msg->header.stationID, std::move(entry));
    }
}

void LocalDynamicMap::dropExpired()
{
    const auto now = omnetpp::simTime();
    for (auto it = mCaMessages.begin(); it != mCaMessages.end();) {
        if (it->second.expiry < now) {
            it = mCaMessages.erase(it);
        } else {
            ++it;
        }
    }
}

unsigned LocalDynamicMap::count(const CamPredicate& predicate) const
{
    return std::count_if(mCaMessages.begin(), mCaMessages.end(), //Returns the number of elements in the range [first, last) satisfying specific criteria.
            [&predicate](const std::pair<const StationID, AwarenessEntry>& map_entry) {
                const Cam& cam = map_entry.second.object.asn1();
                return predicate(cam);
            });
}

LocalDynamicMap::AwarenessEntry::AwarenessEntry(const CaObject& obj, omnetpp::SimTime t) :
    expiry(t), object(obj)
{
}

// Philippos Barabas, Location Proof System
// Returning a list of neighbor IDs that will be "signed" by the claimer and attached to the location request
std::vector<uint32_t> LocalDynamicMap::return_neighbors(const int NeighboringWindow, const int DecisionLogic) const
{
    static const omnetpp::SimTime lifetime { 1100, omnetpp::SIMTIME_MS };
    const auto now = omnetpp::simTime();

    std::vector<uint32_t> neighbor_list;
    int i = 0;
    for (auto it = mCaMessages.begin(); it != mCaMessages.end();) {

        if (i >= NeighboringWindow){
            return neighbor_list;
        }

        // TODO: CHECK IF THIS HAS AN AFFECT
        // Compare expiry times to decide which neighbors to select
        else if(it->second.expiry > now + (DecisionLogic/100) * lifetime){

//        else if(it->second.expiry > now){
            neighbor_list.push_back(it->first);
            ++it;
            ++i;
        }
        else{
            ++it;
        }
    }
    return neighbor_list;
}


} // namespace artery
