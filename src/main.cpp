#include <Geode/Geode.hpp>
using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("R387 Agent loaded. Image conversion is performed locally; no API key or upload is used.");
}
