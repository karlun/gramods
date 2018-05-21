
#include <gmNetwork/NetSync.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(NetSync);
GM_OFI_PARAM(NetSync, peer, std::string, NetSync::addPeer);
GM_OFI_PARAM(NetSync, bind, std::string, NetSync::setBindAddress);

void NetSync::addPeer(std::string address) {}

void NetSync::setBindAddress(std::string address) {}

void NetSync::initialize() {}

void NetSync::waitForAll(bool onlyConnected) {}

END_NAMESPACE_GMNETWORK;
