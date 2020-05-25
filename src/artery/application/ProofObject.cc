#include <artery/application/ProofObject.h>
#include <omnetpp.h>
#include <cassert>

namespace artery
{

void ProofObject::setRequesters (int req){
    requesters = req;
}

void ProofObject::setResponders (int req){
    requesters = responders;
}

} // namespace artery
