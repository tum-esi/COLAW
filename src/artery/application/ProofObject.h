#ifndef ARTERY_PROOFOBJECT_H_
#define ARTERY_PROOFOBJECT_H_

#include <omnetpp/cobject.h>
#include <vanetza/asn1/cam.hpp>
#include <memory>

namespace artery
{

class ProofObject : public cObject
{
public:
    void setRequesters (int req);
    void setResponders (int res);

private:
    int requesters;
    int responders;
};

} // namespace artery

#endif /* ARTERY_PROOFOBJECT_H_ */

