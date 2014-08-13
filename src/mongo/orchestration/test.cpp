#include "orchestration.h"

int main() {
    using namespace mongo::orchestration;

    API api("http://localhost:8889");
    Hosts hosts = api.hosts();
    Host host = hosts.create();

    host.stop();
    host.start();
    host.restart();

    hosts.destroy(host);
}
